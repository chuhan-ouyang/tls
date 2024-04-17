// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved
//
// Example HTTP client. Connect to `s_url`, send request, wait for a response,
// print the response and exit.
// You can change `s_url` from the command line by executing: ./example YOUR_URL
//
// To enable SSL/TLS, , see https://mongoose.ws/tutorials/tls/#how-to-build

#include "mongoose.h"
// #define TLS_TWOWAY

#define line = "------------------------------\n";
// The very first web page in history. You can replace it from command line
static const char *s_url = "https://127.0.0.1:8443";
// static const char *s_url = "http://127.0.0.1:8000";
static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds

// Print HTTP response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_OPEN) {
    // Connection created. Store connect expiration time in c->data
    *(uint64_t *) c->data = mg_millis() + s_timeout_ms;
  } else if (ev == MG_EV_POLL) {
    if (mg_millis() > *(uint64_t *) c->data &&
        (c->is_connecting || c->is_resolving)) {
        mg_error(c, "Connect timeout");
    }
  } else if (ev == MG_EV_CONNECT) {
    // Connected to server. Extract host name from URL
    struct mg_str host = mg_url_host(s_url);

    if (mg_url_is_ssl(s_url)) {
      // String Way
    //    struct mg_tls_opts opts = {
    //         .ca = mg_str(s_tls_ca),
    //         // .ca = mg_unpacked("../certs/ca/ca.crt"),
    // #ifdef TLS_TWOWAY
    //         .cert = mg_str(s_tls_cert),
    //         .key = mg_str(s_tls_key)
    // #endif
    //    };

    // Pem way
        struct mg_str ca = mg_file_read(&mg_fs_posix, "ca.pem");
        struct mg_tls_opts opts = {.ca = ca};
        mg_tls_init(c, &opts);
        printf("----------------");
        printf("\nFinished mg_tls_init\n");
        printf("----------------\n");
        // free(ca.ptr);
    }
    // Send request
    int content_length = s_post_data ? strlen(s_post_data) : 0;
    mg_printf(c,
              "%s %s HTTP/1.0\r\n"
              "Host: %.*s\r\n"
              "Content-Type: octet-stream\r\n"
              "Content-Length: %d\r\n"
              "\r\n",
              s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int) host.len,
              host.ptr, content_length);
    mg_send(c, s_post_data, content_length);
    printf("----------------");
    printf("\nFinished mg_send\n");
    printf("----------------\n");
  } else if (ev == MG_EV_HTTP_MSG) {
    printf("----------------");
    printf("\nGot to if (ev == MG_EV_HTTP_MSG)\n");
    printf("----------------\n");

    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    printf("%.*s", (int) hm->message.len, hm->message.ptr);
    c->is_draining = 1;        // Tell mongoose to close this connection
    *(bool *) c->fn_data = true;  // Tell event loop to stop

    printf("----------------");
    printf("\nExited if (ev == MG_EV_HTTP_MSG)\n");
    printf("----------------\n");
  } else if (ev == MG_EV_ERROR) {
    printf("\nError\n");
    *(bool *) c->fn_data = true;  // Error, tell event loop to stop
  }
}

int main(int argc, char *argv[]) {
  const char *log_level = getenv("LOG_LEVEL");  // Allow user to set log level
  if (log_level == NULL) log_level = "4";       // Default is verbose

  struct mg_mgr mgr;              // Event manager
  bool done = false;              // Event handler flips it to true
  if (argc > 1) s_url = argv[1];  // Use URL provided in the command line
  mg_log_set(atoi(log_level));    // Set to 0 to disable debug
  mg_mgr_init(&mgr);              // Initialise event manager
  mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
  while (!done) mg_mgr_poll(&mgr, 50);      // Event manager loops until 'done'
  mg_mgr_free(&mgr);                        // Free resources
  return 0;
}
