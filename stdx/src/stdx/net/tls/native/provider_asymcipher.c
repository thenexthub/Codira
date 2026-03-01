/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "provider.h"
#include "opensslSymbols.h"
#include "api.h"
#include "securec.h"

/**
 * @brief Constant-time 8-bit zero check.
 *
 * @details Checks whether the least-significant 8 bits of the input are zero in a
 * branchless, constant-time manner and returns a full-width mask.
 *
 * @param x Input value; only the lowest 8 bits are considered.
 *
 * @return:
 *   0xFFFFFFFFu if (x & 0xFFu) == 0, otherwise 0u.
 */
static inline unsigned int CtIsZero8(unsigned int x)
{
    unsigned int v = x & 0xFFu;
    unsigned int orv = v | (0u - v);
    unsigned int msb = (orv >> 31) ^ 1u;
    return 0u - (msb & 1u);
}

/**
 * @brief Constant-time equality check for 8-bit values.
 *
 * @details Compares the least-significant 8 bits of the inputs without introducing
 * data-dependent branches or timing variability, making it suitable for
 * cryptographic use.
 *
 * @param a First value; only the low 8 bits are considered.
 * @param b Second value; only the low 8 bits are considered.
 *
 * @return:
 *  - An 8-bit mask as an unsigned int: 0xFFu if the low bytes are equal,
 *    0x00u otherwise.
 */
static inline unsigned int CtEq8(unsigned int a, unsigned int b)
{
    return CtIsZero8((a ^ b) & 0xFFu);
}

/**
 * @brief Constant-time zero test for size_t values.
 *
 * @details Produces a branch-free mask indicating whether the input is zero,
 * suitable for use in constant-time code paths to avoid timing side channels.
 *
 * @param x The value to test.
 *
 * @return:
 *   An unsigned int mask:
 *     - ~0u (all bits set) if x == 0
 *     - 0u otherwise
 */
static inline unsigned int CtIsZeroSizeT(size_t x)
{
    size_t orv = x | ((size_t)0 - x);
    size_t msb = (orv >> (sizeof(size_t) * 8 - 1)) ^ 1u;
    return 0u - (unsigned int)(msb & 1u);
}

/**
 * @brief Constant‑time equality check for size_t values.
 *
 * @details Compares two size_t values in constant time (no data‑dependent branches) by
 * XORing the inputs and testing the result for zero using a constant‑time
 * zero check. Suitable for cryptographic or side‑channel sensitive code.
 *
 * @param a First value to compare.
 * @param b Second value to compare.
 * @return unsigned int 1 if a equals b; 0 otherwise.
 *
 * @note The constant‑time guarantee relies on CtIsZeroSizeT executing in
 *       constant time for all inputs on the target platform.
 */
static inline unsigned int CtEqSizeT(size_t a, size_t b)
{
    return CtIsZeroSizeT(a ^ b);
}

/**
 * @brief Constant-time less-than comparison for size_t values.
 *
 * @details Compares two size_t operands without data-dependent branches or memory
 * accesses, suitable for cryptographic paths to mitigate timing side channels.
 *
 * @param a The left operand.
 * @param b The right operand.
 * @return 1 if a < b; otherwise 0.
 *
 * Notes:
 * - Treats size_t as an unsigned integer type.
 * - Intended for comparisons involving potentially secret data.
 * - Attempts to keep execution time independent of input values.
 */
static inline unsigned int CtLtSizeT(size_t a, size_t b)
{
    size_t diff = a - b;
    size_t msb = diff >> (sizeof(size_t) * 8 - 1);
    return 0u - (unsigned int)(msb & 1u);
}

/**
 * @brief Constant-time selection between two size_t values without data-dependent branches.
 *
 * @details This helper chooses between two candidates in a way that aims to avoid timing
 * side channels commonly introduced by conditional branches. It is intended for
 * use in cryptographic or other sensitive code paths.
 *
 * @param mask Selection mask.
 * @param a Value returned when mask is non-zero (true).
 * @param b Value returned when mask is zero (false).
 *
 * @return The selected value (a if mask != 0, otherwise b), computed using only bitwise
 *         operations to avoid data-dependent branching.
 *
 * @note
 * - Designed to execute in time independent of the values of a, b, and mask.
 * - Callers should pass a mask that is either 0 or non-zero; non-zero is treated
 *   as "true" and is internally expanded to a full-width mask.
 */
