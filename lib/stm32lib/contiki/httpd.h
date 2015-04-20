
#ifndef _STM32LIB_CONTIKI_HTTPD_H_
#define _STM32LIB_CONTIKI_HTTPD_H_

#include <net/ip/psock.h>

#ifndef HTTPD_CONNS
#  define HTTPD_CONNS UIP_CONNS
#endif

#ifndef WEBSERVER_CONF_CFS_PATHLEN
#  define HTTPD_PATHLEN 80
#else
#  define HTTPD_PATHLEN WEBSERVER_CONF_CFS_PATHLEN
#endif

#ifndef WEBSERVER_CONF_OUTBUF_SIZE
#  define HTTPD_OUTBUF_SIZE (UIP_TCP_MSS + 20)
#else
#  define HTTPD_OUTBUF_SIZE WEBSERVER_CONF_OUTBUF_SIZE
#endif

#define SEND_STRING(s, str, len) PSOCK_SEND((s), (uint8_t *)(str), (len))

#define HTTPD_GET      1
#define HTTPD_POST     2
#define HTTPD_PUT      3
#define HTTPD_RESPONSE 4

#define HTTPD_STATE_UNUSED         0
#define HTTPD_STATE_INPUT          1
#define HTTPD_STATE_OUTPUT         2
#define HTTPD_STATE_REQUEST_OUTPUT 3

struct httpd_state;

typedef char(*httpd_script_t)(process_event_t ev, struct httpd_state *s);

extern const char mimetype_application_javascript[];
extern const char mimetype_image_png[];
extern const char mimetype_text_css[];
extern const char mimetype_text_html[];
extern const char mimetype_text_plain[];

extern const char http_header_200[];
extern const char http_200_ok[];
extern const char http_400_fail[];
extern const char http_500_internalServerError[];
extern const char http_header_101_ws_upgrade[];

struct httpd_file {
  const char* fileName;
  const char* contentType;
  httpd_script_t script;
};

extern bool httpd_get_file(const char *filename, struct httpd_file *file);

struct httpd_state {
  uint32_t startTime;
  struct timer timer;
  struct psock sock;
  struct pt outputpt;
  struct httpd_file file;
  int fileHandle;
  uint32_t file_pos;
  uint32_t content_len;
  char sec_websocket_accept[35];
  uint8_t buf[HTTPD_OUTBUF_SIZE];
  uint16_t outbuf_pos;
  uint8_t state;
  uint8_t request_type;
  struct etimer ws_etimer;
};

PROCESS_NAME(httpd_process);
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr));

#endif
