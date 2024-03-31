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
static const char *s_tls_ca =
  "-----BEGIN CERTIFICATE-----\n"
"MIIDczCCAlugAwIBAgIUbR8meFcE6oalFn57GpQjDNu5qfQwDQYJKoZIhvcNAQEL\n"
"BQAwSTELMAkGA1UEBhMCSUUxDzANBgNVBAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9j\n"
"a3MxDDAKBgNVBAoMA21vczELMAkGA1UEAwwCbWUwHhcNMjQwMzMxMjA0MzQwWhcN\n"
"MjUwMzMxMjA0MzQwWjBJMQswCQYDVQQGEwJJRTEPMA0GA1UECAwGRHVibGluMQ4w\n"
"DAYDVQQHDAVEb2NrczEMMAoGA1UECgwDbW9zMQswCQYDVQQDDAJtZTCCASIwDQYJ\n"
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBANC55Qj4AZMOpfqnUEizCp+0ZYgeYhaE\n"
"4CFtQ5V3z8kt8ulbeAG7tUkMFfoqJgilzkQXJ+Ij5K+raXUlPScIPDi8stx27SiE\n"
"4oU7nTRTRkSVauKchknWm8IBCxI22s/HpY9yaek02iQa6ZUXOPi47sMnD95ir4Ie\n"
"Q4silWBi2mQ+hmJbIdmAYUL0EvSZFL1Rsu5hDAqBZADiGWTzFMifI8xyjWjphhuf\n"
"CwtwP/MAwuk41WD3dXmhGEdq5FRpr88aozth/AU/tV57NOPjKQ0Jk3I6GXiVdKSw\n"
"fF6JkEUoX0JYydV0WFwfLAAlUZ8c8+lw89/CwAjx2xqh2kEAD5R3BfUCAwEAAaNT\n"
"MFEwHQYDVR0OBBYEFPvGvQKJLHeSKu40OblNcBB0JWJGMB8GA1UdIwQYMBaAFPvG\n"
"vQKJLHeSKu40OblNcBB0JWJGMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
"BQADggEBAEZKrefG0c9UZNK1Ves1xKK0Si9Paf/0BUzVqod7b/zo7AkDOunVeTcJ\n"
"IBo8XZUtUtmrTy2IQ5Sm6HCOw6V2OgLl1/VojlhdKyeJi8cWwOMvRL2OcgqG17SD\n"
"KUkj9h89uMi32xsVdkR/OtI4G9kvXPzscihGCoQ3frtRPI65kjoWDAcjjTnjqBwp\n"
"oCSiHSpzdCr5lPzuXei0ceHNYrYciBxrme4obacKKLpU1gLz67Moq1kGZKAzYOc0\n"
"QjOrVpdQCwHMDSD3DMnrdeb3Et9pPGW7BRJDpMkWZW5R5RfGlHQMCTwJeadN5gEZ\n"
"c3QdltNMdK2CnpFcO7FCUl/PTSmRb0Y=\n"
"-----END CERTIFICATE-----\n";

#endif
static const char *s_tls_cert =
"-----BEGIN CERTIFICATE-----\n"
"MIIDDzCCAfcCAQEwDQYJKoZIhvcNAQELBQAwSTELMAkGA1UEBhMCSUUxDzANBgNV\n"
"BAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9ja3MxDDAKBgNVBAoMA21vczELMAkGA1UE\n"
"AwwCbWUwHhcNMjQwMzMxMjA0NTE5WhcNMjUwMzMxMjA0NTE5WjBSMQswCQYDVQQG\n"
"EwJJRTEPMA0GA1UECAwGRHVibGluMQ4wDAYDVQQHDAVEb2NrczESMBAGA1UECgwJ\n"
"TXlDb21wYW55MQ4wDAYDVQQDDAVob3dkeTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAMALomoyClyK6SjMW9u6k6utX3O+2EGomc+Rat3EISzZzuXsS7Ul\n"
"qec/RUGqxwH/TECW7RpzBDLkGDmDnUL2M+36gj+/FNpwVhcTsBo+eE7ccQ0v+zV6\n"
"Q0t8UZyymAv8/vQ8WjPAQjxa5iePE7+CyAG4iGn8PxbYPQRYudtOEgkuuoA8e9lP\n"
"fGh1fM0GwYoVuO20RbbQ+EQonNDQMjKQMl8urtT6E7MUT5BgZF8zMy5FbXa7wCyT\n"
"+Hob9c4XVMTl2sR2bNgCVhzy0zJA19JWU9rpGs7g7wFAtL+oHKdxc7za1tWWYECI\n"
"+kq2+8DOTth2QtA2vWPXZQgiSnaUwqSfDBECAwEAATANBgkqhkiG9w0BAQsFAAOC\n"
"AQEAeZdr3sfmeaLqpbKpHKvsBgJO3vDHrnk0XwMr7MwZqFrDdjsG9aze8RP1F+kq\n"
"99Uu//jow44uOiVUwHcF/ywhreV6lFbNTc2PRymEILZoQYzsmF3KzCOAeThVyMb/\n"
"RrepbqZfBjuSoyP7KeJT3sczFUJsAthhvb3D+nRMNuafzqDyRA2UQS3eBxA/uytc\n"
"7yCXk+pKdEXVwmZBz97q1Ew47QBSJetpxT2Fa6LdtOdA8irKXlYkBc95di9ayOpF\n"
"aQ1tcpV/tiE7DRcf2BEd4GaDpbKrNdKxJJ1pyR7uAcKzMuXtH+yj85CYfZiIcqmb\n"
"2APF7m1qp9uwGFK2wzZNjS2ycA==\n"
"-----END CERTIFICATE-----\n";