static inline size_t CtSelectSizeT(unsigned int mask, size_t a, size_t b)
{
    /* Expand to a full-width mask: 0 -> 0x00..00, nonzero -> 0xFF..FF */
    size_t m = (size_t)0 - (size_t)(mask & 1u);
    return (m & a) | (~m & b);
}

/**
 * @brief Remove PKCS#1 v1.5 padding from an encoded message (EM).
 *
 * @details Validates the PKCS#1 v1.5 structure
 *   0x00 || 0x02 || PS || 0x00 || M
 * without data-dependent branches and, on success, copies the last 'outLen'
 * bytes of M into 'out'. The padding string PS must be at least 8 bytes and
 * contain only non-zero bytes.
 *
 * @param out      Destination buffer for the recovered message bytes.
 * @param outLen  Number of bytes to recover into 'out'.
 * @param em       Source buffer containing the encoded message (EM).
 * @param emLen   Length of 'em' in bytes (typically the RSA modulus size).
 *
 * @return:
 *   1 on success; 0 on failure (invalid format or insufficient padding).
 *
 * @note:
 *   - Buffers 'out' and 'em' must not overlap.
 *   - This routine does not perform RSA; it only removes PKCS#1 v1.5 padding.
 *
 * @see RFC 8017 (PKCS #1 v2.2), §7.2 RSAES-PKCS1-v1_5.
 */
static int Pkcs1V15Unpad(unsigned char* out, size_t outLen, const unsigned char* em, size_t emLen)
{
    /* PKCS#1 v1.5 minimal overhead = 11 bytes:
       0x00 || 0x02 || PS(>=8 non-zero) || 0x00 */
    const size_t pkcs1V15MinOverhead = 11u;

    if (!out || !em || emLen < outLen + pkcs1V15MinOverhead) {
        return 0;
    }

    unsigned int validMask = CtIsZero8(em[0]);
    /*
    * PKCS#1 v1.5: EM = 0x00 || 0x02 || PS || 0x00 || M
    * 0x02 denotes "block type 2"
    * (RSAES-PKCS1-v1_5; PS is at least 8 non-zero random bytes; signatures use 0x01)
    */
    validMask &= CtEq8(em[1], 0x02);

    // Find the index of the first 0x00 separator after the PS.
    // Start at index 2 because [0]=0x00, [1]=0x02 by definition.
    unsigned int zeroFound = 0;         // Will be 0xFFFFFFFF if a zero was found
    unsigned int looking = 0xFFFFFFFFu; // While still looking for the first zero
    size_t sepIndex = 0;                // Will hold the index of the first 0x00

    for (size_t i = 2; i < emLen; ++i) {
        unsigned int isZero = CtIsZero8(em[i]);
        size_t chosen = CtSelectSizeT(looking & isZero, i, sepIndex);
        sepIndex = chosen;
        zeroFound |= looking & isZero;
        looking &= ~isZero;
    }

    size_t msgIndex = sepIndex + 1;
    size_t msgLen = emLen - msgIndex;

    unsigned int psTooShort = CtLtSizeT(sepIndex, 10); // 2 (0x00,0x02) + 8 (min PS)
    unsigned int msgLenBad = ~CtEqSizeT(msgLen, outLen);

    validMask &= zeroFound;
    validMask &= ~psTooShort;
    validMask &= ~msgLenBad;

    size_t fallbackIndex = emLen >= outLen ? emLen - outLen : 0;
    size_t readIndex = CtSelectSizeT(validMask, msgIndex, fallbackIndex);
    unsigned char validMaskByte = (unsigned char)(validMask & 0xFFu);
    unsigned char invalidMaskByte = (unsigned char)(~validMaskByte);

    for (size_t i = 0; i < outLen; ++i) {
        unsigned char fallbackByte = out[i];
        unsigned char selected = em[readIndex + i];
        out[i] = (unsigned char)((validMaskByte & selected) | (invalidMaskByte & fallbackByte));
    }

    return (int)(validMask & 1u);
}

/**
 * @brief Creates a new Keyless asymmetric-cipher context.
 * @param provctx Opaque provider context (unused).
 * @return Pointer to the newly allocated KeylessAsymcipherCtx, or NULL on failure.
 */
