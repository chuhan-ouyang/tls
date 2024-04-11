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
static const char *s_root_dir = ".";

// Self signed certificates
// https://mongoose.ws/documentation/tutorials/tls/#self-signed-certificates
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

// server.crt
static const char *s_tls_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIDDzCCAfcCAQEwDQYJKoZIhvcNAQELBQAwSTELMAkGA1UEBhMCSUUxDzANBgNV\n"
"BAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9ja3MxDDAKBgNVBAoMA21vczELMAkGA1UE\n"
"AwwCbWUwHhcNMjQwNDEwMjAwMjU4WhcNMjUwNDEwMjAwMjU4WjBSMQswCQYDVQQG\n"
"EwJJRTEPMA0GA1UECAwGRHVibGluMQ4wDAYDVQQHDAVEb2NrczESMBAGA1UECgwJ\n"
"TXlDb21wYW55MQ4wDAYDVQQDDAVob3dkeTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAO1pBuAAF0gVUyVytvNwWpuBkP6Q5LTd01MkiMGjgdgRIhE+w40S\n"
"6IRnyyLF5M6JGszXEfhXDLqiAtXf7HBZ8F136R3wssbZpBKWnw8DpMxhLOC40hkC\n"
"sASYQoU/sUuc3mFkTp0WfforoY4alCSTlk9wEwQ32PqHrGBQOk9EeAuftEfgVF5e\n"
"RKzqbL7Mq1vF72PT9QmHE/ZDxoI9LiU76QI8Qa0EdP0Rc0NyNgXdZ4kBTgkaMXsU\n"
"YC78TnLRnnfuul+cdE3gMQKM23eUfWV1NQJqaRrG6aJXaib3K95NoYoQplGIutWy\n"
"wUNWEvvVgCMrUZKPeFj4oZULjv/RJc8/y+UCAwEAATANBgkqhkiG9w0BAQsFAAOC\n"
"AQEALrpBONh6bpNyxqiXDnszGVjHmZxfNqhcNO8jWVckD6U570EvJcOAtP+3Ht1I\n"
"+pjr1AfzpSVvJ/D3iHsQ5/FwrxzDhzth7rdiwvOsVvxqxNk/a3JaJDa1USFl+S1L\n"
"JYaKgDQKl2yGYmlpwhk7WVrsg4OdItOzyZhsZ+LHeV4wkmyEF2GcLhU02Tavhk9R\n"
"kpL8sG6ajShqKmQUt5jDCzJLKaXc/yGerX+8akEtn8/0du0+cVSRwoZ6gykZ03Jr\n"
"DY4+ZZ7TevtsOAiyO/FreR/XrkbbD6PrGfuJoOuP9BgB9+qOcXvMXkmkMuku36wX\n"
"jUZERY/p4FtnhTZ3nbQtrl8/kg==\n"
"-----END CERTIFICATE-----\n";

// server.key
static const char *s_tls_key =
"-----BEGIN PRIVATE KEY-----\n"
" "
"-----END PRIVATE KEY-----\n";

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  // accept clinet connection
  if (ev == MG_EV_ACCEPT && c->fn_data != NULL) {
        struct mg_tls_opts opts = {
    #ifdef TLS_TWOWAY
            .ca = mg_str(s_tls_ca),
    #endif
            .cert = mg_str(s_tls_cert),
            .key = mg_str(s_tls_key)};
        mg_tls_init(c, &opts);
  }
  // accept http request
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/api/stats")) {
      // Print some statistics about currently established connections
      mg_printf(c, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
      mg_http_printf_chunk(c, "ID PROTO TYPE      LOCAL           REMOTE\n");
      for (struct mg_connection *t = c->mgr->conns; t != NULL; t = t->next) {
        mg_http_printf_chunk(c, "%-3lu %4s %s %M %M\n", t->id,
                             t->is_udp ? "UDP" : "TCP",
                             t->is_listening  ? "LISTENING"
                             : t->is_accepted ? "ACCEPTED "
                                              : "CONNECTED",
                             mg_print_ip, &t->loc, mg_print_ip, &t->rem);
      }
      mg_http_printf_chunk(c, "");  // Don't forget the last empty chunk
    } else if (mg_http_match_uri(hm, "/api/f2/*")) {
      mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", (int) hm->uri.len,
                    hm->uri.ptr);
    } else if (mg_http_match_uri(hm, "/api/headers")) {
      struct mg_http_message *hm = (struct mg_http_message *) ev_data;
      size_t i, max = sizeof(hm->headers) / sizeof(hm->headers[0]);
      // Send HTTP response line and Headers
      mg_printf(c, "%s%s%s\r\n",
                "HTTP/1.1 200 OK\r\n",               // Output response line
                "Transfer-Encoding: chunked\r\n",  // Chunked header
                "Content-Type: text/plain\r\n");    // and Content-Type header
      mg_printf(c, "Request headers:\n");
      // Iterate over request headers, and print them one by one
      for (i = 0; i < max && hm->headers[i].name.len > 0; i++) {
        struct mg_str *k = &hm->headers[i].name, *v = &hm->headers[i].value;
        // Use printf_chunk for chunkned http reply
        mg_http_printf_chunk(c, "%.*s -> %.*s\n", (int) k->len, k->ptr, (int) v->len, v->ptr);
      }
      mg_http_write_chunk(c, "", 0);  // Final empty chunk
    } else {
      // show current directories
      struct mg_http_serve_opts opts = {.root_dir = s_root_dir};
      mg_http_serve_dir(c, ev_data, &opts);
    }
  }
  if (ev == MG_EV_TLS_HS) {
    mg_printf(c, "EHLO myname\r\n");
    MG_INFO(("SERVER TLS handshake done! Sending EHLO again"));
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
