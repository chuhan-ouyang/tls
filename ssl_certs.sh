# generate CA key
openssl genrsa -out ca-key.pem 4096

# generate CA cert
openssl req -new -x509 -sha256 -days 365 -key ca-key.pem -out ca.pem

# check CA contents
openssl x509 -in ca.pem -text
openssl x509 -in ca.pem -purpose -noout -text

# generate server key (server-key.pem)
openssl genrsa -out server-key.pem 4096

# generate server csr
openssl req -new -sha256 -subj "/CN=Cascade" -key server-key.pem -out server.csr

# set dns and ip config
echo "subjectAltName=DNS:your-dns.record,IP:127.0.0.1" >> extfile.cnf

# generate server cert (cert.pem)
openssl x509 -req -sha256 -days 365 -in server.csr -CA ca.pem -CAkey ca-key.pem -out server-cert.pem -extfile extfile.cnf -CAcreateserial