static void* KeylessAcNewctx(void* provctx)
{
    (void)provctx;
    KeylessProviderLog("[keyless] asym new ctx\n");
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    KeylessAsymcipherCtx* ctx = DYN_OPENSSL_zalloc(sizeof(KeylessAsymcipherCtx), dynMsg);
    KeylessCheckDynMsg(dynMsg, "asym newctx");
    if (ctx) {
        /* Default to PKCS#1 v1.5 for TLS 1.2 RSA key exchange */
        ctx->padMode = RSA_PKCS1_PADDING;
        KeylessCopyDynMsg(&ctx->dynMsg, dynMsg);
    }
    return ctx;
}

/**
 * @brief Frees a Keyless asymmetric-cipher context.
 * @param v Pointer to the KeylessAsymcipherCtx to free.
 * @note Also releases any associated key data.
 */
static void KeylessAcFreectx(void* v)
{
    KeylessAsymcipherCtx* c = v;
    if (!c) {
        return;
    }
    if (c->keyData) {
        KeylessKeyFreeExtern(c->keyData);
    }
    DYN_OPENSSL_secure_free(c, &c->dynMsg);
    KeylessCheckDynMsg(&c->dynMsg, "[keyless] asym freectx");
}

/**
 * @brief Duplicates a Keyless asymmetric-cipher context.
 * @param v Pointer to the KeylessAsymcipherCtx to duplicate.
 * @return Pointer to the newly allocated duplicate context, or NULL on failure.
 * @note Increments the reference count of any associated key data.
 * @note used by OpenSSL when duplicating operation contexts.
 */
static void* KeylessAcDupctx(void* v)
{
    KeylessAsymcipherCtx* c = v;
    if (!c) {
        return NULL;
    }

    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    KeylessAsymcipherCtx* n = DYN_OPENSSL_memdup(c, sizeof(*c), dynMsg);
    KeylessCheckDynMsg(dynMsg, "[keyless] asym dupctx");
    if (!n) {
        return NULL;
    }

    if (n->keyData) {
        KeylessKeyUpRef(n->keyData);
    }
    return n;
}

/**
 * @brief Initializes the Keyless asymmetric-cipher context for decryption.
 * @param vctx    Opaque operation context to initialize.
 * @param keydata Pointer to the key data to use for decryption.
 * @param params  Array of OSSL_PARAM parameters (unused).
 * @return 1 on success; 0 on failure (e.g., invalid parameters or unsupported key type).
 */
static int KeylessAcDecryptInit(void* vctx, void* keydata, const OSSL_PARAM params[])
{
    KeylessAsymcipherCtx* c = vctx;
    KeylessProviderLog("[keyless] asym decrypt init\n");
    if (!c || !keydata) {
        return 0;
    }

    /* libssl commonly passes pad-mode via params here; honor it */
    if (params) {
        DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
        const OSSL_PARAM* p = DYN_OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_PAD_MODE, dynMsg);
        if (p) {
            if (p->data_type == OSSL_PARAM_INTEGER) {
                int pad = 0;
                if (DYN_OSSL_PARAM_get_int(p, &pad, dynMsg)) {
                    if (pad == RSA_PKCS1_PADDING || pad == RSA_PKCS1_WITH_TLS_PADDING) {
                        c->padMode = RSA_PKCS1_PADDING;
                    } else {
                        c->padMode = pad;
                    }
                    KeylessProviderLog("[keyless] asym init pad_mode=%d\n", pad);
                }
            } else if (p->data_type == OSSL_PARAM_UTF8_STRING && p->data) {
                const char* s = (const char*)p->data;
                if (strcasecmp(s, "pkcs1") == 0 || strcasecmp(s, "pkcs1v15") == 0 || strcasecmp(s, "rsaes-pkcs1-v1_5") == 0 || strcasecmp(s, "tls") == 0) {
                    c->padMode = RSA_PKCS1_PADDING;
                } else {
                    return 0; /* unsupported padding */
                }
                KeylessProviderLog("[keyless] asym init pad_mode(str)=%s -> %d\n", s, c->padMode);
            }
        }
        KeylessCheckDynMsg(dynMsg, "KeylessAcDecryptInit params");
    }

    KeylessKeyUpRef(keydata);
    c->keyData = keydata;
    c->type = KeylessKeyGetType(keydata);
    /* Snapshot dyn loader state from key for consistent symbol resolution */
    KeylessKey* key = (KeylessKey*)keydata;
    if (key) {
        KeylessCopyDynMsg(&c->dynMsg, &key->dynMsg);
    } else {
        KeylessCopyDynMsg(&c->dynMsg, KeylessProviderGetThreadDynMsg());
    }
    KeylessProviderLog("[keyless] asym decrypt init type=%d\n", c->type);
    if (c->type != KEYLESS_KEY_TYPE_RSA) {
        return 0;
    }
    /* Only PKCS#1 v1.5 padding supported for TLS 1.2 kRSA */
    if (c->padMode != RSA_PKCS1_PADDING) {
        return 0;
    }
    return 1;
}

