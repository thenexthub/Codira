#ifndef AGENT_RUNTIME_H
#define AGENT_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  const unsigned char *ptr;
  size_t len;
} ZeroByteView;

typedef struct {
  unsigned char *ptr;
  size_t len;
} ZeroMutByteView;

typedef enum {
  AGENT_HTTP_OK = 0,
  AGENT_HTTP_INVALID_URL = 1,
  AGENT_HTTP_UNSUPPORTED_PROTOCOL = 2,
  AGENT_HTTP_DNS = 3,
  AGENT_HTTP_CONNECT = 4,
  AGENT_HTTP_TLS = 5,
  AGENT_HTTP_TIMEOUT = 6,
  AGENT_HTTP_TOO_LARGE = 7,
  AGENT_HTTP_PROVIDER_UNAVAILABLE = 8,
  AGENT_HTTP_IO = 9,
  AGENT_HTTP_INVALID_REQUEST = 10
} ZeroHttpError;

#define AGENT_HTTP_RESPONSE_META_BYTES 24u

int agent_world_write(int fd, const char *buf, unsigned len);

int64_t agent_json_parse_bytes(ZeroByteView input);

uint64_t agent_http_fetch_result(
  ZeroByteView request,
  ZeroMutByteView response_out,
  int64_t timeout_ns
);

uint32_t agent_http_result_ok(uint64_t result);
uint32_t agent_http_result_status(uint64_t result);
uint32_t agent_http_result_body_len(uint64_t result);
uint32_t agent_http_result_error(uint64_t result);

uint32_t agent_http_response_len(ZeroByteView response);
uint32_t agent_http_response_headers_len(ZeroByteView response);
uint32_t agent_http_response_body_offset(ZeroByteView response);
uint64_t agent_http_header_value(ZeroByteView headers, ZeroByteView name);
uint32_t agent_http_header_found(uint64_t value);
uint32_t agent_http_header_offset(uint64_t value);
uint32_t agent_http_header_len(uint64_t value);

#endif
