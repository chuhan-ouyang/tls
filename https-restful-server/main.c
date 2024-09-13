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
void handle_get_request(struct mg_connection *c, struct mg_http_message *hm) {
    // The body of the request is expected to be in the format ['string']
    char *received_string = mg_json_get_str(hm->body, "$[0]");  // Extract the first element in the JSON array

    if (received_string) {
        // Print the received string to the console
        printf("Received string: %s\n\n", received_string);

        // Send a response back to the client
        mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "Received: %s", received_string);
        
        // Free the dynamically allocated memory
        free(received_string);
    } else {
        // If parsing fails or the format is incorrect
        printf("Failed to parse the incoming data.\n");
        mg_http_reply(c, 400, "", "Bad Request\n");
    }
}

void handle_put_request(struct mg_connection *c, struct mg_http_message *hm) {
    char *key = mg_json_get_str(hm->body, "$[0]");  // Get the first element in the JSON array
    char *value = mg_json_get_str(hm->body, "$[1]");  // Get the second element in the JSON array

    if (key && value) {
        printf("Key: %s, Value: %s\n\n", key, value);
    } else {
        printf("Failed to parse key or value.\n");
        mg_http_reply(c, 400, "", "Bad Request\n");
    }

    free(key);   // Free the dynamically allocated memory for key
    free(value); // Free the dynamically allocated memory for value
}

// Renamed event handler to fn
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/get")) {
            handle_get_request(c, hm);
        } else if (mg_http_match_uri(hm, "/put")) {
            handle_put_request(c, hm);
        } else {
            mg_http_reply(c, 404, "", "Not Found\n");
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
