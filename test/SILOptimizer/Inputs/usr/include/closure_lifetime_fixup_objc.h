
#define LANGUAGE_NOESCAPE __attribute__((__noescape__))

typedef void (^block_t)(void);

block_t block_create_noescape(block_t LANGUAGE_NOESCAPE block);
