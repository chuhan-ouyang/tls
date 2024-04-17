Run Two-Way TLS Server and Client (HTTPS)

```bash
cd http-restful-server
make CFLAGS_EXTRA="-DMG_TLS=MG_TLS_OPENSSL -lssl -lcrypto"

cd http-client
make CFLAGS_EXTRA="-DMG_TLS=MG_TLS_OPENSSL -lssl -lcrypto"
```

Generate Certificates
```bash
# gen ca, server, client certs in cert/s
./gen_certs.sh
```
server/main.c and client/main.c import certificates from certs/

Certificates are generated using: 
https://mongoose.ws/documentation/tutorials/tls/#linux
Self-signed certificates

Enable Browser HTTPS Connection
1. Method 1: Direct download in chrome and enable (Recommended)
2. Method 2: Use script
```bash
# run in docker container
openssl x509 -outform der -in /root/workspace/tls/certs/ca.pem -out /root/workspace/tls/certs/cascade.der

# run in server
docker cp cbdcdev:/root/workspace/tls/certs/cascade.der ~/cascade.der

# run in local machine
scp cbdcdev:~/cascade.der C:/Users/<>/cascade.der

# in Command Prompt, run:
certutil.exe -addstore root C:\Users\<>\casacde.der
```