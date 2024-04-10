To Run Two-Way TLS Server and Client (HTTPS)

```bash
cd http-restful-server
make CFLAGS_EXTRA="-DMG_TLS=MG_TLS_OPENSSL -lssl -lcrypto"

cd http-client
make CFLAGS_EXTRA="-DMG_TLS=MG_TLS_OPENSSL -lssl -lcrypto"
```

Certificates are generated using: 
https://mongoose.ws/documentation/tutorials/tls/#linux
Self-signed certificates

To gennerate certificates
```bash
./generate_certificates.sh
./format.sh
```
formatted_certs/ will contain relevant certificates
see server/main.c and client/main.c for how to import certificates