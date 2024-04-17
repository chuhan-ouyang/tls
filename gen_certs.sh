#!/bin/bash

# Create certs directory if it doesn't exist
mkdir -p certs

# generate CA key
openssl genrsa -out certs/ca-key.pem 4096

# generate CA cert
openssl req -new -x509 -sha256 -days 365 -key certs/ca-key.pem -out certs/ca.pem

# generate server key (server-key.pem)
openssl genrsa -out certs/server-key.pem 4096

# generate server csr
openssl req -new -sha256 -subj "/CN=Cascade" -key certs/server-key.pem -out certs/server.csr

# set dns and ip config
echo "subjectAltName=DNS:your-dns.record,IP:127.0.0.1" >> certs/extfile.cnf

# generate server cert (cert.pem)
openssl x509 -req -sha256 -days 365 -in certs/server.csr -CA certs/ca.pem -CAkey certs/ca-key.pem -out certs/server-cert.pem -extfile certs/extfile.cnf -CAcreateserial

# generate client key (client-key.pem)
openssl genrsa -out certs/client-key.pem 4096

# generate client csr
openssl req -new -sha256 -subj "/CN=Cascade" -key certs/client-key.pem -out certs/client.csr

# generate client cert (cert.pem)
openssl x509 -req -sha256 -days 365 -in certs/client.csr -CA certs/ca.pem -CAkey certs/ca-key.pem -out certs/client-cert.pem -extfile certs/extfile.cnf -CAcreateserial
