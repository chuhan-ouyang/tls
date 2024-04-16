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
"MIIDczCCAlugAwIBAgIULSyNrHx8NdoaeSgrLwj62lv+5/gwDQYJKoZIhvcNAQEL\n"
"BQAwSTELMAkGA1UEBhMCSUUxDzANBgNVBAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9j\n"
"a3MxDDAKBgNVBAoMA21vczELMAkGA1UEAwwCbWUwHhcNMjQwNDE2MTg1MDM3WhcN\n"
"MjUwNDE2MTg1MDM3WjBJMQswCQYDVQQGEwJJRTEPMA0GA1UECAwGRHVibGluMQ4w\n"
"DAYDVQQHDAVEb2NrczEMMAoGA1UECgwDbW9zMQswCQYDVQQDDAJtZTCCASIwDQYJ\n"
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAL/PgLJ3IW1fJTirgsZqhDGgf7i5/SBV\n"
"iJ1CD2qmVbsrWbZMZ8kxZF283C0pMzdtLhX2JivtImxo9juMg1MNU/zc38rmEVy8\n"
"8zv5v6VZnWbRkDBmHWpE3OXzb9L3BdIwB7VXsJxPplZPSzsuql0kEpwPF3NHBs4A\n"
"06OTxN5stZ50uSEw8gGK+ZqfIo+RWkjwFgznRx8SqxmeFWE+T0GhIhpWwOsxiiOF\n"
"gLy5tjkODZk5jxuMAC5zBvHkPM5HCIQnw/65qD6n07qJO7vx7yyaY/GrhVx705Ge\n"
"g2F5htzIhtUmenpohSPk8YaEh5FUWxCE/Yx8kwO4aO8KBytX5Ex/SKMCAwEAAaNT\n"
"MFEwHQYDVR0OBBYEFM2iwLoiIUb35o/gpQmSCAl+RymwMB8GA1UdIwQYMBaAFM2i\n"
"wLoiIUb35o/gpQmSCAl+RymwMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
"BQADggEBAF8E9MYDpb9VBZDlDMtZmAZKQILsVmb2mF4aZrYF4/SfvM6nylf7mKxs\n"
"dXPS5b42yNm9NaBupFAa8XqGHOpEYZCynM/3k3JuXdVJdo5lommGtmWZgzyABRZ1\n"
"cfjt0IcZ2kFAgE1DfXzmP23XQG8fD8ab7lSlXU5WQr0bbTbebc2BCIqr6kRlKX1y\n"
"yEBsnNoT2/94Ujrf04cadoYcqG068odoWVz7dqrEmKGdwCgbjw13EtQrHIk34irq\n"
"d2Lzpe3L0GM+X2g2NR3wXFMlFO11yFNMIv7uDa2cxGaBA8a+6ps44dlobm7aDuQ9\n"
"TVh9ZrBUj5ZwvTpWtY290EBm8kYsd8c=\n"
"-----END CERTIFICATE-----\n";
#endif

// server.crt
static const char *s_tls_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIDDzCCAfcCAQEwDQYJKoZIhvcNAQELBQAwSTELMAkGA1UEBhMCSUUxDzANBgNV\n"
"BAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9ja3MxDDAKBgNVBAoMA21vczELMAkGA1UE\n"
"AwwCbWUwHhcNMjQwNDE2MTg1MDM4WhcNMjUwNDE2MTg1MDM4WjBSMQswCQYDVQQG\n"
"EwJJRTEPMA0GA1UECAwGRHVibGluMQ4wDAYDVQQHDAVEb2NrczESMBAGA1UECgwJ\n"
"TXlDb21wYW55MQ4wDAYDVQQDDAVob3dkeTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAKOHLAZvDiNRjj+4QzU2wEji43lDEoxHkkZrp+j03nbjTku7QA8V\n"
"a2PDekgyEKaj2C99S/7OeW+BY5fjn+epumn5xJ7BV/NFN0FJxmMyMWUO2H/9WO0+\n"
"aWjv+Dr9AnPuXYxHBY57Af9zq5aoskKQyh2iQn5lLse9dHC4dB77ttaM0LVzizZ8\n"
"M+axtWyKq6Wp38pjwpelTI82utV7CCE3aGaV12YTFYd63Fuu5qP1ijCZgZv997j1\n"
"Js9sCKHN4bg8xx/TmOtEH+CnS3yuiyj2Ey2LL4sKV/00wcHLIY8I2/J1MOWJs/Ws\n"
"OfQ0ZXxQCi2ad5X+/NMQfqyZEkN4kw5YVu8CAwEAATANBgkqhkiG9w0BAQsFAAOC\n"
"AQEAHGZ+fGn7RPlk4n13WU1SkfcK92PZ/+ODkuv3JO+E8EITm1FmwiHI4bylqQI5\n"
"aTyE+rbdJnVPI21BjH2+4AcTIliGqHhDF/L9Qo1dAPuz6eMw93WWJFvkKb+2AApW\n"
"0LcyN1D27PzV6Kr/2bxZ9UJfvmLo7RJIB7Ncpc2v8CKc1bFapr1s9R2syMhRJ5rr\n"
"4wh636d1TzKzCwlatQXUMiIaIZbj5DcLG43iIVJi208uH3BbklekxjtiLi4ZwQyc\n"
"ipWx1IvSZR9oRV23WmxC0702oFuHFct5D6a4lw2uNzdbicFSbtevvVqY+F87wZzr\n"
"DTMlh/NB91iNr1kDVaQ3Hiwrsg==\n"
"-----END CERTIFICATE-----\n";

// server.key
static const char *s_tls_key = 
"-----BEGIN PRIVATE KEY-----\n"
 
"-----END PRIVATE KEY-----\n";

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_ACCEPT && c->fn_data != NULL) {
        struct mg_tls_opts opts = {
    #ifdef TLS_TWOWAY
            .ca = mg_str(s_tls_ca),
    #endif
            .cert = mg_str(s_tls_cert),
            .key = mg_str(s_tls_key)};
        mg_tls_init(c, &opts);
  }
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
    } else {
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