static const char *s_tls_key =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDAC6JqMgpciuko\n"
"zFvbupOrrV9zvthBqJnPkWrdxCEs2c7l7Eu1JannP0VBqscB/0xAlu0acwQy5Bg5\n"
"g51C9jPt+oI/vxTacFYXE7AaPnhO3HENL/s1ekNLfFGcspgL/P70PFozwEI8WuYn\n"
"jxO/gsgBuIhp/D8W2D0EWLnbThIJLrqAPHvZT3xodXzNBsGKFbjttEW20PhEKJzQ\n"
"0DIykDJfLq7U+hOzFE+QYGRfMzMuRW12u8Ask/h6G/XOF1TE5drEdmzYAlYc8tMy\n"
"QNfSVlPa6RrO4O8BQLS/qByncXO82tbVlmBAiPpKtvvAzk7YdkLQNr1j12UIIkp2\n"
"lMKknwwRAgMBAAECggEAF7sYUGvAkLDwS4Sn294RU1yIIq20MWDb03oSV4xd1m9P\n"
"vA+tR0QmhkHGQBgR9rLibEokyxOSsusxUczBXmiSqtc9cxJx9DrUwziWlpeeQY3H\n"
"QSxAVtpgfcUYnKFhB9I1JbReJ8Ob1l4bz0rvKMdzXusQ8u/Np/gD34lLO3NJPUMG\n"
"nOnvMufWXziBAHF3z/DCVf4N1EmBvoY3wVWCxsXhkwHKhd3+V0pnqEn4XRKBGKKn\n"
"CRmI76quA+c0L2S8Qy05ag8TtTox2ZRxj1Ifqf3XAvboNb3kQW8ZRFrQVq0Kg2G9\n"
"rbS4fovBIp7h/dmb+sdWRWLWUbxG1Jdg95+4xFR9lQKBgQDAp5IEpdAc6TZhliDG\n"
"T6N0o9tgUtsi4LKl47b6tR+J/gwe7ridRDh8KwzgBMo/NjQbko8EURYPntFkPxrV\n"
"o0RW6ZGfxVVlVJaFXCzYm1ay6gL6aZLuMxAasMUDGazL52+WvuP5tKDaVagC+Bx4\n"
"C24jQXLLvtkwTsD83n0VDqDZVwKBgQD/MMq0q13zcoLST0GFICYNqUUWKqtzacNh\n"
"91ULDnEpv14iB2OmKvnToHOPmE8x5DPZmLXyie+YKnB9LpA6TjVwHCqYkTgfkG4+\n"
"kJh7IMflyVA9mvI0HyABM9acyfyMl6GxoiS2NZ7m2De948b4r8f4t0mw0qx7p2L1\n"
"+kbkqXUc1wKBgFg/af9/BKl5mlNxNWohqLoRJLgmvAJp8utSN8r3lI7dNCgnKaHA\n"
"Ui9ueONUzl/OMGhA5goQKo7VLQ1CasRZmUBGlVIeTlOwP5Ku+kvuO/SQhPS3z3oV\n"
"IddYCaa16BUu2kqu/javqs7YVbGmfUPv2G+F11PFBC/Ao14xfBJdNQjZAoGAB7+t\n"
"iArkEW5atpQdgzNUtHl9VYz6I8kEW8cs1+caXJApBZN/3R3U9C4dFDElxngNcwAG\n"
"FlVF5zLys3pkesDBMqRITtWqBJtYHaI6XxLBvyJ7w0MfM6H2UNMIIz6m4oYALOql\n"
"frrmFlBecO+nrQl5Yv39yNa80gxlHt74pBzZJ4UCgYAe9VCTiNVuvq85IRRRhq35\n"
"RCV9iJ3YgDwQ58YxGAo7+lYclW17yccQiHjbW9XRMjUcrGTppT5VHqiksjtewtOb\n"
"JqXy5wtOR8RHodVRCR/4fr1hrOZ7UTtbjFV0zmVkXZQdduqzUy1onGUbou0en5Zr\n"
"V9qgYaBsj2EefGYMs+VFbw==\n"
"-----END PRIVATE KEY-----\n";

// We use the same event handler function for HTTP and HTTPS connections
// fn_data is NULL for plain HTTP, and non-NULL for HTTPS
static void fn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_ACCEPT && c->fn_data != NULL) {
    // printf("Got to MG_EV_ACCEPT");
        struct mg_tls_opts opts = {
    #ifdef TLS_TWOWAY
            .ca = mg_str(s_tls_ca),
    #endif
            .cert = mg_str(s_tls_cert),
            .key = mg_str(s_tls_key)};
        mg_tls_init(c, &opts);
    // struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/ca.pem"),
    //                             .cert = mg_unpacked("/certs/server_cert.pem"),
    //                             .key = mg_unpacked("/certs/server_key.pem")};
    // mg_tls_init(c, &opts);
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
