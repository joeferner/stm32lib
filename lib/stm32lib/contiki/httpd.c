
#include "network.h"
#include "httpd.h"
#include "../sha1.h"
#include "../base64.h"
#include <net/ip/tcpip.h>
#include <stdlib.h>

const char http_10[] = " HTTP/1.0\r\n";
const char http_content_type[] = "Content-Type:";
const char http_content_len[] = "Content-Length:";
const char http_sec_websocket_key[] = "Sec-WebSocket-Key:";
const char http_websocket_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const char http_header_404[] = "HTTP/1.0 404 Not found\r\nConnection: close\r\n";
const char html_not_found[] = "<html><body><h1>Page not found</h1></body></html>";

const char mimetype_application_javascript[] = "application/javascript";
const char mimetype_image_png[] = "image/png";
const char mimetype_text_css[] = "text/css";
const char mimetype_text_html[] = "text/html";
const char mimetype_text_plain[] = "text/plain";

const char http_header_200[] = "HTTP/1.1 200 OK\r\nCache-Control: public, max-age=864000\r\nConnection: close\r\n";
const char http_200_ok[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 2\r\n\r\nOK";
const char http_400_fail[] = "HTTP/1.1 200 BAD\r\nConnection: close\r\nContent-Length: 4\r\n\r\nFAIL";
const char http_header_101_ws_upgrade[] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n";

uint16_t http_connections = 0;
static struct httpd_state conns[HTTPD_CONNS];

void httpd_init();
void httpd_appcall(process_event_t ev, void *state);
void httpd_state_init();
struct httpd_state *httpd_state_alloc();
void httpd_state_free(struct httpd_state *s);
void httpd_handle_connection(process_event_t ev, struct httpd_state *s);

PROCESS(httpd_process, "HTTP server");

PROCESS_THREAD(httpd_process, ev, data) {
  static struct etimer et;
  int i;

  PROCESS_BEGIN();

  httpd_init();

  /* Delay 2-4 seconds */
  etimer_set(&et, CLOCK_SECOND * 10);

  /* GC any http session that is too long lived - either because other
     end never closed or if any other state cause too long lived http
     sessions */
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event || etimer_expired(&et));
    if (ev == tcpip_event) {
      httpd_appcall(ev, data);
    } else if (etimer_expired(&et)) {
      printf("HTTPD States: ");
      for (i = 0; i < HTTPD_CONNS; i++) {
        printf("%04x ", conns[i].state);
        if (conns[i].state != HTTPD_STATE_UNUSED && timer_expired(&conns[i].timer)) {
          conns[i].state = HTTPD_STATE_UNUSED;
          printf("*** RELEASED HTTPD Session");
          http_connections--;
        }
      }
      printf("\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}

void httpd_init() {
  printf("?httpd_init\n");
  tcp_listen(UIP_HTONS(80));
  httpd_state_init();
}

void httpd_appcall(process_event_t ev, void *state) {
  struct httpd_state *s = (struct httpd_state *)state;

  if (uip_closed() || uip_aborted() || uip_timedout()) {
    if (s != NULL) {
      printf("HTTPD: closed/aborted: ");
      if (s->file.fileName) {
        printf("%s", s->file.fileName);
      }
      printf("\n");
      http_connections--;
      httpd_state_free(s);
    } else {
      printf("HTTPD: closed/aborted ** NO HTTPD_WS_STATE!!! **\n");
    }
  } else if (uip_connected()) {
    if (s == NULL) {
      s = httpd_state_alloc();
      if (s == NULL) {
        uip_abort();
        printf("HTTPD: aborting - no resource\n");
        return;
      }
      http_connections++;

      tcp_markconn(uip_conn, s);
      s->state = HTTPD_STATE_INPUT;
    } else {
      /* this is a request that is to be sent! */
      s->state = HTTPD_STATE_REQUEST_OUTPUT;
    }
    PSOCK_INIT(&s->sock, (uint8_t *)s->buf, sizeof(s->buf) - 1);
    PT_INIT(&s->outputpt);
    timer_set(&s->timer, CLOCK_SECOND * 30);
    httpd_handle_connection(ev, s);
  } else if (s != NULL) {
    if (uip_poll()) {
      if (timer_expired(&s->timer)) {
        uip_abort();
        printf("HTTPD: aborting - http timeout\n");
        http_connections--;
        httpd_state_free(s);
      }
    } else {
      timer_restart(&s->timer);
    }
    httpd_handle_connection(ev, s);
  } else {
    printf("HTTPD: aborting - no state\n");
    uip_abort();
  }
}

PT_THREAD(send_string(struct httpd_state *s, const char *str, uint16_t len)) {
  PSOCK_BEGIN(&s->sock);
  SEND_STRING(&s->sock, str, len);
  PSOCK_END(&s->sock);
}

PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr)) {
  PSOCK_BEGIN(&s->sock);

  SEND_STRING(&s->sock, statushdr, strlen(statushdr));
  strcpy((char *)s->buf, http_content_type);
  strcat((char *)s->buf, " ");
  strcat((char *)s->buf, (s->file.contentType == NULL) ? mimetype_text_html : s->file.contentType);
  strcat((char *)s->buf, "\r\n\r\n");
  s->outbuf_pos = strlen((char *)s->buf);
  SEND_STRING(&s->sock, s->buf, s->outbuf_pos);
  s->outbuf_pos = 0;

  PSOCK_END(&s->sock);
}

void httpd_calculate_websocket_accept(const char *key, char *accept) {
  SHA1Context ctx;
  int i, j;
  uint8_t data[20];

  SHA1Reset(&ctx);
  SHA1Input(&ctx, (uint8_t *)key, strlen(key));
  SHA1Input(&ctx, (uint8_t *)http_websocket_guid, strlen(http_websocket_guid));
  SHA1Result(&ctx);

  for (i = 0, j = 0; i < 5; i++, j += 4) {
    data[j + 0] = (ctx.Message_Digest[i] >> 24) & 0xff;
    data[j + 1] = (ctx.Message_Digest[i] >> 16) & 0xff;
    data[j + 2] = (ctx.Message_Digest[i] >> 8) & 0xff;
    data[j + 3] = (ctx.Message_Digest[i] >> 0) & 0xff;
  }

  base64_encode(data, 20, accept);
}

PT_THREAD(httpd_handle_input(process_event_t ev, struct httpd_state *s)) {
  PSOCK_BEGIN(&s->sock);
  PSOCK_READTO(&s->sock, ' ');

  if (strncmp((char *)s->buf, "GET ", 4) == 0) {
    s->request_type = HTTPD_GET;
  } else if (strncmp((char *)s->buf, "POST ", 5) == 0) {
    s->request_type = HTTPD_POST;
    s->content_len = 0;
  } else if (strncmp((char *)s->buf, "HTTP ", 5) == 0) {
    s->request_type = HTTPD_RESPONSE;
  } else {
    PSOCK_CLOSE_EXIT(&s->sock);
  }
  PSOCK_READTO(&s->sock, ' ');

  if (s->buf[0] != '/') {
    PSOCK_CLOSE_EXIT(&s->sock);
  }

  s->buf[PSOCK_DATALEN(&s->sock) - 1] = 0;
  if (strcmp((const char *)s->buf, "/") == 0) {
    strcpy((char *)s->buf, "/index.html");
  }
  s->file.contentType = NULL;
  s->file.fileName = NULL;
  s->file.script = NULL;
  httpd_get_file((const char *)s->buf, &s->file);
  s->file_pos = 0;
  printf("?httpd file: %s\n", (const char *)s->buf);

  while (1) {
    PSOCK_READTO(&s->sock, '\n');

    if (s->request_type == HTTPD_POST && strncmp((char *)s->buf, http_content_len, 15) == 0) {
      s->buf[PSOCK_DATALEN(&s->sock) - 2] = 0;
      s->content_len = atoi((char *)&s->buf[16]);
    }

    if (strncmp((char *)s->buf, http_sec_websocket_key, 18) == 0) {
      s->buf[PSOCK_DATALEN(&s->sock) - 2] = 0;
      httpd_calculate_websocket_accept((char *)&s->buf[19], s->sec_websocket_accept);
    }

    /* should have a header callback here check_header(s) */
    if (PSOCK_DATALEN(&s->sock) > 2) {
      s->buf[PSOCK_DATALEN(&s->sock) - 2] = 0;
    } else if (s->request_type == HTTPD_POST) {
      if (s->content_len > 0) {
        PSOCK_READBUF_LEN(&s->sock, s->content_len);
        s->buf[PSOCK_DATALEN(&s->sock)] = 0;
      } else {
        s->buf[0] = 0;
      }
      s->state = HTTPD_STATE_OUTPUT;
      break;
    } else {
      s->state = HTTPD_STATE_OUTPUT;
      break;
    }
  }
  PSOCK_END(&s->sock);
}

PT_THREAD(httpd_handle_output(process_event_t ev, struct httpd_state *s)) {
  PT_BEGIN(&s->outputpt);

  if (s->file.script == NULL) {
    PT_WAIT_THREAD(&s->outputpt, send_headers(s, http_header_404));
    PT_WAIT_THREAD(&s->outputpt, send_string(s, html_not_found, strlen(html_not_found)));
  } else {
    PT_WAIT_THREAD(&s->outputpt, s->file.script(ev, s));
  }
  PSOCK_CLOSE(&s->sock);
  PT_END(&s->outputpt);
}

void httpd_handle_connection(process_event_t ev, struct httpd_state *s) {
  if (s->state == HTTPD_STATE_INPUT) {
    httpd_handle_input(ev, s);
  }
  if (s->state == HTTPD_STATE_OUTPUT) {
    httpd_handle_output(ev, s);
  }
}

void httpd_state_init() {
  int i;

  for (i = 0; i < HTTPD_CONNS; i++) {
    conns[i].state = HTTPD_STATE_UNUSED;
  }
}

struct httpd_state *httpd_state_alloc() {
  int i;

  for (i = 0; i < HTTPD_CONNS; i++) {
    if (conns[i].state == HTTPD_STATE_UNUSED) {
      conns[i].state = HTTPD_STATE_INPUT;
      return &conns[i];
    }
  }
  return NULL;
}

void httpd_state_free(struct httpd_state *s) {
  s->state = HTTPD_STATE_UNUSED;
}