/**
 * @brief Decrypts a ciphertext buffer using the keyless asymmetric-cipher provider context.
 *
 * @details This function follows the provider asym-cipher decrypt contract:
 * - If out is NULL, the function sets *outlen to the required plaintext size and returns success.
 * - If out is not NULL, outsize must be large enough; the function writes the plaintext to out
 *   and sets *outlen to the number of bytes written.
 *
 * Parameters:
 * @param vctx    Opaque operation context initialized for the intended key/algorithm and mode.
 * @param out     Destination buffer for the plaintext, or NULL to query the required size.
 * @param outlen  Output: receives the required or actual plaintext length in bytes.
 * @param outsize Size of the out buffer in bytes (ignored when out is NULL).
 * @param in      Source buffer containing the ciphertext to decrypt.
 * @param inlen   Length of the ciphertext in bytes.
 *
 * @returns
 * - 1 on success (whether sizing query or actual decryption),
 * - 0 on failure (e.g., invalid parameters, buffer too small, unsupported operation, or backend error).
 *
 * @note
 * - The vctx is not thread-safe for concurrent use across threads.
 * - On error, no plaintext is written and *outlen is left unmodified unless performing a size query.
 */
static int KeylessAcDecrypt(void* vctx, unsigned char* out, size_t* outlen, size_t outsize, const unsigned char* in, size_t inlen)
{
    KeylessAsymcipherCtx* c = vctx;
    unsigned char* remote = NULL; // Early declare for cleanup
    if (!c || !c->keyData) {
        return 0;
    }
    size_t modlen = KeylessKeyGetRsaNLen(c->keyData);
    if (!modlen) {
        modlen = KeylessKeyGetSize(c->keyData);
    }
    if (modlen == 0) {
        return 0;
    }

    const size_t TLS_RSA_PMS_LENGTH = 48; // TLS RSA PMS length fixed 48 bytes
    if (!out) {
        if (outlen) {
            *outlen = TLS_RSA_PMS_LENGTH;
        }
        KeylessProviderLog("[keyless] asym size query -> %zu\n", TLS_RSA_PMS_LENGTH);
        return 1;
    }

    if (outsize < TLS_RSA_PMS_LENGTH) {
        return 0;
    }

    DynMsg* dynMsg = &c->dynMsg;
    const char* keyId = ((KeylessKey*)(c->keyData))->keyId;
    KeylessRemoteDecryptCb dcb = KeylessLookupDecryptCb(keyId);
    if (!dcb) {
        int lib = KeylessErrorLibInit();
        KeylessProviderLog("[keyless] %d: decrypt callback not set.\n", lib);
        return 0;
    }

    if (DYN_RAND_bytes(out, TLS_RSA_PMS_LENGTH, dynMsg) <= 0) {
        int lib = KeylessErrorLibInit();
        KeylessProviderLog("[keyless] %d: RAND_bytes failed.\n", lib);
        return 0;
    }

    unsigned char* cipher = NULL;
    unsigned char* em = NULL;
    int ok = 0;

    cipher = DYN_OPENSSL_secure_malloc(modlen, dynMsg);
    em = DYN_OPENSSL_secure_malloc(modlen, dynMsg);
    if (!cipher || !em) {
        goto cleanup;
    }

    memset_s(cipher, modlen, 0, modlen);
    memset_s(em, modlen, 0, modlen);

    size_t copyLen = inlen;
    const unsigned char* src = in;
    if (copyLen > modlen) {
        src = in + (inlen - modlen);
        copyLen = modlen;
    }
    if (memcpy_s(cipher + (modlen - copyLen), copyLen, src, copyLen) != EOK) {
        goto cleanup;
    }

    KeylessProviderLog("[keyless] asym decrypt inlen=%zu paddedLen=%zu\n", inlen, modlen);

    int64_t written = 0;
    remote = (unsigned char*)dcb(keyId, cipher, (int64_t)modlen, &written);
    int success = 0;
    if (remote && written > 0) {
        size_t produced = (size_t)written;
        if (produced == TLS_RSA_PMS_LENGTH) {
            /* Support backends that return the already-unpadded PMS (48 bytes). */
            if (memcpy_s(out, TLS_RSA_PMS_LENGTH, remote, TLS_RSA_PMS_LENGTH) != EOK) {
                goto cleanup;
            }
            success = 1;
            KeylessProviderLog("[keyless] asym decrypt PMS direct len=48\n");
        } else if (produced <= modlen) {
            /* Backend returned raw RSA decrypt output (EM) of up to modulus length. */
            if (memcpy_s(em + (modlen - produced), produced, remote, produced) != EOK) {
                goto cleanup;
            }
            success = Pkcs1V15Unpad(out, TLS_RSA_PMS_LENGTH, em, modlen);
            KeylessProviderLog("[keyless] asym decrypt EM len=%zu success=%d\n", produced, success);
        } else {
            KeylessProviderLog("[keyless] asym decrypt invalid size=%zu (> modlen=%zu)\n", produced, modlen);
        }
    } else {
        KeylessProviderLog("[keyless] asym decrypt remote failure (written=%ld)\n", (long)written);
        (void)Pkcs1V15Unpad(out, TLS_RSA_PMS_LENGTH, em, modlen);
    }

    if (outlen) {
        *outlen = TLS_RSA_PMS_LENGTH;
    }

    ok = 1;

cleanup:
    if (cipher) {
        DYN_OPENSSL_cleanse(cipher, modlen, dynMsg);
        DYN_OPENSSL_secure_free(cipher, dynMsg);
    }

    if (em) {
        DYN_OPENSSL_cleanse(em, modlen, dynMsg);
        DYN_OPENSSL_secure_free(em, dynMsg);
    }

    if (remote != NULL && written > 0) {
        (void)memset_s(remote, (size_t)written, 0, (size_t)written);
    }
    free(remote);

    KeylessCheckDynMsg(dynMsg, "KeylessAcDecrypt");
    return ok;
}

