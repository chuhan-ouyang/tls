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

// ca.crt
static const char *s_tls_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIUE0YHFEKQXunI0C2/uKfjt2I+NeowDQYJKoZIhvcNAQEL\n"
"BQAwRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
"GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yNDA0MTcwMDAwMzBaFw0yNTA0\n"
"MTcwMDAwMzBaMEUxCzAJBgNVBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEw\n"
"HwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwggIiMA0GCSqGSIb3DQEB\n"
"AQUAA4ICDwAwggIKAoICAQCx2LUx+FkcMN4dQtSJzF7+ZPc3PswccP0AKpvDWh7w\n"
"XNJnq1gwpZlUljYPRFUvNBKcWGeoPRtiSU17xn8WQFXo4l5FwAKnZje2tGQcOFex\n"
"DS934GJQ6ozATb0yqPHfeAvq5B72AfiXX5pA5ezCmkLPVuIZZHwOHH8JrkmgLyyB\n"
"mXE5Kp3v7LksYMqsckm38EvwvH/sxWQwOmzgZsPCjehgGA+2c5rV5qwIMX4Wuclm\n"
"cQX1uJNBxsA6jwcFyMRKAUN8uldZZZ5VD+iuvexSXcrX5zkG+2R62g63PdznzUII\n"
"xs94sSHPJg7nHCcqlozKj2D7jl04cR0xwsUy+pJtd9wz1rHbtchwdS6inPa0bQWa\n"
"ShRG5HS2IbX+Jjr0Yz2A275NehiuydYNPXyEnUDsl5+CAc/EOnLrADJbWcwVb/45\n"
"G0qWkV6tjYGySWz2Ol2m7CQUATDD9x1udSHMBxS4Pb3GmGFqahQiNzO5eu1mkuqN\n"
"KYGc320wfvrMWxKaTyvwWDzhH44a+QK6Kog9U6/9nL5j8sb8S41+QowWnBIYEl5e\n"
"mS02FD8Q/L0FI3aXKUJLk+xU+1tsyCZEwRPgvmZWVAx20UbKteknKm6EJGn3/Eor\n"
"KBGlDXJB0xTuQ4PcVO4ZSBC9xm1d17gADYi6xUP1rfSVfXPykvSPRVph0xya0Yvd\n"
"UQIDAQABo1MwUTAdBgNVHQ4EFgQU97N0aYJb6OF6i6yCPfHuTtosbtkwHwYDVR0j\n"
"BBgwFoAU97N0aYJb6OF6i6yCPfHuTtosbtkwDwYDVR0TAQH/BAUwAwEB/zANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAZZwZ51nNWeFdmU/keNE/yi3XN5mYTnuwfSBLFRXK85gL\n"
"sMUC3GZDDSTCq600gOxGTBU/co592dy/LqLpmzDNnNqcAs2n95D/5UdRFEkaxmPx\n"
"x+SJFBdrpAGQMzkLWpsTtw7eh6BVGkP9o537HI/KIrQ1+9Pc/XoIk+axdE28jutc\n"
"lRYqzUOdPKDyK7YmqEtxI27zPT5yLOVMT68kpqNZlBcEzU7yaF6qf/BXSKbhf8xr\n"
"hk6j8D530OaQdLUCSDZs/fWN/5M8YniNdZ9Ij+jU0CkbXg8kIlfPSYU3+NNDHeTG\n"
"WPP8m/VVzW3y68yrUj9dqBbh+mKTCuhL5KQde3CUOoF2tMS5iaoHjkdUARATDWtg\n"
"NfoiHpKfYEDz0XTnyQ2Pgc3ePq5ntumZx5wCVxRlTuF2JvYoHj2A5PYDY/bs0Yoe\n"
"zzJo9fd3mb5aMtch3zY/uTo+VeHmysh8hK8iAXTY1za2MKjnGrcrrbK5TYKX3iIN\n"
"hOYkfy6WMNXUDwHr2rdxarD8vIqBmluL1GCaoyQHRK6a2Qqa71HKu2FRNw3KyuH2\n"
"+9JWA12BAX8kR7RUx7lj3cRk0V+vjW7g8B08PJtL1DGC8W4MLg6H09AFU+97q+LA\n"
"y6cuOSITLDq2QvGI5Qrd1jbj/gl+3SCfkfaQfp1UvdE6HnnsP/qLj6ynkW/b9AY=\n"
"-----END CERTIFICATE-----\n";

#ifdef TLS_TWOWAY
// client.crt
static const char *s_tls_cert =
"";

// client.key
static const char *s_tls_key =
"";
#endif

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
