#!/bin/sh

# Define the directory where the certificates will be stored
CERT_DIR="../server_certs"

# Check if the directory exists, and create it if it doesn't
if [ ! -d "$CERT_DIR" ]; then
    mkdir -p "$CERT_DIR"
fi

# Run the OpenSSL command to generate the certificate and key
openssl req -nodes -new -x509 -keyout "$CERT_DIR/key.pem" -out "$CERT_DIR/cert.pem"

echo "Certificate and key have been generated in $CERT_DIR"
