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
static const char *s_url = "https://127.0.0.1:8443";
static const char *s_post_data = NULL;      // POST data
static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds

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
"AwwCbWUwHhcNMjQwMzMxMjA0MzU4WhcNMjUwMzMxMjA0MzU4WjBSMQswCQYDVQQG\n"
"EwJJRTEPMA0GA1UECAwGRHVibGluMQ4wDAYDVQQHDAVEb2NrczESMBAGA1UECgwJ\n"
"TXlDb21wYW55MQ4wDAYDVQQDDAVob3dkeTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAKhp5R4zmcCpahEuqTvbf3xqTcjFe9dw+6jq8QCB7R4TvS2fP50a\n"
"QVR/TtUfJgVupIY8NeKT3vL+i4Me6UdGOMOYb7mHNDxBV29nd9ys5SC7/nM9E8Bm\n"
"wZDwkxKXRelIUkRS4iBWDF2w2KL8EjkqP9DIrlKX9PEPTjVl5E4JvZ+cekmH65dR\n"
"Y+TURyCXgGapIiz3SJB9TBX8DDdrX+nCM+BrPFvMb4GxtAkNG8yJEFUk51WRu6jg\n"
"Tc8f4YDd3laTuuU4qW9QOCC7/H5aa+bIUaPzbCWZuaViYlI7cqTy+Nwr+Kg84vJt\n"
"SU0NSwltiEH/aSSVEDkTqvqGoDuaT58u66ECAwEAATANBgkqhkiG9w0BAQsFAAOC\n"
"AQEAJ6AOhbnsq7uK40xF89L6FcrVDJ98V8siguCeR2KvL9PymyyzzciK66nUX6rx\n"
"nEpYQEzj5q/uXqP7w1shzrcvptivORiWXSyuL6pDgbdodeisYPS1TNsK7Um9wX5a\n"
"NREjxig5i4fUfUP1hTnxtmOCf0Uq4Z1VKfYmNt+mgzm8EQaRLo+Iepqrt0zCIj7/\n"
"l5voYxaCYfdOfEcH40i9UvJIXaKKuIUN9MDJ2nKFod8vQrlG78iJWdn6AvNa6RLv\n"
"0h4Pzqw2v/tTAvkeEWpVWGHxfshQMDZs9rZSQBMM0Bonz+pjMIkD9ONR5cFew+xI\n"
"9deWyacZ6oesadpWAbK5rMQqaQ==\n"
"-----END CERTIFICATE-----\n";

static const char *s_tls_key =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCoaeUeM5nAqWoR\n"
"Lqk72398ak3IxXvXcPuo6vEAge0eE70tnz+dGkFUf07VHyYFbqSGPDXik97y/ouD\n"
"HulHRjjDmG+5hzQ8QVdvZ3fcrOUgu/5zPRPAZsGQ8JMSl0XpSFJEUuIgVgxdsNii\n"
"/BI5Kj/QyK5Sl/TxD041ZeROCb2fnHpJh+uXUWPk1Ecgl4BmqSIs90iQfUwV/Aw3\n"
"a1/pwjPgazxbzG+BsbQJDRvMiRBVJOdVkbuo4E3PH+GA3d5Wk7rlOKlvUDggu/x+\n"
"WmvmyFGj82wlmbmlYmJSO3Kk8vjcK/ioPOLybUlNDUsJbYhB/2kklRA5E6r6hqA7\n"
"mk+fLuuhAgMBAAECggEABqJWQtJuSvU5fwgHJVscCmT/H6qs2lOibDjmCbq0gh/t\n"
"rrWEWl5RhsA1lSZjF2lwfHO0fTKNQKX3jbCa6KJ647SWaRjq/TknNjWdloEujPOu\n"
"3jAAnk6F7z1U09GG8N23FOctbm/pUIsB+6ihmKQiC6nOGSWQger0Dh0viUXvtPDz\n"
"CYgPW5pLdHIMIumJ1kmaTCq7LDC5aHZCQsGukaCTDCG15rO1tOT/r0GUf85DhNoo\n"
"nA1LPsFHDUNbmL/a+WVcRPTEAjyMdd0RxJ+pUdc4S1q0n3C0XJe43a+pDhU1q8lO\n"
"7QdGahuMg6kW0C2gdyYaX/7TY8oAdBPHczVYRX00OQKBgQDdLwVtkOULysA4Jloh\n"
"i3jW5pJhG8cWhSD+i1Ct3aatAL1+ahHpcb6eDk9Z8prQObpWH8n2neggjf3enLVp\n"
"qBWzqe1QD9Yd857onBaQywiaGzUAXJ7sLQnsg990PN0XG3cRCZriwPYbEc0LCSK6\n"
"uR5aMQiJrIupdihoX8vV6vBUeQKBgQDC7GoVo/1NKGUKjm7T7BbjPUS0ZSQe9pE6\n"
"0H16f4SvAG0fPluSiLV8XTU/ULz/G2k7ecpeOBLbs6ZIcWg7cKTPIuWcrd8at8oH\n"
"TuqzwMz6mQTUHTw4RpH0IrgXeHVM6RSGmwF/8DhpP906AzLacrJ5eqmamwUM5ocx\n"
"v705IPD2aQKBgE7DseEkmgwq9WyJa8tKSYvtq+7c33bLjcZXgjGIHxS4kMJ1r088\n"
"bC8I1YUoeazxAcQtqdQiw0jkh6VAlPG9I41JU/MQ0KT13n7jJr4FL1kBm0w3r3hX\n"
"SZNqGqbgDbNu2mW8z+Uz5fAZ1LS7Why4U3C2yV2NjmaYZydUBdh1w4OxAoGAZwvA\n"
"UUxuXlnagKgnotUOjHcZWRyW6d+Di0oo65d6N2HMTlYS5vZqqyJDQgOWxm5hRVXv\n"
"S7vtDER+xZFefv3fwrk6FYg9h8xde1uW7Awi0OAQE2qZKU71XdD07UN7D3uwpDBR\n"
"rKb7y3g9pauHOMCl5V6ZRuho1RcRpZUtnYPynvECgYByNpn0sB4sZO5iuXtjYp0u\n"
"KaGdl5xm6Ng5ndy5MZU0vANx5kPnt1p+1lX3EOzfSoc5Dcf374xeAELC2K3Fr23j\n"
"fuO8aBA11taXPcD9FgN8gOPwKFbFZCJxnJzGJlbnFu+UklVp7DIj91YL1OE0EyIw\n"
"za13xnLJNRCTakIGLLvJ3Q==\n"
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
    printf("\nMG_EV_CONNECT\n");
    // Connected to server. Extract host name from URL
    struct mg_str host = mg_url_host(s_url);

    if (mg_url_is_ssl(s_url)) {
      printf("\nDetected ssl\n");
      // struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/ca.pem"),
      //                            .name = mg_url_host(s_url)};
      // mg_tls_init(c, &opts);

        // struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/ca.pem"),
        //                      .cert = mg_unpacked("/certs/client_cert.pem"),
        //                      .key = mg_unpacked("/certs/client_key.pem")};
        // mg_tls_init(c, &opts);

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
    MG_INFO(("TLS handshake done! Sending EHLO again"));
    mg_printf(c, "EHLO myname\r\n");
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
