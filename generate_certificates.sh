#!/bin/bash

# Base directory for all certs
BASE_CERT_DIR="certs"

# Specific directories for CA, client, and server certs
CA_DIR="$BASE_CERT_DIR/ca"
CLIENT_DIR="$BASE_CERT_DIR/client"
SERVER_DIR="$BASE_CERT_DIR/server"

# Function to create directory if it doesn't exist
ensure_dir() {
    if [ ! -d "$1" ]; then
        mkdir -p "$1"
    fi
}

# Ensure all directories exist
ensure_dir "$CA_DIR"
ensure_dir "$CLIENT_DIR"
ensure_dir "$SERVER_DIR"

# Common parameters
SUBJ="/C=IE/ST=Dublin/L=Docks/O=MyCompany/CN=howdy"

# Generate CA
openssl genrsa -out "$CA_DIR/ca.key" 2048
openssl req -new -x509 -days 365 -key "$CA_DIR/ca.key" -out "$CA_DIR/ca.crt" \
  -subj "/C=IE/ST=Dublin/L=Docks/O=mos/CN=me"

# Generate client certificate
openssl genrsa -out "$CLIENT_DIR/client.key" 2048
openssl req -new -key "$CLIENT_DIR/client.key" -out "$CLIENT_DIR/client.csr" -subj "$SUBJ"
openssl x509 -req -days 365 -in "$CLIENT_DIR/client.csr" -CA "$CA_DIR/ca.crt" \
  -CAkey "$CA_DIR/ca.key" -set_serial 01 -out "$CLIENT_DIR/client.crt"

# Generate server certificate
openssl genrsa -out "$SERVER_DIR/server.key" 2048
openssl req -new -key "$SERVER_DIR/server.key" -out "$SERVER_DIR/server.csr" -subj "$SUBJ"
openssl x509 -req -days 365 -in "$SERVER_DIR/server.csr" -CA "$CA_DIR/ca.crt" \
  -CAkey "$CA_DIR/ca.key" -set_serial 01 -out "$SERVER_DIR/server.crt"