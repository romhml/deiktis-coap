idf_component_register(
  SRCS "connect.c" "server.c"
  INCLUDE_DIRS "."
  EMBED_TXTFILES certs/coap_ca.pem certs/coap_server.crt certs/coap_server.key
)
