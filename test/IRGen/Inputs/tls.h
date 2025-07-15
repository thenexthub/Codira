#include "shims/CodiraStdint.h"

static inline __language_uint32_t _language_stdlib_gettid() {
  static __thread __language_uint32_t tid = 0;

  return tid;
}
