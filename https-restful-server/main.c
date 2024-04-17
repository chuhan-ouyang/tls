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
// #define TLS_TWOWAY

static const char *s_http_addr = "http://127.0.0.1:8000";    // HTTP port
static const char *s_https_addr = "https://127.0.0.1:8443";  // HTTPS port
static const char *s_root_dir = ".";

// Self signed certificates
// https://mongoose.ws/documentation/tutorials/tls/#self-signed-certificates
#ifdef TLS_TWOWAY

// ca.crt
static const char *s_tls_ca =
"";
#endif

// server.crt
static const char *s_tls_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIFSTCCAzGgAwIBAgIUIVogzzhANbOnmL38G/3tfcUEwpowDQYJKoZIhvcNAQEL\n"
"BQAwRTELMAkGA1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
"GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yNDA0MTcwMDAxMzBaFw0yNTA0\n"
"MTcwMDAxMzBaMBIxEDAOBgNVBAMMB0Nhc2NhZGUwggIiMA0GCSqGSIb3DQEBAQUA\n"
"A4ICDwAwggIKAoICAQDBXxef4N9y52dE+9dJh4I8boSE2pCRbKfai1z2zwNT53dv\n"
"Y/jkNAlzHYQ0t1BSFlglTZQ5RC1CFxqwZr6VY/kTFub0NsBY2XAxoC3BgQs9xz8Q\n"
"TPjGFy62et2Kw4+KYzoRSm6y8cdmBYxHNG+14BTSFojmjuUOzDaTXnD83ukWtKgf\n"
"2uJJixAQxCOO6H8byoIslHc5Fh+9NSUohdbFMxNgTTdqMW6w/r0M1cr5vvZn5ZLh\n"
"4AnZlHll6L0EL2mueahrHl/xOcRm38d7S/P+F5vsKLmrBVp3XOf5kfUCitbZkVaa\n"
"QcBfIWnbKQ4XRUeRm9kW47blGmVnqZlOyc/SBFfMlVhpQRRVeAMexVBagcmpKzgc\n"
"uy4xbJMReRGZ1ed9CzJ4XSmijRQfGGQy6abmSsqVJBjIcRhts2t5HsNlI/3wujni\n"
"PBnr/pFJipNRr4svFr0Rpx85oOmmcu0FmPR/GafM0VA1B1uhUOAMIUEI3lsQ2EYF\n"
"Wxc9XW7B0sItGn/DXoRCHNr6MXaOMULQOCxOqemM85qs+H8/HMs4g65gBqd5DJCl\n"
"7U777rWBqxfm1wfITMyylXVQ9WBcdxJOMfAYUPYuvmToHaDBG88IyNOzm4hK/2tB\n"
"gQpomX+yJGtBHDtN4ZH+rGzcTr7s/w5OcQD0vP6l6WXMRm8BWCvrunXRP2cNJQID\n"
"AQABo2QwYjAgBgNVHREEGTAXgg95b3VyLWRucy5yZWNvcmSHBH8AAAEwHQYDVR0O\n"
"BBYEFGD7Ggs1Deat+ebmUUExX0CCwVqzMB8GA1UdIwQYMBaAFPezdGmCW+jheous\n"
"gj3x7k7aLG7ZMA0GCSqGSIb3DQEBCwUAA4ICAQCLrXgepLKBNTOJIvSYvDTOJD4z\n"
"ViuYH4HZI0xVDvzeLo4Y9Jcu8qcxf7WMvjmq0TgO0hqtPrX3lXaUmMuSrDQT3+3r\n"
"2O4YdjxXQPwgAsvJRRE5DBsVPjRAKSNBFEutXFykUM0yyDPA8pHkhtjHuEaeUW6S\n"
"LKRWQQQctQ22tW94liBFlqwOGmot8Hl29hlEVLC+fanH8xowzXvsu6gBWC0IVm1X\n"
"90LzY5W07ZhY+Y0BWV+jm88wk1e1Ww5MyAEV43qaOUU7mzgimMQRwmgQHCkSygCb\n"
"51QlcX0ToFHIRL+ckvuzpg1mVdu0CK1IPgcFxV7l7oF4dt2fydcLlpV0R9SvJTrL\n"
"nVdyFeibyQ9+FaRRJiqVzaLFJICZvm6eozsZtD1t2CZYPTOqr856YMnIXQ3fsxJ9\n"
"PhVHHJT0JY2zNJLVzC2PIFIW0csG6yB0F+1w/uW481NtjO+TGWGSwqXsXfsxOx3C\n"
"kA/hslbsZVaf3hsQYDLn6sTXd6FKEDF9CjX/fpyT41fMbGNSZsHdx1G3hc3zgNlm\n"
"iExMviHNmB6WPJE6iK64K+nB5TxZicy9cGLZNO5dVMxiIfWVcUD+VvDdByhoVENq\n"
"SvpOK+hKmKawWXK3TkFXdrH+JdRodYF/VvZaCAyrY8RAyw7gECNe1eeU/yyewLOK\n"
"DT2og2l4Y4IfnMRACQ==\n"
"-----END CERTIFICATE-----\n";

// server.key
static const char *s_tls_key =
"-----BEGIN PRIVATE KEY-----\n"
"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\n"
"JQeacCHYEWiAtZWW7og2cxTuyg0UHA==\n"
"-----END PRIVATE KEY-----\n";

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_ACCEPT && c->fn_data != NULL) {
    // String way
    //     struct mg_tls_opts opts = {
    // #ifdef TLS_TWOWAY
    //         .ca = mg_str(s_tls_ca),
    // #endif
    //         .cert = mg_str(s_tls_cert),
    //         .key = mg_str(s_tls_key)};
    // Pem way
    // TODO: modify
        printf("\n before init \n");
        struct mg_str cert = mg_file_read(&mg_fs_posix, "server-cert.pem");
        struct mg_str key = mg_file_read(&mg_fs_posix, "server-key.pem");
        struct mg_tls_opts opts = {.cert = cert,
                                   .key = key};
        mg_tls_init(c, &opts);
        printf("\n passed MG_EV_ACCEPT \n");
        // free(cert.ptr);
        // free(key.ptr);
  }
  if (ev == MG_EV_HTTP_MSG) {
    printf("\nEntered MG_EV_HTTP_MSG \n");
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
      printf("\nEntered else case\n");
      mg_http_reply(c, 200, "", "Hello, World!");
      printf("\nSent simple message\n");
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
