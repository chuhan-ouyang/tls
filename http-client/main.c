// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved
//
// Example HTTP client. Connect to `s_url`, send request, wait for a response,
// print the response and exit.
// You can change `s_url` from the command line by executing: ./example YOUR_URL
//
// To enable SSL/TLS, , see https://mongoose.ws/tutorials/tls/#how-to-build

#include "mongoose.h"
#define TLS_TWOWAY

// The very first web page in history. You can replace it from command line
static const char *s_url = "http://127.0.0.1:8000/api/f2/hi";
static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds

#ifdef TLS_TWOWAY

// ca.crt
static const char *s_tls_ca = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDczCCAlugAwIBAgIUG4pDaNQrU8lxaFxbAphJK3lr4N4wDQYJKoZIhvcNAQEL\n"
"BQAwSTELMAkGA1UEBhMCSUUxDzANBgNVBAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9j\n"
"a3MxDDAKBgNVBAoMA21vczELMAkGA1UEAwwCbWUwHhcNMjQwNDEwMjAwMjU3WhcN\n"
"MjUwNDEwMjAwMjU3WjBJMQswCQYDVQQGEwJJRTEPMA0GA1UECAwGRHVibGluMQ4w\n"
"DAYDVQQHDAVEb2NrczEMMAoGA1UECgwDbW9zMQswCQYDVQQDDAJtZTCCASIwDQYJ\n"
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAIn1rWI9YN7Or7leSAdtgIboKcD+hWtT\n"
"ODL0thqgfAo4wbwI1HIo9rslWp5rpV3HQ5e2/ihTWvOKR1En7Runge54QolwVFXR\n"
"MIcX2lp7+YueiO2KCPvn5XP6bssuF49BaJwxJFcEguqdBRzpOYQCGG0M7/GZtKNp\n"
"utgcq7qJcpWYUolz3VV91neo+CLhramnAOji4eAlfJ9vq1BNS5dypDBIXz0ZRsEe\n"
"UwOfGeJFBGMjKuvosNSDe8j19rmE7b0lh3F6+LbVIba0fgHx8h8D+exvtGegxYZc\n"
"M0ok/g/wo+8rJpztXAQIRgYI78txFneziZSGLuLOdVmcNxB7vftL9x8CAwEAAaNT\n"
"MFEwHQYDVR0OBBYEFB9tGdECRiOy0ULUzXcgPac/EY4oMB8GA1UdIwQYMBaAFB9t\n"
"GdECRiOy0ULUzXcgPac/EY4oMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
"BQADggEBACzg8HXi3DG/x/sFEFTNNO1iPvmagjTIrmdsweLXiqRkb+/1m17y1vqN\n"
"/yqM+Ir6NdQNB/ev7z2H7u59HDFvs87opcmGxjgMgqFb+yxdxQd46CU1BeQjczPo\n"
"8nTxJQBB4Rn5KPdUmfo0rvzLqQdRyDdJxPWYlpmm4jZ54ECe401w5d7LZqBljkZl\n"
"CJs1UMr3D0CmVhJ5EdBi/hdJPV8PgKcYi7PRgnl5SMKvtbY7oTo9RBHfN6XhvyOc\n"
"FbxqQ5z6xNs6ApVEGVqFIoVQzb/4VGMxaMy3wpSJ3b0XuveqDuciD+BbPEAHRhmS\n"
"K3XYKT2J22wa++XfsfYQ7c3pLUqcpds=\n"
"-----END CERTIFICATE-----\n";

#endif

// client.crt
static const char *s_tls_cert = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDDzCCAfcCAQEwDQYJKoZIhvcNAQELBQAwSTELMAkGA1UEBhMCSUUxDzANBgNV\n"
"BAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9ja3MxDDAKBgNVBAoMA21vczELMAkGA1UE\n"
"AwwCbWUwHhcNMjQwNDEwMjAwMjU4WhcNMjUwNDEwMjAwMjU4WjBSMQswCQYDVQQG\n"
"EwJJRTEPMA0GA1UECAwGRHVibGluMQ4wDAYDVQQHDAVEb2NrczESMBAGA1UECgwJ\n"
"TXlDb21wYW55MQ4wDAYDVQQDDAVob3dkeTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAMbQ1u06xzj3vsgEklpFSxQE9QVsbjMXi3X18bGJlJEK8uR3ZGH5\n"
"mXTY72u+W33gQQ3ID8FTGjr5zAK0Y+fxJvVxT/PNFxj1FCaibjso7SawwzUwz901\n"
"8DISNSbzmQWbLZOOPcPp/v5SmYjDmgtbjlHhJePhRQiCVHfXMDkHd86gpaDzhk2Z\n"
"s/w1WsiLmBBiWY+RULI7/khQCqAMs856F0zuwXCvZBjv+U39vZLaOWdwURLeSTUs\n"
"uZ79x9ijLOn5miKbRBnJSZF0j1dNQgMtoBGhMNpvmi5Kl5aUvoGfBolVlo8bzknt\n"
"zxJI0EMzXkzrPopN4a815iDBy1W1caX2yQ0CAwEAATANBgkqhkiG9w0BAQsFAAOC\n"
"AQEAgRW9Xa3AwLuEMlpWnTeivwC8i6dFs2Q+5x6yhT5rg6eA/pmzvnnN0x+zG88c\n"
"XbU6IyWEF4w+M866dNiGXaQhpP+QYgC7aoLSge4A0rUblEZnIDni1VRPgbkEYFu4\n"
"Pk0MYVLh3L48Du/o7eOpY1hyE4UyxyvwZsyOe3nwcg78Xz6BH7BhFjvDLreviTtQ\n"
"D+s1wRMg4Bljc/96ztLnjtgGTAzKl1HKMxc2NBUHL8MWcG++FIrli69luqglFAjL\n"
"eYgM2oXi4pAyDjbHI5QX+RM4ZiXFVuRLiLl31D6n2C8txmCNQ+Pe9qovFy8t7ESK\n"
"WksIGT+SpgnYXTq5yc/Le+BlHg==\n"
"-----END CERTIFICATE-----\n";

// client.key
static const char *s_tls_key = 
"-----BEGIN PRIVATE KEY-----\n"
" "
"-----END PRIVATE KEY-----\n";

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
    printf("\ngot host name\n");
    if (mg_url_is_ssl(s_url)) {
       printf("\nmg_url_is_ssl\n");
       struct mg_tls_opts opts = {
    #ifdef TLS_TWOWAY
            .ca = mg_str(s_tls_ca),
    #endif
            .cert = mg_str(s_tls_cert),
            .key = mg_str(s_tls_key)};
        mg_tls_init(c, &opts);
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
  } else if (ev == MG_EV_HTTP_MSG) {
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    printf("%.*s", (int) hm->message.len, hm->message.ptr);
    c->is_draining = 1;        // Tell mongoose to close this connection
    *(bool *) c->fn_data = true;  // Tell event loop to stop
  } else if (ev == MG_EV_TLS_HS) {
    mg_printf(c, "EHLO myname\r\n");
    MG_INFO(("CLIENT TLS handshake done! Sending EHLO again"));
  } else if (ev == MG_EV_ERROR) {
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
