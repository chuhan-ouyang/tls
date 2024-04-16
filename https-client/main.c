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

// client.crt
static const char *s_tls_cert = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDDzCCAfcCAQEwDQYJKoZIhvcNAQELBQAwSTELMAkGA1UEBhMCSUUxDzANBgNV\n"
"BAgMBkR1YmxpbjEOMAwGA1UEBwwFRG9ja3MxDDAKBgNVBAoMA21vczELMAkGA1UE\n"
"AwwCbWUwHhcNMjQwNDE2MTg1MDM4WhcNMjUwNDE2MTg1MDM4WjBSMQswCQYDVQQG\n"
"EwJJRTEPMA0GA1UECAwGRHVibGluMQ4wDAYDVQQHDAVEb2NrczESMBAGA1UECgwJ\n"
"TXlDb21wYW55MQ4wDAYDVQQDDAVob3dkeTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBAJK/BRhwaqtu+/+Ewg2jbwdvMlCG6yuHrHvV+htVgjYum6v71Lzp\n"
"DBPFIw7HvJUpsfh5ruycuOYzmhvdk4t2sq+RGKDuD/E0oqdyYAWK4g3Ij1spEHSQ\n"
"jfFeuYhvHtBH3Ql+GmZojNufhyIG4ZTQOY0O0VLn4sA8l8lKuyIaPWRNuxuncBxY\n"
"aG5Eh3n0R6hLQKwv3KbFID+rIjCsTfW+tbJc29xozpBO6tfZaLkCIqroYNY8+zyB\n"
"vXP+0dOdyo9djN4Tco2EmSVYmjvs0pyhR1iaeXbGLYpE/R+mVBsh1ibR9MNv62go\n"
"fw7Mjhmsz6xuWLnh8Ijlwgi9vpPtnahNKbMCAwEAATANBgkqhkiG9w0BAQsFAAOC\n"
"AQEAOR/FVoXwGY0rq163z0hgBRi/kdzNxcaoLy2wTdXknrfkchbigS3wkApF/iEc\n"
"koA3T9+qvpfuLPJwRsHgysc+hSCWlCkeaug5OfQ+uue6RGYQxONrUY9vcuIU+SP4\n"
"LkBAmfJSsT+wItRIjVL23Ql8OAjzOuYAAVKU0AOlnErtgvhBnfG2jeUgQmZcrL+x\n"
"UiLirxbzC1hbD+xHtR+/mVORsniQTXHnmWrFa66Kyq95X1q72Vur3eg1jLdGr30R\n"
"MkQPuqfjLy5xFdnuR8I6ZmDogPf/XY9NwuUha2o/Gjd++T0dSXRzFeixJwfDYhBI\n"
"sJC0704FKdxJBlaJJSvw3gG9lg==\n"
"-----END CERTIFICATE-----\n";

// client.key
static const char *s_tls_key = 
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCSvwUYcGqrbvv/\n"
"hMINo28HbzJQhusrh6x71fobVYI2Lpur+9S86QwTxSMOx7yVKbH4ea7snLjmM5ob\n"
"3ZOLdrKvkRig7g/xNKKncmAFiuINyI9bKRB0kI3xXrmIbx7QR90JfhpmaIzbn4ci\n"
"BuGU0DmNDtFS5+LAPJfJSrsiGj1kTbsbp3AcWGhuRId59EeoS0CsL9ymxSA/qyIw\n"
"rE31vrWyXNvcaM6QTurX2Wi5AiKq6GDWPPs8gb1z/tHTncqPXYzeE3KNhJklWJo7\n"
"7NKcoUdYmnl2xi2KRP0fplQbIdYm0fTDb+toKH8OzI4ZrM+sbli54fCI5cIIvb6T\n"
"7Z2oTSmzAgMBAAECggEABgf2yCXapHgXwE0vAot8hTQzP68D3FKjqqVFNblidCKf\n"
"QEbF3AvB9NAN3E4mOIUxrNInd88LJF7rjwigx0MWHfQbLIYMPrAYnUQsoR2vo7xh\n"
"8++wnvHP+6MWgf57KDlD4t99a0cRvvcEAUiCHfEe5Ga/AKbKIXd8D4hUP5cPfd9K\n"
"CdPSiJEUO5z8b31JMFi/W6NKlqK/4qNm3oDB15woRXwkfFUFJJ2QedRQ+MaRspRH\n"
"weM/UyXklt6TKfqi9JsODeQBT6XgnBJM1JUId6Dk5+FaZi5CsCBUk4Bk4sRAOVwc\n"
"CuH1abBEGXFcIJJY04DUlfK6wdlcSRb6yPELG6CKZQKBgQDEtFiyTI2Q+vnUlU6v\n"
"zBjW9/Zd+6AtofdHJkMk+JNFq/lSKgkUqhJZnzDmLht2guGSU8IW3qTsaLK0ftvl\n"
"IjEqpfvOGKlhxnWy+unoZybNNOkFtFKiGVme7vApKpQs2HSIoDcUY0ZtWZEQ0qa3\n"
"lu5QBU6ubSh4+rPzrzfV2HuNVwKBgQC++2YWf3hEistV8VyVL7LkdreYn1Oo33IJ\n"
"hqsWa6huZUxNcQfogWAnBSoSwzXk1fVht18SVIcQjkyiVuozbysV21L6XIHZzhiQ\n"
"ldnNxoc1rpMne93EtzPRx4BJ6Jqu5zX9FHFQtVyyEv8tcoI7EwOdCr/07OtSOzTP\n"
"y66x74BxBQKBgQCfa7bzEp4toWr3gubaDaUYHLE8rGG+eiVKMdV7Ukcx0VZo5A6m\n"
"A8yJfD6KX1X8r8iX6nBETsAFpjknliUryJFmik3RVCV7S/g6Tadjpq5I5Bfzepj6\n"
"LWd4KxjTlGGxEuEnlgnt6SO8Hjo3EzLxWk7c5eBF98w0eKN93UV4eFJyUwKBgCEQ\n"
"QQJQGzAynzVHzfF+CoiHDQ39N2ybEpEXoXMGPGiwXN9jXC0iiNT4jeolpaTybcL3\n"
"DhyMSb3SBO5uAanZ6ymjiOATN9/PUG1npAd+AVaH3K4PQbuWw//jfBBPlsgE+1HP\n"
"Qjbozx19MHRbyLQVfKCphcYPZkYcPe5jouDnR+NJAoGAZDJ4C7AsO5TqbdLiHCXq\n"
"VBFPpoh9xCeiYGaFzALXl+QiQYYddSEX/QZcXxncvgdTW2fXghyVcvZ2oJxESx+9\n"
"7ZVWVFFf0QI5RJIGMS5OcxGiA0MqF7RER14850EGHz5mtLLiZIpGHqbZ4QMZ02rV\n"
"0C7ixQc0xwS5GOobMHeH050=\n"
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

    if (mg_url_is_ssl(s_url)) {
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