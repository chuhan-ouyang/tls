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

extern "C" {
#include "mongoose.h"
}

#include <cascade/service_client_api.hpp>
using namespace derecho::cascade;

#define TLS_TWOWAY

static const char *s_http_addr = "http://127.0.0.1:8000";    // HTTP port
static const char *s_https_addr = "https://127.0.0.1:8443";  // HTTPS port

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
"MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDtaQbgABdIFVMl\n"
"crbzcFqbgZD+kOS03dNTJIjBo4HYESIRPsONEuiEZ8sixeTOiRrM1xH4Vwy6ogLV\n"
"3+xwWfBdd+kd8LLG2aQSlp8PA6TMYSzguNIZArAEmEKFP7FLnN5hZE6dFn36K6GO\n"
"GpQkk5ZPcBMEN9j6h6xgUDpPRHgLn7RH4FReXkSs6my+zKtbxe9j0/UJhxP2Q8aC\n"
"PS4lO+kCPEGtBHT9EXNDcjYF3WeJAU4JGjF7FGAu/E5y0Z537rpfnHRN4DECjNt3\n"
"lH1ldTUCamkaxumiV2om9yveTaGKEKZRiLrVssFDVhL71YAjK1GSj3hY+KGVC47/\n"
"0SXPP8vlAgMBAAECggEAdDYE94ET5GUL3jjY9TD6KUSzBOlICoDLANfRWXgABxPb\n"
"6TUf495tkuJ7vPnBlpfmVn6wfS+fO3wyhx6lThm4XwhjRBSeblf6yF6SXXcbJDAt\n"
"F5NgxH1cS+N8LF2RG8Id2O6QzLRF1j+lRTodagoUea4SoO31eC0RY9Phtwt897Kz\n"
"2W97yjvXw07kS8Q8ypvLJ+KI089ZcakTXjsIzpy4oyzNxz6tucND5rnCcNitkr7H\n"
"0cBzrP0dqEhpPkpwvawnA0GAqvxJArfnPgSF8pphX2z1O/yt8trVGHHtplJEC1lM\n"
"7oF5bBvF5L+Ufjcct7tTiLo0HhdgWQaw9Rdltuc1wQKBgQDybCNafaguVwo4OI9N\n"
"y0PC/wOreF5QkY3+NSN/JQwL1la1ScnjH0JAPKGCBtEtEFbNFA6bGuS74mQQ1NvN\n"
"g/g05seKbJFO/XCIksqOmZOzlNjd6X79rukFShI988TaeTUMbFMsZssG0/Qu2d5I\n"
"FKtoG7DHmq4Bh1GJZiCI4x36AwKBgQD6tQY5IUHBDSwdVD+Ybz4zg7sWME/bAZwU\n"
"FdPJOdrFS8YA7YkNxvJUhuXTMYVITCJwe21Y+0/cLiiCAB4LHFW73mXMzAJ/dVWz\n"
"X4YaR1h47D2/eGJFttcoD4cJC9wSCJRquAXeQMVdvBMqgJy71RtWV/07NfbhCjkc\n"
"voXhoNMx9wKBgQCvDGze29uEUq97d0PrPbpZZMFmoM0hymjIxeAy4DJ5nbUQxQ51\n"
"5U12ixFojbmfm6vB/Bir4ViHXus4yCtLlF62/Gnbx5eJfTI0PCLto3Hm+7U5UHC1\n"
"iAaVHi5b89kSlPE8XkAU0MkLziMFbaa3T82RqGUXTNcXHXpAB7G4hp9PtwKBgQCh\n"
"2Z3z4eF2RQ+2u3325dItJZzxHS1ggLCHUvQP03bFqj5guPii2U32fcP/TNfzrZsK\n"
"LNSR/mQ3C+GA05yRpbA/gbsMtKPkLxatnzyO2IJoRnMymgAva3OJXETb9TkUt09B\n"
"eh3ntr7z3YguByuoI1LrshA8WoFQFO57/Q+nctEFTwKBgQDJBSy82wDoMLkkHCsc\n"
"wmkPa06qFQ453brwOplHvH9fhEUPHmELx/cu16frbMD0G5uxyvweh0mmX4ImGavL\n"
"wfZzrx5FqROJ359o71NiSLzHZrKCwR6Ecr4pXpSPzyTgtM5m1R9L9GAGhFzSqyFV\n"
"t7Hlg6z7p7csKxNvI3sJmbM6/A==\n"
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
    if (mg_http_match_uri(hm, "/cascade/put")) {
      // parse key value
      struct mg_str json = hm->body;
      double num1, num2;
      if (mg_json_get_num(json, "$[0]", &num1) &&
          mg_json_get_num(json, "$[1]", &num2)) {
        // Success! create a JSON response
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{%m: %g, %g}\n",
                      MG_ESC("Received Key Value"), num1, num2);
         }
      // capi
      // message
    } else if (mg_http_match_uri(hm, "/cascade/get")) {
      // parse key
      struct mg_str json = hm->body;
      double num1;
      if (mg_json_get_num(json, "$[0]", &num1)) {
        // Success! create a JSON response
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{%m : %g}\n",
                      MG_ESC("Received Key"), num1);
      }
      // capi
      // message
    } else if (mg_http_match_uri(hm, "/cascade")) {
      MG_INFO(("Matched to /cascade"));
      mg_http_reply(c, 200, "", "commands: /cascade/get, /cascade/put\n");
    }
  }
  if (ev == MG_EV_TLS_HS) {
    MG_INFO(("SERVER TLS handshake done! Sending EHLO again"));
  }
}

int main(void) {
  auto& capi = ServiceClientAPI::get_service_client();
  struct mg_mgr mgr;                            // Event manager
  mg_log_set(MG_LL_DEBUG);                      // Set log level
  mg_mgr_init(&mgr);                            // Initialise event manager
  mg_http_listen(&mgr, s_http_addr, fn, NULL);  // Create HTTP listener
  mg_http_listen(&mgr, s_https_addr, fn, (void *) 1);  // HTTPS listener
  for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
  mg_mgr_free(&mgr);
  return 0;
}