static const OSSL_PARAM* KeylessAcGettableCtxParams(void* provctx)
{
    (void)provctx;
    static const OSSL_PARAM params[] = {OSSL_PARAM_END};
    return params;
}

static int KeylessAcGetCtxParams(void* vctx, OSSL_PARAM params[])
{
    (void)vctx;
    (void)params;
    return 1;
}

static int KeylessAcSetCtxParams(void* vctx, const OSSL_PARAM params[])
{
    KeylessAsymcipherCtx* c = vctx;
    if (!c || !params) {
        return 1;
    }

    KeylessProviderLog("[keyless] asym set_ctx params\n");
    const OSSL_PARAM* p = NULL;
    DynMsg* dynMsg = &c->dynMsg;

    /* Handle pad-mode: accept PKCS#1 v1.5 and TLS special pad (treated as v1.5) */
    p = DYN_OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_PAD_MODE, dynMsg);
    if (!p) {
        return 0;
    }

    if (p->data_type == OSSL_PARAM_INTEGER) {
        int pad = 0;
        if (DYN_OSSL_PARAM_get_int(p, &pad, dynMsg)) {
            if (pad == RSA_PKCS1_PADDING || pad == 7 /* RSA_PKCS1_WITH_TLS_PADDING */) {
                c->padMode = RSA_PKCS1_PADDING;
            } else {
                c->padMode = pad;
            }
            KeylessProviderLog("[keyless] asym set_ctx pad_mode=%d\n", pad);
            if (c->padMode != RSA_PKCS1_PADDING) {
                return 0; /* not supported */
            }
        }
    } else if (p->data_type == OSSL_PARAM_UTF8_STRING && p->data) {
        const char* s = (const char*)p->data;
        if (strcasecmp(s, "pkcs1") == 0 || strcasecmp(s, "pkcs1v15") == 0 || strcasecmp(s, "rsaes-pkcs1-v1_5") == 0 || strcasecmp(s, "tls") == 0) {
            c->padMode = RSA_PKCS1_PADDING;
        } else {
            return 0; /* unsupported padding */
        }
        KeylessProviderLog("[keyless] asym set_ctx pad_mode(str)=%s -> %d\n", s, c->padMode);
    }

    KeylessCheckDynMsg(dynMsg, "KeylessAcSetCtxParams");
    return 1;
}

