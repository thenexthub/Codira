#include <stdint.h>

void *language_retain_n(void *, uint32_t);
void language_release_n(void *, uint32_t);
void *language_nonatomic_retain_n(void *, uint32_t);
void language_nonatomic_release_n(void *, uint32_t);

void *language_unownedRetain_n(void *, uint32_t);
void language_unownedRelease_n(void *, uint32_t);
void *language_nonatomic_unownedRetain_n(void *, uint32_t);
void language_nonatomic_unownedRelease_n(void *, uint32_t);

// Wrappers so we can call these from Codira without upsetting the ARC optimizer.
void *wrapper_language_retain_n(void *obj, uint32_t n) {
  return language_retain_n(obj, n);
}

void wrapper_language_release_n(void *obj, uint32_t n) {
  language_release_n(obj, n);
}

void *wrapper_language_nonatomic_retain_n(void *obj, uint32_t n) {
  return language_nonatomic_retain_n(obj, n);
}

void wrapper_language_nonatomic_release_n(void *obj, uint32_t n) {
  language_nonatomic_release_n(obj, n);
}

void *wrapper_language_unownedRetain_n(void *obj, uint32_t n) {
  return language_unownedRetain_n(obj, n);
}

void wrapper_language_unownedRelease_n(void *obj, uint32_t n) {
  language_unownedRelease_n(obj, n);
}

void *wrapper_language_nonatomic_unownedRetain_n(void *obj, uint32_t n) {
  return language_nonatomic_unownedRetain_n(obj, n);
}

void wrapper_language_nonatomic_unownedRelease_n(void *obj, uint32_t n) {
  language_nonatomic_unownedRelease_n(obj, n);
}


