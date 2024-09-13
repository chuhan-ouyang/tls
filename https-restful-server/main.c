// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// HTTP server example. This server serves both static and dynamic content.
// It opens two ports: plain HTTP on port 8000 and HTTPS on port 8443.
// It implements the following endpoints:
//    /api/stats - respond with free-formatted stats on current connections
//    /api/f2/:id - wildcard example, respond with JSON string {"result": "URI"}
//    any other URI serves static files from s_root_dir
//
// To enable SSL/TLS (using self-signed certificates in PEM files),
//    1. See https://mongoose.ws/tutorials/tls/#how-to-build
//    2. curl -k https://127.0.0.1:8443

#include "mongoose.h"
#define TLS_TWOWAY

static const char *s_http_addr = "http://127.0.0.1:8000";    // HTTP port
static const char *s_https_addr = "https://127.0.0.1:8443";  // HTTPS port

// Self signed certificates
// https://mongoose.ws/documentation/tutorials/tls/#self-signed-certificates

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
// Connection event handler function
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/get")) {
      struct mg_str json = hm->body;  // The body of the HTTP message which contains JSON

      // Allocate memory for res to store the JSON string
      char *res = (char *)malloc(json.len + 1);  // +1 for the null terminator
      if (res == NULL) {
          printf("Memory allocation failed\n");
          mg_http_reply(c, 500, "", "Internal Server Error\n");
          return;
      }

      // Copy the JSON data to res and null-terminate it
      memcpy(res, json.ptr, json.len);
      res[json.len] = '\0';  // Null-terminate the string

      // Now you can use res as a regular null-terminated string
      printf("Stored JSON: %s\n", res);

      // Always remember to free dynamically allocated memory
      free(res);
    } else {
      mg_http_reply(c, 200, "", "Cascade CBDC Web Server\n");
    }
  }
}

int main(void) {
  struct mg_mgr mgr;                            // Event manager
  mg_log_set(MG_LL_DEBUG);                      // Set log level
  mg_mgr_init(&mgr);                            // Initialise event manager
  mg_http_listen(&mgr, s_http_addr, fn, NULL);  // Create HTTP listener
  mg_http_listen(&mgr, s_https_addr, fn, (void *) 1);  // HTTPS listener
  for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
  mg_mgr_free(&mgr);
  return 0;
}