static const OSSL_PARAM* KeylessAcSettableCtxParams(void* provctx)
{
    (void)provctx;
    static const OSSL_PARAM settable[] = {OSSL_PARAM_int(OSSL_ASYM_CIPHER_PARAM_PAD_MODE, NULL), OSSL_PARAM_END};
    return settable;
}

static int KeylessAcEncryptInit(void* vctx, void* keydata, const OSSL_PARAM params[])
{
    KeylessAsymcipherCtx* c = vctx;
    KeylessProviderLog("[keyless] asym encrypt init\n");
    if (!c || !keydata) {
        return 0;
    }

    /* Honor pad-mode param if provided */
    if (params) {
        DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
        const OSSL_PARAM* p = DYN_OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_PAD_MODE, dynMsg);
        if (p) {
            if (p->data_type == OSSL_PARAM_INTEGER) {
                int pad = 0;
                if (DYN_OSSL_PARAM_get_int(p, &pad, dynMsg)) {
                    c->padMode = pad;
                }
            } else if (p->data_type == OSSL_PARAM_UTF8_STRING && p->data) {
                const char* s = (const char*)p->data;
                if (strcasecmp(s, "pkcs1") == 0 || strcasecmp(s, "pkcs1v15") == 0 || strcasecmp(s, "rsaes-pkcs1-v1_5") == 0) {
                    c->padMode = RSA_PKCS1_PADDING;
                } else {
                    return 0; /* unsupported padding */
                }
            }
        }
        KeylessCheckDynMsg(dynMsg, "KeylessAcEncryptInit params");
    }

    KeylessKeyUpRef(keydata);
    c->keyData = keydata;
    c->type = KeylessKeyGetType(keydata);
    KeylessKey* key = (KeylessKey*)keydata;
    if (key) {
        KeylessCopyDynMsg(&c->dynMsg, &key->dynMsg);
    } else {
        KeylessCopyDynMsg(&c->dynMsg, KeylessProviderGetThreadDynMsg());
    }
    /* Only RSA with PKCS#1 v1.5 supported. */
    if (c->type != KEYLESS_KEY_TYPE_RSA) {
        return 0;
    }
    if (c->padMode != RSA_PKCS1_PADDING) {
        return 0;
    }
    return 1;
}

