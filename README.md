To Run Two-Way TLS Server and Client (HTTPS)

cd http-restful-server
make CFLAGS_EXTRA="-DMG_TLS=MG_TLS_OPENSSL -lssl -lcrypto"

cd http-client
make CFLAGS_EXTRA="-DMG_TLS=MG_TLS_OPENSSL -lssl -lcrypto"

Certificates are generated using: 
https://mongoose.ws/documentation/tutorials/tls/#linux
Self-signed certificates