static int KeylessAcEncrypt(void* vctx, unsigned char* out, size_t* outlen, size_t outsize, const unsigned char* in, size_t inlen)
{
    KeylessAsymcipherCtx* c = vctx;
    if (!c || !c->keyData) {
        return 0;
    }
    if (c->type != KEYLESS_KEY_TYPE_RSA || c->padMode != RSA_PKCS1_PADDING) {
        return 0;
    }

    size_t modlen = KeylessKeyGetRsaNLen(c->keyData);
    if (!modlen) {
        modlen = KeylessKeyGetSize(c->keyData);
    }
    if (!modlen) {
        return 0;
    }

    if (!out) {
        if (outlen) {
            *outlen = modlen;
        }
        KeylessProviderLog("[keyless] asym encrypt size query -> %zu\n", modlen);
        return 1;
    }

    if (outsize < modlen) {
        return 0;
    }
    /* TLS 1.2 kRSA PMS length is 48 bytes */
    if (inlen == 0 || inlen > modlen - 11) { /* PKCS#1 v1.5 min padding 11 bytes */
        return 0;
    }

    DynMsg* dynMsg = &c->dynMsg;

    /* Build EM = 0x00 || 0x02 || PS || 0x00 || M */
    unsigned char* em = DYN_OPENSSL_secure_malloc(modlen, dynMsg);
    if (!em) {
        return 0;
    }
    memset_s(em, modlen, 0, modlen);
    em[0] = 0x00;
    em[1] = 0x02;
    size_t psLen = modlen - inlen - 3; /* 2 prefix + 1 separator */
    if (psLen < 8) {
        DYN_OPENSSL_secure_free(em, dynMsg);
        return 0;
    }
    /* Fill PS with non-zero random bytes */
    size_t filled = 0;
    while (filled < psLen) {
        unsigned char b = 0;
        if (DYN_RAND_bytes(&b, 1, dynMsg) <= 0) {
            DYN_OPENSSL_secure_free(em, dynMsg);
            return 0;
        }
        if (b == 0) {
            continue;
        }
        em[2 + filled] = b;
        filled++;
    }
    em[2 + psLen] = 0x00;
    if (memcpy_s(em + 3 + psLen, modlen - (3 + psLen), in, inlen) != EOK) {
        DYN_OPENSSL_secure_free(em, dynMsg);
        return 0;
    }

    /* RSA c = em^e mod n */
    const unsigned char* nBytes = KeylessKeyGetN(c->keyData);
    size_t nLen = KeylessKeyGetNLen(c->keyData);
    if (!nBytes || nLen != modlen) {
        /* Allow leading zeros: compute length from modlen anyway */
    }

    BIGNUM* bnN = DYN_BN_bin2bn(nBytes, (int)nLen, NULL, dynMsg);
    KeylessKey* kk = (KeylessKey*)c->keyData;
    BIGNUM* bnE = DYN_BN_bin2bn(kk->e, (int)kk->eLen, NULL, dynMsg);
    BIGNUM* bnM = DYN_BN_bin2bn(em, (int)modlen, NULL, dynMsg);
    BIGNUM* bnC = DYN_BN_new(dynMsg);
    BN_CTX* bnCtx = DYN_BN_CTX_new(dynMsg);
    int ok = (bnN && bnE && bnM && bnC && bnCtx) ? 1 : 0;
    if (ok) {
        ok = DYN_BN_mod_exp(bnC, bnM, bnE, bnN, bnCtx, dynMsg);
    }
    int ret = 0;
    if (ok) {
        if (DYN_BN_bn2binpad(bnC, out, (int)modlen, dynMsg) == (int)modlen) {
            if (outlen) {
                *outlen = modlen;
            }
            ret = 1;
        }
    }
    if (bnCtx) {
        DYN_BN_CTX_free(bnCtx, dynMsg);
    }
    if (bnC) {
        DYN_BN_free(bnC, dynMsg);
    }
    if (bnM) {
        DYN_BN_free(bnM, dynMsg);
    }
    if (bnE) {
        DYN_BN_free(bnE, dynMsg);
    }
    if (bnN) {
        DYN_BN_free(bnN, dynMsg);
    }
    DYN_OPENSSL_secure_free(em, dynMsg);
    KeylessCheckDynMsg(dynMsg, "KeylessAcEncrypt");
    return ret;
}
const OSSL_DISPATCH keylessAsymcipherFunctions[] = {{OSSL_FUNC_ASYM_CIPHER_NEWCTX, (void (*)(void))KeylessAcNewctx},
                                                    {OSSL_FUNC_ASYM_CIPHER_FREECTX, (void (*)(void))KeylessAcFreectx},
                                                    {OSSL_FUNC_ASYM_CIPHER_DUPCTX, (void (*)(void))KeylessAcDupctx},
                                                    {OSSL_FUNC_ASYM_CIPHER_ENCRYPT_INIT, (void (*)(void))KeylessAcEncryptInit},
                                                    {OSSL_FUNC_ASYM_CIPHER_ENCRYPT, (void (*)(void))KeylessAcEncrypt},
                                                    {OSSL_FUNC_ASYM_CIPHER_DECRYPT_INIT, (void (*)(void))KeylessAcDecryptInit},
                                                    {OSSL_FUNC_ASYM_CIPHER_DECRYPT, (void (*)(void))KeylessAcDecrypt},
                                                    {OSSL_FUNC_ASYM_CIPHER_GETTABLE_CTX_PARAMS, (void (*)(void))KeylessAcGettableCtxParams},
                                                    {OSSL_FUNC_ASYM_CIPHER_GET_CTX_PARAMS, (void (*)(void))KeylessAcGetCtxParams},
                                                    {OSSL_FUNC_ASYM_CIPHER_SET_CTX_PARAMS, (void (*)(void))KeylessAcSetCtxParams},
                                                    {OSSL_FUNC_ASYM_CIPHER_SETTABLE_CTX_PARAMS, (void (*)(void))KeylessAcSettableCtxParams},
                                                    {0, NULL}};

/* Algorithm export (referenced in provider) */
const OSSL_ALGORITHM keylessAsymcipherAlgorithms[] = {{"RSA:rsaEncryption:1.2.840.113549.1.1.1", "provider=keyless", keylessAsymcipherFunctions, "Keyless RSA Decrypt (remote)"},
                                                      {NULL, NULL, NULL, NULL}};
