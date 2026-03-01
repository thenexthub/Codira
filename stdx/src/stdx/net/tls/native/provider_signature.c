/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/bn.h>
#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include "provider.h"
#include "opensslSymbols.h"
#include "securec.h"

static _Atomic int LIB_NUM = 0; /* Library number for error reporting */

int KeylessErrorLibInit(void)
{
    int v = atomic_load(&LIB_NUM);
    if (v != 0) {
        return v;
    }
    const DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    int assigned = DYN_ERR_get_next_error_library((DynMsg*)dynMsg);
    KeylessCheckDynMsg(dynMsg, "KeylessErrorLibInit");
    if (assigned <= 0) {
        return 0;
    }
    int expected = 0;
    if (atomic_compare_exchange_strong(&LIB_NUM, &expected, assigned)) {
        return assigned;
    }
    /* Lost race, return existing */
    return atomic_load(&LIB_NUM);
}

enum
{
    ALG_NAME_MAX_LEN = 64,
    DIGEST_NAME_MAX_LEN = 16,
    MGF1_NAME_MAX_LEN = 16
};
/* Signature context */
typedef struct KeylessSignCtx
{
    void* keyData;                  /* provider key object */
    int type;                       /* 1=rsa 2=ec */
    char algName[ALG_NAME_MAX_LEN]; /* canonical algorithm string passed to remote */
    char digestName[DIGEST_NAME_MAX_LEN];
    size_t mdLen; /* expected digest length */
    int isRsa;    /* dispatch kind check */
    int isEc;
    int padMode; /* for RSA: expect PSS padding */
    char mgf1Name[MGF1_NAME_MAX_LEN];
    int pssSaltlen; /* if not set(pssSaltlen: -1), use mdLen */
    DynMsg dynMsg;
} KeylessSignCtx;

static inline DynMsg* KeylessSigDynMsg(KeylessSignCtx* c)
{
    return c ? &c->dynMsg : NULL;
}

#define KEYLESS_CHECK_DYNMSG_RETURN(dynMsg, ctx, retval)                                                                                                                           \
    do {                                                                                                                                                                           \
        if (!KeylessCheckDynMsg((dynMsg), (ctx))) {                                                                                                                                \
            return (retval);                                                                                                                                                       \
        }                                                                                                                                                                          \
    } while (0)

// forward declarations
static int KeylessSigSetCtxParams(void* vctx, const OSSL_PARAM params[]);
static size_t DigestLenFor(const char* name);
static size_t RsaModulusBits(const unsigned char* n, size_t nLen);

/* Normalize digest name (in-place). */
static void NormalizeMd(char* name, size_t capacity)
{
    if (!name) {
        return;
    }
    for (size_t i = 0; name[i] != '\0' && i + 1 < capacity; ++i) {
        name[i] = (char)tolower((unsigned char)name[i]);
    }
    if (strcmp(name, "sha2-256") == 0) {
        strncpy_s(name, capacity, "sha256", capacity);
    } else if (strcmp(name, "sha2-384") == 0) {
        strncpy_s(name, capacity, "sha384", capacity);
    } else if (strcmp(name, "sha2-512") == 0) {
        strncpy_s(name, capacity, "sha512", capacity);
    }
    name[capacity - 1] = '\0';
}

/**
 * @brief Generate a byte mask using MGF1 (PKCS #1 v2.2) with the specified digest.
 *
 * Expands the given seed deterministically into maskLen bytes using a
 * counter-based construction and writes the result to the provided output buffer.
 *
 * @param mask      Caller-allocated output buffer that will receive the mask.
 * @param maskLen  Number of bytes to generate and write to 'mask'.
 * @param seed      Input seed bytes used as the basis for mask generation.
 * @param seedLen  Length of 'seed' in bytes.
 * @param md        Digest algorithm (e.g., EVP_sha256()); must not be NULL.
 *
 * @return 1 on success; 0 on failure (e.g., invalid arguments or digest errors).
 *
 * @note Output is deterministic for the same (seed, maskLen, md).
 * @note On failure, the contents of 'mask' are indeterminate.
 */
static int Mgf1(unsigned char* mask, size_t maskLen, const unsigned char* seed, size_t seedLen, const EVP_MD* md, DynMsg* dynMsg)
{
    if (!mask || !md) {
        return 0;
    }

    int mdSize = DYN_EVP_MD_get_size(md, dynMsg);
    size_t mdLen = mdSize > 0 ? (size_t)mdSize : 0;

    if (mdLen == 0) {
        KEYLESS_CHECK_DYNMSG_RETURN(dynMsg, "EVP_MD_get_size", 0);
        return 0;
    }
    EVP_MD_CTX* ctx = DYN_EVP_MD_CTX_new(dynMsg);
    if (!ctx) {
        KEYLESS_CHECK_DYNMSG_RETURN(dynMsg, "EVP_MD_CTX_new", 0);
        return 0;
    }
    unsigned char counter[4]; // 4-byte counter (big-endian). PKCS #1 v2.2 MGF1 uses a 32-bit counter encoded as I2OSP(i, 4).
    unsigned char digest[EVP_MAX_MD_SIZE];
    size_t offset = 0;
    for (unsigned int i = 0; offset < maskLen; ++i) {
        counter[0] = (unsigned char)((i >> 24) & 0xff); // 24: high byte
        counter[1] = (unsigned char)((i >> 16) & 0xff); // 16: mid-high byte
        counter[2] = (unsigned char)((i >> 8) & 0xff);  // 8: mid-low byte
        counter[3] = (unsigned char)(i & 0xff);         // 8: low byte
        if (!DYN_EVP_DigestInit_ex(ctx, md, NULL, dynMsg) || !DYN_EVP_DigestUpdate(ctx, (const char*)seed, seedLen, dynMsg) ||
            !DYN_EVP_DigestUpdate(ctx, (const char*)counter, sizeof(counter), dynMsg) || !DYN_EVP_DigestFinal_ex(ctx, digest, NULL, dynMsg)) {
            DYN_EVP_MD_CTX_free(ctx, dynMsg);
            DYN_OPENSSL_cleanse(digest, sizeof(digest), dynMsg);
            KEYLESS_CHECK_DYNMSG_RETURN(dynMsg, "EVP_DigestInit_ex", 0);
            return 0;
        }
        size_t toCopy = maskLen - offset;
        if (toCopy > mdLen) {
            toCopy = mdLen;
        }
        (void)memcpy_s(mask + offset, toCopy, digest, toCopy);
        offset += toCopy;
    }
    DYN_EVP_MD_CTX_free(ctx, dynMsg);
    DYN_OPENSSL_cleanse(digest, sizeof(digest), dynMsg);
    KeylessCheckDynMsg(dynMsg, "Mgf1");
    return 1;
}

/**
 * @brief Encode a precomputed message hash using RSA-PSS (EMSA-PSS, RFC 8017).
 *
 * Constructs EM = maskedDB || H || 0xBC where:
 * - H = Hash(0x00..00 (8 bytes) || mhash || salt)
 * - maskedDB = (PS || 0x01 || salt) XOR MGF1(H, emLen - hashLen - 1)
 * The leftmost (8*emLen - em_bits) bits of maskedDB are cleared.
 *
 * @param em        Output buffer for the encoded message (EM). Must be emLen bytes.
 * @param emLen    Size in bytes of em. Requires emLen >= hashLen + saltLen + 2 and em_bits <= 8*emLen - 1.
 * @param em_bits   Maximum number of significant bits in EM (typically modulus_bits - 1).
 * @param mhash     Precomputed message digest (must match @p hashMd).
 * @param hashLen  Length of @p mhash in bytes; should equal EVP_MD_size(@p hashMd).
 * @param hashMd   Message digest for H and mhash (e.g., EVP_sha256()).
 * @param mgf1Md   Digest used by MGF1 (commonly same as @p hashMd).
 * @param saltLen  Salt length in bytes. If 0, use zero-length salt. Must satisfy emLen >= hashLen + saltLen + 2.
 *
 * @return 1 on success, 0 on failure.
 *
 * @note May allocate a salt and draw randomness when @p saltLen > 0.
 */
static int KeylessPssPrepareBuffers(size_t dbLen, size_t saltLen, unsigned char** salt, unsigned char** db, unsigned char** dbmask, DynMsg* dynMsg)
{
    *salt = saltLen ? DYN_OPENSSL_secure_malloc(saltLen, dynMsg) : NULL;
    *db = dbLen ? DYN_OPENSSL_secure_malloc(dbLen, dynMsg) : NULL;
    *dbmask = dbLen ? DYN_OPENSSL_secure_malloc(dbLen, dynMsg) : NULL;
    if ((saltLen && !*salt) || (dbLen && (!*db || !*dbmask))) {
        KeylessCheckDynMsg(dynMsg, "CRYPTO_secure_malloc");
        return 0;
    }
    return 1;
}

static int KeylessPssFillSalt(unsigned char* salt, size_t saltLen, DynMsg* dynMsg)
{
    if (saltLen == 0) {
        return 1;
    }
    int rc = DYN_RAND_bytes(salt, saltLen, dynMsg);
    if (rc <= 0) {
        KeylessCheckDynMsg(dynMsg, "RAND_bytes");
        return 0;
    }
    return 1;
}

static int KeylessPssComputeDigest(const EVP_MD* hashMd, const unsigned char* mhash, size_t hashLen, const unsigned char* salt, size_t saltLen,
                                   unsigned char digest[EVP_MAX_MD_SIZE], DynMsg* dynMsg)
{
    size_t prefixZeroLen = 8;
    size_t mprimeLen = prefixZeroLen + hashLen + saltLen;
    unsigned char* mprime = DYN_OPENSSL_secure_malloc(mprimeLen, dynMsg);
    if (!mprime) {
        KeylessCheckDynMsg(dynMsg, "CRYPTO_secure_malloc");
        return 0;
    }
    (void)memset_s(mprime, prefixZeroLen, 0x00, prefixZeroLen);
    int ret = memcpy_s(mprime + prefixZeroLen, hashLen + saltLen, mhash, hashLen);
    if (ret != 0) {
        DYN_OPENSSL_secure_free(mprime, dynMsg);
        return 0;
    }

    if (saltLen > 0) {
        (void)memcpy_s(mprime + prefixZeroLen + hashLen, saltLen, salt, saltLen);
    }

    EVP_MD_CTX* hash_ctx = DYN_EVP_MD_CTX_new(dynMsg);
    if (!hash_ctx) {
        DYN_OPENSSL_cleanse(mprime, mprimeLen, dynMsg);
        DYN_OPENSSL_secure_free(mprime, dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_MD_CTX_new");
        return 0;
    }
    int ok = DYN_EVP_DigestInit_ex(hash_ctx, hashMd, NULL, dynMsg) && DYN_EVP_DigestUpdate(hash_ctx, (const char*)mprime, mprimeLen, dynMsg) &&
             DYN_EVP_DigestFinal_ex(hash_ctx, digest, NULL, dynMsg);
    DYN_EVP_MD_CTX_free(hash_ctx, dynMsg);
    DYN_OPENSSL_cleanse(mprime, mprimeLen, dynMsg);
    DYN_OPENSSL_secure_free(mprime, dynMsg);
    if (!ok) {
        KeylessCheckDynMsg(dynMsg, "EVP_DigestInit_ex");
    }
    return ok;
}

static int KeylessPssPopulateDb(unsigned char* db, size_t dbLen, const unsigned char* salt, size_t saltLen)
{
    if (dbLen < saltLen + 1) {
        return 0;
    }
    size_t psLen = dbLen - saltLen - 1;
    (void)memset_s(db, psLen, 0x00, psLen);
    db[psLen] = 0x01;
    if (saltLen > 0) {
        (void)memcpy_s(db + psLen + 1, saltLen, salt, saltLen);
    }
    return 1;
}

static int KeylessPssMaskDb(unsigned char* db, unsigned char* dbmask, size_t dbLen, size_t emLen, size_t em_bits, const unsigned char* digest, size_t hashLen, const EVP_MD* mgf1Md,
                            DynMsg* dynMsg)
{
    if (!Mgf1(dbmask, dbLen, digest, hashLen, mgf1Md, dynMsg)) {
        return 0;
    }
    for (size_t i = 0; i < dbLen; ++i) {
        db[i] ^= dbmask[i];
    }

    size_t unused_bits = emLen * 8 - em_bits;
    if (unused_bits == 0 || unused_bits > 8) {
        return 0;
    }
    db[0] &= (unsigned char)(0xFF >> unused_bits);
    return 1;
}

static void KeylessPssCleanupBuffers(unsigned char* salt, size_t saltLen, unsigned char* db, unsigned char* dbmask, size_t dbLen, DynMsg* dynMsg)
{
    if (salt) {
        DYN_OPENSSL_cleanse(salt, saltLen, dynMsg);
        DYN_OPENSSL_secure_free(salt, dynMsg);
    }
    if (db) {
        DYN_OPENSSL_cleanse(db, dbLen, dynMsg);
        DYN_OPENSSL_secure_free(db, dynMsg);
    }
    if (dbmask) {
        DYN_OPENSSL_cleanse(dbmask, dbLen, dynMsg);
        DYN_OPENSSL_secure_free(dbmask, dynMsg);
    }
}

/**
 * @brief Encode a precomputed message hash using RSA-PSS (EMSA-PSS, RFC 8017).
 *
 * The function constructs the encoded message EM = maskedDB || H || 0xBC where:
 *  - H = Hash(0x00..00 (8 bytes) || mhash || salt)
 *  - maskedDB = (PS || 0x01 || salt) XOR MGF1(H, emLen - hashLen - 1)
 *  - The leftmost (8*emLen - em_bits) bits of maskedDB are cleared.
 */
static int EncodePss(unsigned char* em, size_t emLen, size_t em_bits, const unsigned char* mhash, size_t hashLen, const EVP_MD* hashMd, const EVP_MD* mgf1Md, int saltLen,
                     DynMsg* dynMsg)
{
    if (!em || !hashMd || !mgf1Md || !mhash) {
        return 0;
    }
    if (saltLen == RSA_PSS_SALTLEN_DIGEST || saltLen < 0) {
        saltLen = (int)hashLen;
    }
    size_t saltSize = (size_t)saltLen;
    if (emLen < hashLen + saltSize + 2) {
        return 0;
    }

    size_t dbLen = emLen - hashLen - 1;
    if (dbLen < saltSize + 1) {
        return 0;
    }

    unsigned char* salt = NULL;
    unsigned char* db = NULL;
    unsigned char* dbmask = NULL;
    unsigned char digest[EVP_MAX_MD_SIZE];
    int ok = 0;

    if (!KeylessPssPrepareBuffers(dbLen, saltSize, &salt, &db, &dbmask, dynMsg)) {
        goto done;
    }
    if (!KeylessPssFillSalt(salt, saltSize, dynMsg)) {
        goto done;
    }
    if (!KeylessPssComputeDigest(hashMd, mhash, hashLen, salt, saltSize, digest, dynMsg)) {
        goto done;
    }
    if (!KeylessPssPopulateDb(db, dbLen, salt, saltSize)) {
        goto done;
    }
    if (!KeylessPssMaskDb(db, dbmask, dbLen, emLen, em_bits, digest, hashLen, mgf1Md, dynMsg)) {
        goto done;
    }

    (void)memcpy_s(em, dbLen, db, dbLen);
    (void)memcpy_s(em + dbLen, hashLen, digest, hashLen);
    em[emLen - 1] = 0xBC;
    ok = 1;

done:
    KeylessPssCleanupBuffers(salt, saltSize, db, dbmask, dbLen, dynMsg);
    DYN_OPENSSL_cleanse(digest, sizeof(digest), dynMsg);
    return ok;
}

/**
 * Encodes an EMSA-PKCS1-v1_5 block for RSA signatures into the provided buffer.
 *
 * The output(Encoded Message) format is:
 *   EM = 0x00 || 0x01 || PS || 0x00 || T
 * where:
 *   - PS is a padding string of 0xFF bytes of length k - 3 - |T| (k = emLen),
 *   - T is the DER-encoded DigestInfo (AlgorithmIdentifier + digest OCTET STRING).
 *
 * This function does not compute a hash or build the DigestInfo; the caller must
 * supply a complete, DER-encoded DigestInfo in 'digestinfo'.
 *
 * Requirements:
 *   - em must be a writable buffer of size emLen.
 *   - emLen must be >= digestinfoLen + 11, ensuring PS is at least 8 bytes
 *     as required by PKCS#1 v1.5.
 *
 * Parameters:
 *   @param em               Output buffer for the encoded message (EM).
 *   @param emLen           Length of EM in bytes (typically the RSA modulus size).
 *   @param digestinfo       Pointer to DER-encoded DigestInfo (T).
 *   @param digestinfoLen   Length of DigestInfo in bytes.
 *
 * Returns:
 *   @return 0 on success.
 *   @return Non-zero error code on failure (e.g., invalid arguments, insufficient emLen).
 *
 * Notes:
 *   - Intended for signature encoding (EMSA-PKCS1-v1_5), not for RSA encryption (EME-PKCS1-v1_5).
 *   - The operation is not guaranteed to be constant-time with respect to all inputs.
 */
static int EncodePkcs1V15(unsigned char* em, size_t emLen, const unsigned char* digestinfo, size_t digestinfoLen)
{
    if (!em || !digestinfo) {
        return 0;
    }
    if (emLen < digestinfoLen + 11) {
        return 0;
    }
    size_t psLen = emLen - digestinfoLen - 3;
    em[0] = 0x00;
    em[1] = 0x01;
    (void)memset_s(em + 2, psLen, 0xFF, psLen);
    em[2 + psLen] = 0x00;
    (void)memcpy_s(em + 3 + psLen, digestinfoLen, digestinfo, digestinfoLen);
    return 1;
}

/*
PKCS#1 v1.5 DigestInfo prefixes for SHA-1, SHA-256, SHA-384, and SHA-512.

These constants are ASN.1 DER-encoded prefixes for the DigestInfo structure used
with RSA PKCS#1 v1.5 signatures. Each prefix encodes:
  SEQUENCE {
    AlgorithmIdentifier { hashAlgorithmOID, NULL }
    OCTET STRING (hash)
  }

They include the AlgorithmIdentifier and the OCTET STRING tag and length; the raw
hash bytes must be appended to form the complete DigestInfo: prefix || hash.

- PKCS1_SHA1_PREFIX  (15 bytes): OID 1.3.14.3.2.26, expects 20-byte SHA-1 hash.
  Note: SHA-1 is deprecated and should not be used in new designs.
- PKCS1_SHA256_PREFIX (19 bytes): OID 2.16.840.1.101.3.4.2.1, expects 32-byte SHA-256 hash.
- PKCS1_SHA384_PREFIX (19 bytes): OID 2.16.840.1.101.3.4.2.2, expects 48-byte SHA-384 hash.
- PKCS1_SHA512_PREFIX (19 bytes): OID 2.16.840.1.101.3.4.2.3, expects 64-byte SHA-512 hash.

Intended use:
- Construct the DER DigestInfo for RSA PKCS#1 v1.5 by concatenating the appropriate
  prefix with the computed hash, then apply the RSA operation.
- Verification compares the decrypted signature with prefix || hash.
- Not applicable to RSA-PSS, which uses a different encoding (PSS padding).

These constants avoid runtime DER construction and ensure interoperability with TLS/X.509.
*/
static const unsigned char PKCS1_SHA1_PREFIX[] = {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14};
static const unsigned char PKCS1_SHA256_PREFIX[] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};
static const unsigned char PKCS1_SHA384_PREFIX[] = {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30};
static const unsigned char PKCS1_SHA512_PREFIX[] = {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40};

static const Pkcs1DigestDescriptor pkcs1DigestTable[] = {{"sha1", PKCS1_SHA1_PREFIX, sizeof(PKCS1_SHA1_PREFIX), 20},
                                                         {"sha256", PKCS1_SHA256_PREFIX, sizeof(PKCS1_SHA256_PREFIX), 32},
                                                         {"sha384", PKCS1_SHA384_PREFIX, sizeof(PKCS1_SHA384_PREFIX), 48},
                                                         {"sha512", PKCS1_SHA512_PREFIX, sizeof(PKCS1_SHA512_PREFIX), 64},
                                                         {NULL, NULL, 0, 0}};

static const Pkcs1DigestDescriptor* LookupDigestDescriptor(const char* name)
{
    if (!name) {
        return NULL;
    }

    for (const Pkcs1DigestDescriptor* d = pkcs1DigestTable; d->name != NULL; ++d) {
        if (strcasecmp(name, d->name) == 0) {
            return d;
        }
    }
    return NULL;
}

/**
 * @brief Computes the bit length of an RSA modulus interpreted as an unsigned big-endian integer.
 *
 * Interprets the input buffer as a big-endian non-negative integer and returns
 * the count of significant bits (floor(log2(n)) + 1). Leading zero bytes are ignored.
 *
 * @param n      Pointer to the modulus byte buffer in big-endian order.
 *               Must not be NULL if @p nLen is non-zero.
 * @param nLen  Length of the buffer in bytes.
 *
 * @return The number of significant bits in the modulus, or 0 if @p nLen is 0
 *         or the value is zero (all bytes are 0x00).
 *
 * @note This utility is typically used to validate RSA key sizes
 *       (e.g., enforcing a minimum modulus size such as 2048 bits).
 * @note The function does not allocate memory, does not modify its inputs,
 *       and has no side effects.
 * @note Thread-safety: The function is pure and reentrant; safe to call from
 *       multiple threads concurrently.
 */
static size_t RsaModulusBits(const unsigned char* n, size_t nLen)
{
    if (!n || nLen == 0) {
        return 0;
    }

    size_t i = 0;
    while (i < nLen && n[i] == 0) {
        ++i;
    }
    if (i == nLen) {
        return 0;
    }

    unsigned char byte = n[i];
    size_t leading = 0;
    while ((byte & 0x80) == 0) { // count leading zero bits in first non-zero byte
        byte <<= 1;
        ++leading;
    }

    size_t bits = (nLen - i) * 8 - leading; // total bits minus leading zeros
    return bits;
}

static const EVP_MD* ResolveMd(const char* name, DynMsg* dynMsg)
{
    if (!name) {
        return NULL;
    }
    if (strcasecmp(name, "sha256") == 0) {
        const EVP_MD* md = DYN_EVP_sha256(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha256");
        return md;
    }
    if (strcasecmp(name, "sha2-256") == 0) {
        const EVP_MD* md = DYN_EVP_sha256(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha256");
        return md;
    }
    if (strcasecmp(name, "sha384") == 0) {
        const EVP_MD* md = DYN_EVP_sha384(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha384");
        return md;
    }
    if (strcasecmp(name, "sha2-384") == 0) {
        const EVP_MD* md = DYN_EVP_sha384(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha384");
        return md;
    }
    if (strcasecmp(name, "sha512") == 0) {
        const EVP_MD* md = DYN_EVP_sha512(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha512");
        return md;
    }
    if (strcasecmp(name, "sha2-512") == 0) {
        const EVP_MD* md = DYN_EVP_sha512(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha512");
        return md;
    }
    if (strcasecmp(name, "sha1") == 0) {
        const EVP_MD* md = DYN_EVP_sha1(dynMsg);
        KeylessCheckDynMsg(dynMsg, "EVP_sha1");
        return md;
    }
    return NULL;
}

static void* KeylessSigNewctx(void* provctx, const char* propq)
{
    (void)provctx;
    (void)propq;
    const DynMsg* threadMsg = KeylessProviderGetThreadDynMsg();
    KeylessSignCtx* c = DYN_OPENSSL_zalloc(sizeof(*c), (DynMsg*)threadMsg);
    if (!c) {
        KeylessCheckDynMsg(threadMsg, "CRYPTO_secure_zalloc");
        return NULL;
    }
    /* we cannot know rsa/ec until sign_init, mark generic */
    c->pssSaltlen = RSA_PSS_SALTLEN_DIGEST;
    KeylessCopyDynMsg(&c->dynMsg, threadMsg);
    return c;
}

static void KeylessSigFreectx(void* vctx)
{
    KeylessSignCtx* c = vctx;
    if (!c) {
        return;
    }
    DynMsg* dynMsg = KeylessSigDynMsg(c);
    DynMsg localDynMsg;
    DynMsg* freeDynMsg = NULL;
    if (dynMsg) {
        localDynMsg = *dynMsg;
        freeDynMsg = &localDynMsg;
    }
    if (c->keyData) {
        KeylessKeyFreeExtern(c->keyData);
    }
    DYN_OPENSSL_secure_free(c, freeDynMsg);
    KeylessCheckDynMsg(freeDynMsg, "CRYPTO_secure_free");
}

static int KeylessSigInitInternal(KeylessSignCtx* c, void* keydata, int type)
{
    if (c->keyData) {
        KeylessKeyFreeExtern(c->keyData);
    }
    c->keyData = keydata;
    c->type = type;
    KeylessKey* key = (KeylessKey*)keydata;
    if (key) {
        KeylessCopyDynMsg(&c->dynMsg, &key->dynMsg);
    } else {
        KeylessCopyDynMsg(&c->dynMsg, KeylessProviderGetThreadDynMsg());
    }
    return 1;
}

static int KeylessSigSignInit(void* vctx, void* keydata, const OSSL_PARAM params[])
{
    KeylessSignCtx* c = vctx;
    if (!c || !keydata) {
        return 0;
    }
    KeylessProviderLog("[keyless] sign_init called\n");
    KeylessKeyUpRef(keydata);
    if (!KeylessSigInitInternal(c, keydata, KeylessKeyGetType(keydata))) {
        return 0;
    }
    /* Default to PKCS#1 v1.5 for maximum compatibility (TLS 1.2).
     * TLS 1.3 or callers that need PSS will explicitly set PAD_MODE. */
    c->padMode = RSA_PKCS1_PADDING;
    c->pssSaltlen = RSA_PSS_SALTLEN_DIGEST;
    c->digestName[0] = '\0';
    c->mgf1Name[0] = '\0';
    c->mdLen = 0;

    if (params && !KeylessSigSetCtxParams(c, params)) {
        return 0;
    }

    if (!c->digestName[0]) {
        strncpy_s(c->digestName, DIGEST_NAME_MAX_LEN, "sha256", sizeof(c->digestName) - 1);
        c->digestName[sizeof(c->digestName) - 1] = '\0';
    }
    NormalizeMd(c->digestName, sizeof(c->digestName));
    c->mdLen = DigestLenFor(c->digestName);

    if (c->padMode == RSA_PKCS1_PSS_PADDING && !c->mgf1Name[0]) {
        strncpy_s(c->mgf1Name, MGF1_NAME_MAX_LEN, c->digestName, sizeof(c->mgf1Name) - 1);
        c->mgf1Name[sizeof(c->mgf1Name) - 1] = '\0';
    }
    return 1;
}

/**
 * @brief Set context parameters for keyless signature.
 *
 * @param vctx   Pointer to KeylessSignCtx.
 * @param params Array of OSSL_PARAM to set.
 *
 * @return 1 on success, 0 on failure.
 *
 * @note If vctx or params is NULL, returns 1 (no-op).
 * @note Recognizes and sets:
 *  - OSSL_SIGNATURE_PARAM_PAD_MODE (int): RSA padding mode (e.g., RSA_PKCS1_PSS_PADDING).
 *  - OSSL_SIGNATURE_PARAM_DIGEST (string): Digest algorithm name (e.g., "sha256").
 *  - OSSL_SIGNATURE_PARAM_MGF1_DIGEST (string): MGF1 digest algorithm name (e.g., "sha256").
 *  - OSSL_SIGNATURE_PARAM_PSS_SALTLEN (int): RSA-PSS salt length.
 *
 *  Behavior:
 *  - If PAD_MODE is PSS and MGF1_DIGEST is not set, defaults MGF1_DIGEST to DIGEST.
 *  - Ignores unrecognized parameters.
 *  - Caller retains ownership of params; function does not modify or free them.
 */

static int KeylessSigSetCtxParams(void* vctx, const OSSL_PARAM params[])
{
    KeylessSignCtx* c = vctx;
    if (!c || !params) {
        return 1;
    }

    const OSSL_PARAM* p = NULL;
    DynMsg* dynMsg = KeylessSigDynMsg(c);

    if ((p = DYN_OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_PAD_MODE, dynMsg)) && p->data_type == OSSL_PARAM_INTEGER) {
        int pad = 0;
        if (DYN_OSSL_PARAM_get_int(p, &pad, dynMsg)) {
            c->padMode = pad;
            KeylessProviderLog("[keyless] set_ctx_params pad_mode=%d\n", c->padMode);
        }
    }

    if ((p = DYN_OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_DIGEST, dynMsg)) && p->data_type == OSSL_PARAM_UTF8_STRING) {
        strncpy_s(c->digestName, DIGEST_NAME_MAX_LEN, p->data, sizeof(c->digestName) - 1);
        c->digestName[sizeof(c->digestName) - 1] = '\0';
        NormalizeMd(c->digestName, sizeof(c->digestName));
        c->mdLen = DigestLenFor(c->digestName);
    }

    if ((p = DYN_OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_MGF1_DIGEST, dynMsg)) && p->data_type == OSSL_PARAM_UTF8_STRING) {
        strncpy_s(c->mgf1Name, MGF1_NAME_MAX_LEN, p->data, sizeof(c->mgf1Name) - 1);
        c->mgf1Name[sizeof(c->mgf1Name) - 1] = '\0';
        NormalizeMd(c->mgf1Name, sizeof(c->mgf1Name));
    }

    if ((p = DYN_OSSL_PARAM_locate_const(params, OSSL_SIGNATURE_PARAM_PSS_SALTLEN, dynMsg)) && p->data_type == OSSL_PARAM_INTEGER) {
        int salt = RSA_PSS_SALTLEN_DIGEST;
        if (DYN_OSSL_PARAM_get_int(p, &salt, dynMsg)) {
            c->pssSaltlen = salt;
            KeylessProviderLog("[keyless] set_ctx_params saltlen=%d\n", c->pssSaltlen);
        }
    }

    if (c->padMode == RSA_PKCS1_PSS_PADDING && c->digestName[0] && !c->mgf1Name[0]) {
        strncpy_s(c->mgf1Name, MGF1_NAME_MAX_LEN, c->digestName, sizeof(c->mgf1Name) - 1);
        c->mgf1Name[sizeof(c->mgf1Name) - 1] = '\0';
    }
    KeylessCheckDynMsg(dynMsg, "KeylessSigSetCtxParams");
    return 1;
}

static const OSSL_PARAM* KeylessSigSettableCtxParams(void* provctx)
{
    (void)provctx;

    /**
     * OSSL_SIGNATURE_PARAM_DIGEST for the digest algorithm name.
     * OSSL_SIGNATURE_PARAM_MGF1_DIGEST for the MGF1 digest algorithm name.
     * OSSL_SIGNATURE_PARAM_PSS_SALTLEN for the RSA-PSS salt length.
     * OSSL_SIGNATURE_PARAM_PAD_MODE for the RSA padding mode.
     *
     * RSA-PKCS#1 v1.5(TLS 1.2 common scenario) need OSSL_SIGNATURE_PARAM_DIGEST
     * RSA-PSS(TLS1.3 or TLS1.2 variant) need OSSL_SIGNATURE_PARAM_PAD_MODE, OSSL_SIGNATURE_PARAM_DIGEST. OSSL_SIGNATURE_PARAM_MGF1_DIGEST, OSSL_SIGNATURE_PARAM_PSS_SALTLEN is optional.
     * ECDSA(TLS1.2/TLS1.3) need OSSL_SIGNATURE_PARAM_DIGEST
     */
    static const OSSL_PARAM settable[] = {OSSL_PARAM_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST, NULL, 0), OSSL_PARAM_utf8_string(OSSL_SIGNATURE_PARAM_MGF1_DIGEST, NULL, 0),
                                          OSSL_PARAM_int(OSSL_SIGNATURE_PARAM_PSS_SALTLEN, NULL), OSSL_PARAM_int(OSSL_SIGNATURE_PARAM_PAD_MODE, NULL), OSSL_PARAM_END};
    return settable;
}

/* Map digest name to expected length */
static size_t DigestLenFor(const char* name)
{
    if (!name) {
        return 0;
    }
    if (strcasecmp(name, "SHA256") == 0 || strcasecmp(name, "SHA2-256") == 0) {
        return 32; // 256 bits / 8 = 32 bytes
    }
    if (strcasecmp(name, "SHA384") == 0 || strcasecmp(name, "SHA2-384") == 0) {
        return 48; // 384 bits / 8 = 48 bytes
    }
    if (strcasecmp(name, "SHA512") == 0 || strcasecmp(name, "SHA2-512") == 0) {
        return 64; // 512 bits / 8 = 64 bytes
    }
    if (strcasecmp(name, "SHA1") == 0 || strcasecmp(name, "SHA-1") == 0) {
        return 20; // 160 bits / 8 = 20 bytes
    }
    return 0; /* unsupported */
}

/**
 * @brief Builds and initializes the remote signing algorithm descriptor for the
 * keyless signature provider represented by the given context.
 *
 * @details
 * - Derives and encodes the algorithm parameters (e.g., key type, hash,
 *   padding/curve) required by the remote provider.
 * - Populates the context with the computed remote-algorithm metadata used by
 *   subsequent sign/verify requests.
 *
 * @param c: Non-NULL pointer to an initialized keyless_sig_ctx. The caller retains
 *      ownership of the context.
 *
 * @return 0 on success.
 * @return A negative project-specific or errno-compatible error code on failure.
 *
 */
static int BuildRemoteAlg(KeylessSignCtx* c)
{
    if (c->type == KEYLESS_KEY_TYPE_RSA) { /* RSA */
        const char* mode = (c->padMode == RSA_PKCS1_PSS_PADDING) ? "pss" : "pkcs1";
        int ret = snprintf_s(c->algName, sizeof(c->algName), sizeof(c->algName) - 1, "rsa-raw-%s", mode);
        return (ret == -1) ? 0 : 1;
    }

    if (c->type == KEYLESS_KEY_TYPE_EC) {
        const char* grp = KeylessKeyGetGroup(c->keyData);
        const char* gshort = grp ? grp : "unknown";
        int ret = snprintf_s(c->algName, sizeof(c->algName), sizeof(c->algName) - 1, "ecdsa-%s-%s", gshort, c->digestName[0] ? c->digestName : "sha256");
        return (ret == -1) ? 0: 1;
    }

    return 0;
}

static size_t KeylessRsaModulusLen(KeylessSignCtx* c)
{
    size_t modlen = KeylessKeyGetRsaNLen(c->keyData);
    if (modlen == 0) {
        modlen = KeylessKeyGetSize(c->keyData);
    }
    return modlen;
}

static int KeylessRsaHandleSizeRequest(size_t modlen, unsigned char* sig, size_t* siglen)
{
    if (sig != NULL) {
        return 0;
    }
    if (siglen) {
        *siglen = modlen;
    }
    KeylessProviderLog("[keyless] RSA size query returning %zu\n", modlen);
    return 1;
}

static int KeylessRsaDigestMessage(const unsigned char* tbs, size_t tbslen, const EVP_MD* hashMd, unsigned char* hash_tmp, size_t hash_tmp_sz, size_t expectedLen,
                                   const unsigned char** out, DynMsg* dynMsg)
{
    if (tbslen == expectedLen) {
        *out = tbs;
        return 1;
    }
    if (hash_tmp_sz < expectedLen) {
        return 0;
    }
    if (!DYN_EVP_Digest(tbs, tbslen, hash_tmp, NULL, hashMd, NULL, dynMsg)) {
        KeylessCheckDynMsg(dynMsg, "EVP_Digest");
        return 0;
    }
    *out = hash_tmp;
    return 1;
}

static int KeylessRsaPreparePssPayload(KeylessSignCtx* c, const unsigned char* tbs, size_t tbslen, const EVP_MD* hashMd, const EVP_MD* mgf1Md, unsigned char* em, size_t modlen,
                                       unsigned char* hash_tmp, size_t hash_tmp_sz, size_t hashLen, const unsigned char** payload, size_t* payloadLen)
{
    DynMsg* dynMsg = KeylessSigDynMsg(c);
    const unsigned char* mhash = NULL;
    if (!KeylessRsaDigestMessage(tbs, tbslen, hashMd, hash_tmp, hash_tmp_sz, hashLen, &mhash, dynMsg)) {
        return 0;
    }
    size_t mod_bits = RsaModulusBits(KeylessKeyGetN(c->keyData), KeylessKeyGetNLen(c->keyData));
    if (mod_bits == 0) {
        return 0;
    }
    if (!EncodePss(em, modlen, mod_bits - 1, mhash, hashLen, hashMd, mgf1Md, c->pssSaltlen, dynMsg)) {
        return 0;
    }
    *payload = em;
    *payloadLen = modlen;
    return 1;
}

static int KeylessRsaPreparePkcs1Payload(KeylessSignCtx* c, const unsigned char* tbs, size_t tbslen, const EVP_MD* hashMd, unsigned char* hash_tmp, size_t hash_tmp_sz,
                                         unsigned char* diBuf, size_t diBufSize, unsigned char* em, size_t modlen, const unsigned char** payload, size_t* payloadLen)
{
    DynMsg* dynMsg = KeylessSigDynMsg(c);
    const Pkcs1DigestDescriptor* desc = LookupDigestDescriptor(c->digestName);
    if (!desc) {
        return 0;
    }
    if (desc->hashLen > hash_tmp_sz) {
        return 0;
    }

    const unsigned char* digestinfo_ptr = NULL;
    size_t digestinfoLen = 0;

    if (tbslen == desc->derLen + desc->hashLen && memcmp(tbs, desc->derPrefix, desc->derLen) == 0) {
        digestinfo_ptr = tbs;
        digestinfoLen = tbslen;
    } else {
        const unsigned char* hashSrc = NULL;
        if (!KeylessRsaDigestMessage(tbs, tbslen, hashMd, hash_tmp, hash_tmp_sz, desc->hashLen, &hashSrc, dynMsg)) {
            return 0;
        }
        if (desc->derLen + desc->hashLen > diBufSize) {
            return 0;
        }
        int ret = memcpy_s(diBuf, diBufSize, desc->derPrefix, desc->derLen);
        if (ret != 0) {
            return 0;
        }

        ret = memcpy_s(diBuf + desc->derLen, diBufSize - desc->derLen, hashSrc, desc->hashLen);
        if (ret != 0) {
            return 0;
        }
        digestinfo_ptr = diBuf;
        digestinfoLen = desc->derLen + desc->hashLen;
    }

    if (!EncodePkcs1V15(em, modlen, digestinfo_ptr, digestinfoLen)) {
        return 0;
    }
    *payload = em;
    *payloadLen = modlen;
    return 1;
}

static int KeylessRsaPreparePayload(KeylessSignCtx* c, const unsigned char* tbs, size_t tbslen, const EVP_MD* hashMd, const EVP_MD* mgf1Md, unsigned char* em, size_t modlen,
                                    unsigned char* hash_tmp, size_t hash_tmp_sz, unsigned char* diBuf, size_t diBufSize, const unsigned char** payload, size_t* payloadLen)
{
    DynMsg* dynMsg = KeylessSigDynMsg(c);
    int mdSize = DYN_EVP_MD_get_size(hashMd, dynMsg);
    if (mdSize <= 0) {
        KEYLESS_CHECK_DYNMSG_RETURN(dynMsg, "EVP_MD_get_size", 0);
        return 0;
    }
    size_t hashLen = (size_t)mdSize;
    if (hashLen > hash_tmp_sz) {
        return 0;
    }

    if (c->padMode == RSA_PKCS1_PSS_PADDING) {
        return KeylessRsaPreparePssPayload(c, tbs, tbslen, hashMd, mgf1Md, em, modlen, hash_tmp, hash_tmp_sz, hashLen, payload, payloadLen);
    }

    return KeylessRsaPreparePkcs1Payload(c, tbs, tbslen, hashMd, hash_tmp, hash_tmp_sz, diBuf, diBufSize, em, modlen, payload, payloadLen);
}

static int KeylessRsaInvokeRemote(KeylessSignCtx* c, KeylessRemoteSignCb cb, unsigned char* sig, size_t* siglen, size_t modlen, const unsigned char* payload, size_t payloadLen)
{
    char* keyId = ((KeylessKey*)(c->keyData))->keyId;
    (void)keyId; /* avoid logging sensitive ids by default */
    KeylessProviderLog("[keyless] invoking remote RSA raw alg=%s payloadLen=%zu\n", c->algName, payloadLen);

    int64_t written = 0;
    unsigned char* remote = (unsigned char*)cb(keyId, c->algName, payload, (int64_t)payloadLen, &written);
    if (!remote || written <= 0) {
        return 0;
    }

    size_t got = (size_t)written;
    if (got > modlen) {
        free(remote);
        return 0;
    }

    size_t leading_zeros = modlen - got;
    if (leading_zeros) {
        (void)memset_s(sig, leading_zeros, 0, leading_zeros);
    }
    (void)memcpy_s(sig + leading_zeros, got, remote, got);
    if (siglen) {
        free(remote);
        *siglen = modlen;
    }
    KeylessProviderLog("[keyless] RSA sign complete alg=%s wrote=%zu (padded to %zu)\n", c->algName, got, modlen);
    return 1;
}

static int KeylessRsaSign(KeylessSignCtx* c, KeylessRemoteSignCb cb, unsigned char* sig, size_t* siglen, size_t sigsize, const unsigned char* tbs, size_t tbslen)
{
    size_t modlen = KeylessRsaModulusLen(c);
    if (modlen == 0) {
        return 0;
    }

    if (KeylessRsaHandleSizeRequest(modlen, sig, siglen)) {
        return 1;
    }

    if (sigsize == 0) {
        sigsize = modlen;
    }

    if (sigsize < modlen) {
        return 0;
    }

    if (!BuildRemoteAlg(c)) {
        return 0;
    }

    DynMsg* dynMsg = KeylessSigDynMsg(c);
    const EVP_MD* hashMd = ResolveMd(c->digestName, dynMsg);
    if (!hashMd) {
        int lib = KeylessErrorLibInit();
        KeylessProviderLog("[keyless] %d: unsupported digest %s\n", lib, c->digestName);
        return 0;
    }

    const EVP_MD* mgf1Md = ResolveMd(c->mgf1Name[0] ? c->mgf1Name : c->digestName, dynMsg);
    if (c->padMode == RSA_PKCS1_PSS_PADDING && !mgf1Md) {
        mgf1Md = hashMd;
    }

    unsigned char* em = DYN_OPENSSL_secure_malloc(modlen, dynMsg);
    if (!em) {
        KeylessCheckDynMsg(dynMsg, "CRYPTO_secure_malloc");
        return 0;
    }

    unsigned char hash_tmp[EVP_MAX_MD_SIZE] = {0};
    unsigned char diBuf[128] = {0}; // 128 bytes should be enough for any DigestInfo
    const unsigned char* payload = NULL;
    size_t payloadLen = 0;

    int ok = KeylessRsaPreparePayload(c, tbs, tbslen, hashMd, mgf1Md, em, modlen, hash_tmp, sizeof(hash_tmp), diBuf, sizeof(diBuf), &payload, &payloadLen);
    if (ok) {
        ok = KeylessRsaInvokeRemote(c, cb, sig, siglen, modlen, payload, payloadLen);
    }

    DYN_OPENSSL_cleanse(hash_tmp, sizeof(hash_tmp), dynMsg);
    DYN_OPENSSL_cleanse(diBuf, sizeof(diBuf), dynMsg);
    DYN_OPENSSL_cleanse(em, modlen, dynMsg);
    DYN_OPENSSL_secure_free(em, dynMsg);
    KeylessCheckDynMsg(dynMsg, "CRYPTO_secure_free");
    return ok;
}

static unsigned BitsFromUncompressedPoint(const unsigned char* pt, size_t len)
{
    if (!pt || len < 1) {
        return 0;
    }

    // The first byte of an uncompressed point is 0x04,
    // so when the code sees any other tag it immediately returns 0 to signal an unsupported format
    if (pt[0] != 0x04) {
        return 0;
    }

    if ((len - 1) % 2 != 0) {
        return 0; // must be even
    }

    size_t nbytes = (len - 1) / 2;
    if (nbytes == 0) {
        return 0;
    }

    // For standard NIST curves, nbytes is exactly ceil(bits/8), which is 32/48/66.
    // Here we just return nbytes*8 as a reasonable upper bound.
    // If the curve is non-standard and has a non-byte-aligned size (e.g. 521 bits),
    // this will return a slightly larger value (e.g. 528 bits) but that's acceptable.
    return (unsigned)(nbytes * 8);
}

static unsigned KeylesskeyCurveBits(const KeylessKey* k)
{
    if (!k || k->type != KEYLESS_KEY_TYPE_EC) {
        return 0;
    }
    unsigned bits = GetEcOrderBitsFromGroup(k->groupName);
    if (bits) {
        return bits;
    }

    bits = BitsFromUncompressedPoint(k->ecPoint, k->ecPointLen);
    return bits;
}

/* Return a safe upper bound for DER-encoded ECDSA signature length
 * for common prime curves based on curve bits. */
static size_t EcdsaDerSigMaxLenFromBits(unsigned bits)
{
    /* Worst-case DER sizes (r and s may need a leading 0x00):
     *  P-256/secp256r1,secp256k1: 72 bytes
     *  P-384/secp384r1:          104 bytes
     *  P-521/secp521r1:          141 bytes
     */
    if (bits <= 256u) {
        return 72u;
    }
    if (bits <= 384u) {
        return 104u;
    }
    if (bits <= 521u) {
        return 141u;
    }
    /* Fallback: two integers of ceil(bits/8)+1 plus headers. Conservative. */
    size_t nbytes = (bits + 7u) / 8u;
    size_t intLen = 1 /*tag*/ + 1 /*len*/ + (nbytes + 1) /*value with lead 0*/;
    size_t seq_payload = intLen * 2;
    size_t seqLen_hdr = (seq_payload < 128u) ? 2u : 3u; /* tag + len */
    return seq_payload + seqLen_hdr;
}

static int KeylessEcdsaSign(KeylessSignCtx* c, KeylessRemoteSignCb cb, unsigned char* sig, size_t* siglen, size_t sigsize, const unsigned char* tbs, size_t tbslen)
{
    if (!c || !cb || !tbs) {
        return 0;
    }
    if (!BuildRemoteAlg(c)) {
        return 0;
    }

    unsigned bits = KeylesskeyCurveBits((KeylessKey*)(c->keyData));
    size_t max_need = EcdsaDerSigMaxLenFromBits(bits);

    if (sig == NULL) {
        if (siglen) {
            *siglen = max_need;
        }
        KeylessProviderLog("[keyless] ECDSA size query returning %zu\n", max_need);
        return 1;
    }

    if (sigsize == 0) {
        sigsize = max_need;
    }

    char* keyId = ((KeylessKey*)(c->keyData))->keyId;
    KeylessProviderLog("[keyless] invoking remote ECDSA alg=%s dlen=%zu cap=%zu\n", c->algName, tbslen, sigsize);

    int64_t got = 0;
    unsigned char* out = (unsigned char*)cb(keyId, c->algName, tbs, (int64_t)tbslen, &got);
    if (!out || got <= 0) {
        free(out);
        KeylessProviderLog("[keyless] remote sign failed or returned empty\n");
        return 0;
    }

    if ((size_t)got > sigsize) {
        if (siglen) {
            *siglen = (size_t)got;
        }
        free(out);
        KeylessProviderLog("[keyless] ECDSA output too large: need=%ld cap=%zu (alg=%s)\n", (long)got, sigsize, c->algName);
        return 0;
    }

    int ret = memcpy_s(sig, sigsize, out, (size_t)got);
    if (ret != 0) {
        free(out);
        return 0;
    }

    if (siglen) {
        free(out);
        *siglen = (size_t)got;
    }
    KeylessProviderLog("[keyless] ECDSA sign complete alg=%s wrote=%ld\n", c->algName, (long)got);

    return 1;
}

/**
 * @brief Signs the supplied to-be-signed (TBS) data using the keyless signature provider context.
 *
 * This function implements the provider-side signature operation (OSSL_FUNC_signature_sign)
 * and typically delegates the signing to a remote/keyless service so that private
 * key material never resides locally.
 *
 * Parameters:
 *   @param vctx    Opaque signing context created by the keyless provider. It must
 *                  contain a valid key reference, algorithm parameters (e.g., RSA-PSS,
 *                  ECDSA, EdDSA), and any digest configuration determined at init time.
 *   @param sig     Output buffer for the resulting signature. May be NULL to query
 *                  the required signature size without producing a signature.
 *   @param siglen  On success, set to the number of bytes written to 'sig'. When
 *                  'sig' is NULL, set to the required size for the signature.
 *                  Must not be NULL.
 *   @param sigsize Size in bytes of the 'sig' buffer.
 *   @param tbs     Pointer to the data to be signed. Depending on the context/config,
 *                  this may be the raw message or a pre-computed digest (e.g., when
 *                  a specific OSSL_SIGNATURE_PARAM_DIGEST has been configured).
 *   @param tbslen  Length of the TBS data in bytes.
 *
 * Behavior:
 *   - If 'sig' is NULL, the function performs a size query and writes the required
 *     signature size to '*siglen' without producing a signature.
 *   - If 'sig' is not NULL, the signature is written to 'sig' and '*siglen' is set
 *     to the actual number of bytes produced, provided 'sigsize' is sufficient.
 *   - The signature format follows the algorithm configured in 'vctx'
 *     (e.g., DER-encoded ECDSA, RSA-PKCS#1 v1.5 or RSA-PSS).
 *
 * Return value:
 *   @return 1 on success; 0 on failure.
 *
 * Possible failure reasons:
 *   - 'vctx' or 'siglen' is NULL.
 *   - Unsupported or inconsistent algorithm/digest parameters in 'vctx'.
 *   - 'sig' provided but 'sigsize' is too small for the resulting signature
 *     (in this case '*siglen' may be set to the required size).
 *   - Invalid 'tbs'/'tbslen' combination or data incompatible with the configured digest mode.
 *   - Communication or service error when invoking the remote/keyless signer.
 *
 * Thread-safety:
 *   - The context 'vctx' must be used according to the provider's threading rules.
 *     Unless documented otherwise, do not share a single signing context across
 *     concurrent threads.
 *
 * Notes:
 *   - This function may perform network I/O and can block.
 *   - Callers should first query the required size by passing 'sig' = NULL, then
 *     allocate a buffer of that size and call again to produce the signature.
 */
static int KeylessSigSign(void* vctx, unsigned char* sig, size_t* siglen, size_t sigsize, const unsigned char* tbs, size_t tbslen)
{
    KeylessSignCtx* c = vctx;
    if (!c || !c->keyData) {
        return 0;
    }
    if (c->digestName[0] == '\0') {
        strncpy_s(c->digestName, DIGEST_NAME_MAX_LEN, "sha256", sizeof(c->digestName) - 1);
        c->digestName[sizeof(c->digestName) - 1] = '\0';
    }
    if (c->mdLen == 0) {
        c->mdLen = DigestLenFor(c->digestName);
    }

    KeylessProviderLog("[keyless] sign() entry type=%d pad_mode=%d MGF1=%s, pss_saltlen=%d, sig_null=%d tbslen=%zu sigsize=%zu\n", c->type, c->padMode,
                         c->mgf1Name[0] ? c->mgf1Name : "default", c->pssSaltlen, sig ? 0 : 1, tbslen, sigsize);

    KeylessKey* key = (KeylessKey*)(c->keyData);
    const char* keyId = key ? key->keyId : NULL;
    KeylessRemoteSignCb cb = KeylessLookupSignCb(keyId);
    if (!cb) {
        int lib = KeylessErrorLibInit();
        KeylessProviderLog("[keyless] %d: sign callback not set\n", lib);
        return 0;
    }

    if (c->type == KEYLESS_KEY_TYPE_RSA) {
        return KeylessRsaSign(c, cb, sig, siglen, sigsize, tbs, tbslen);
    }

    if (c->type == KEYLESS_KEY_TYPE_EC) {
        return KeylessEcdsaSign(c, cb, sig, siglen, sigsize, tbs, tbslen);
    }

    return 0;
}

static int KeylessSigGetCtxParams(void* vctx, OSSL_PARAM params[])
{
    KeylessSignCtx* c = vctx;
    if (!c) {
        return 0;
    }
    OSSL_PARAM* p = NULL;
    DynMsg* dynMsg = KeylessSigDynMsg(c);
    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_SIGNATURE_PARAM_DIGEST, dynMsg))) {
        DYN_OSSL_PARAM_set_utf8_string(p, c->digestName[0] ? c->digestName : "sha256", dynMsg);
    }
    KeylessCheckDynMsg(dynMsg, "KeylessSigGetCtxParams");
    return 1;
}

static const OSSL_PARAM* KeylessSigGettableCtxParams(void* provctx)
{
    (void)provctx;
    static const OSSL_PARAM gettable[] = {OSSL_PARAM_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST, NULL, 0), OSSL_PARAM_int(OSSL_SIGNATURE_PARAM_PSS_SALTLEN, NULL), OSSL_PARAM_END};
    return gettable;
}

static void* KeylessSigDupctx(void* vctx)
{
    KeylessSignCtx* src = vctx;
    if (!src) {
        return NULL;
    }
    DynMsg* dynMsg = KeylessSigDynMsg(src);
    KeylessSignCtx* c = DYN_OPENSSL_memdup(src, sizeof(*src), dynMsg);
    if (!c) {
        KeylessCheckDynMsg(dynMsg, "CRYPTO_memdup");
        return NULL;
    }
    if (c->keyData) {
        KeylessKeyUpRef(c->keyData);
    }
    return c;
}

/*
* Provider only needs to perform server-side signing for TLS.
* Verification is handled entirely on the client side (or by OpenSSL itself),
* We don't support remote verify (not needed for TLS server side), just fail gracefully
*/
static int KeylessSigVerifyInit(void* vctx, void* keydata, const OSSL_PARAM params[])
{
    (void)vctx;
    (void)keydata;
    (void)params;
    return 0;
}

/*
* Provider only needs to perform server-side signing for TLS.
* Verification is handled entirely on the client side (or by OpenSSL itself),
* We don't support remote verify (not needed for TLS server side), just fail gracefully
*/
static int KeylessSigVerify(void* vctx, const unsigned char* sig, size_t siglen, const unsigned char* tbs, size_t tbslen)
{
    (void)vctx;
    (void)sig;
    (void)siglen;
    (void)tbs;
    (void)tbslen;
    return 0;
}

/**
 * @brief Initializes the keyless signature context for digest-based signing.
 *
 * This function prepares the context for a signature operation where the input
 * to the sign function is expected to be a precomputed digest rather than raw data.
 * It sets up the digest algorithm, key reference, and relevant padding parameters.
 *
 * @param vctx      Pointer to the keyless signature context to initialize.
 * @param mdname    Name of the digest algorithm to use (e.g., "sha256").
 * @param keydata   Provider key object to use for signing.
 * @param params    Optional array of OSSL_PARAM parameters for additional configuration.
 *
 * @return 1 on success, 0 on failure (e.g., unsupported digest or invalid arguments).
 */
static int KeylessSigDigestSignInit(void* vctx, const char* mdname, void* keydata, const OSSL_PARAM params[])
{
    /* Accept digest name then expect pre-hash sign() call: store name only */
    KeylessSignCtx* c = (KeylessSignCtx*)vctx;
    if (!c || !keydata) {
        return 0;
    }

    KeylessKeyUpRef(keydata);
    KeylessSigInitInternal(c, keydata, KeylessKeyGetType(keydata));
    /* Default to PKCS#1 v1.5 for TLS 1.2 compatibility. */
    c->padMode = RSA_PKCS1_PADDING;
    if (c->type == KEYLESS_KEY_TYPE_RSA) {
        c->pssSaltlen = RSA_PSS_SALTLEN_DIGEST;
    }
    c->digestName[0] = '\0';
    c->mgf1Name[0] = '\0';
    c->digestName[0] = '\0';
    c->mgf1Name[0] = '\0';

    if (mdname) {
        strncpy_s(c->digestName, DIGEST_NAME_MAX_LEN, mdname, sizeof(c->digestName) - 1);
        c->digestName[sizeof(c->digestName) - 1] = '\0';
        NormalizeMd(c->digestName, sizeof(c->digestName));
    }

    if (params && !KeylessSigSetCtxParams(c, params)) {
        return 0;
    }
    if (!c->digestName[0]) {
        NormalizeMd(c->digestName, sizeof(c->digestName));
    }
    c->mdLen = DigestLenFor(c->digestName);
    if (c->type == KEYLESS_KEY_TYPE_RSA && c->padMode == RSA_PKCS1_PSS_PADDING && !c->mgf1Name[0]) {
        strncpy_s(c->mgf1Name, MGF1_NAME_MAX_LEN, c->digestName, sizeof(c->mgf1Name) - 1);
        c->mgf1Name[sizeof(c->mgf1Name) - 1] = '\0';
    }
    return 1;
}

static int KeylessSigDigestSign(void* vctx, unsigned char* sig, size_t* siglen, size_t sigsize, const unsigned char* data, size_t datalen)
{
    // Expect caller already hashed -> behave like sign
    return KeylessSigSign(vctx, sig, siglen, sigsize, data, datalen);
}

/*
*  Provider only needs to perform server-side signing for TLS.
*  Verification is handled entirely on the client side (or by OpenSSL itself),
*  We don't support remote verify (not needed for TLS server side), just fail gracefully
*/
static int KeylessSigDigestVerifyInit(void* vctx, const char* mdname, void* keydata, const OSSL_PARAM params[])
{
    (void)vctx;
    (void)mdname;
    (void)keydata;
    (void)params;
    return 0;
}

/*
*  Provider only needs to perform server-side signing for TLS.
*  Verification is handled entirely on the client side (or by OpenSSL itself),
*  We don't support remote verify (not needed for TLS server side), just fail gracefully
*/
static int KeylessSigDigestVerify(void* vctx, const unsigned char* sig, size_t siglen, const unsigned char* data, size_t datalen)
{
    (void)vctx;
    (void)sig;
    (void)siglen;
    (void)data;
    (void)datalen;
    return 0;
}

/* For now RSA / ECDSA share same function implementations (sign only) */
static const OSSL_DISPATCH keylessRsaSigFunctions[] = {{OSSL_FUNC_SIGNATURE_NEWCTX, (void (*)(void))KeylessSigNewctx},
                                                       {OSSL_FUNC_SIGNATURE_FREECTX, (void (*)(void))KeylessSigFreectx},
                                                       {OSSL_FUNC_SIGNATURE_SIGN_INIT, (void (*)(void))KeylessSigSignInit},
                                                       {OSSL_FUNC_SIGNATURE_SIGN, (void (*)(void))KeylessSigSign},
                                                       {OSSL_FUNC_SIGNATURE_VERIFY_INIT, (void (*)(void))KeylessSigVerifyInit},
                                                       {OSSL_FUNC_SIGNATURE_VERIFY, (void (*)(void))KeylessSigVerify},
                                                       {OSSL_FUNC_SIGNATURE_DIGEST_SIGN_INIT, (void (*)(void))KeylessSigDigestSignInit},
                                                       {OSSL_FUNC_SIGNATURE_DIGEST_SIGN, (void (*)(void))KeylessSigDigestSign},
                                                       {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_INIT, (void (*)(void))KeylessSigDigestVerifyInit},
                                                       {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY, (void (*)(void))KeylessSigDigestVerify},
                                                       {OSSL_FUNC_SIGNATURE_GET_CTX_PARAMS, (void (*)(void))KeylessSigGetCtxParams},
                                                       {OSSL_FUNC_SIGNATURE_GETTABLE_CTX_PARAMS, (void (*)(void))KeylessSigGettableCtxParams},
                                                       {OSSL_FUNC_SIGNATURE_SET_CTX_PARAMS, (void (*)(void))KeylessSigSetCtxParams},
                                                       {OSSL_FUNC_SIGNATURE_SETTABLE_CTX_PARAMS, (void (*)(void))KeylessSigSettableCtxParams},
                                                       {OSSL_FUNC_SIGNATURE_DUPCTX, (void (*)(void))KeylessSigDupctx},
                                                       {0, NULL}};

static const OSSL_DISPATCH keylessEcdsaSigFunctions[] = {{OSSL_FUNC_SIGNATURE_NEWCTX, (void (*)(void))KeylessSigNewctx},
                                                         {OSSL_FUNC_SIGNATURE_FREECTX, (void (*)(void))KeylessSigFreectx},
                                                         {OSSL_FUNC_SIGNATURE_SIGN_INIT, (void (*)(void))KeylessSigSignInit},
                                                         {OSSL_FUNC_SIGNATURE_SIGN, (void (*)(void))KeylessSigSign},
                                                         {OSSL_FUNC_SIGNATURE_VERIFY_INIT, (void (*)(void))KeylessSigVerifyInit},
                                                         {OSSL_FUNC_SIGNATURE_VERIFY, (void (*)(void))KeylessSigVerify},
                                                         {OSSL_FUNC_SIGNATURE_DIGEST_SIGN_INIT, (void (*)(void))KeylessSigDigestSignInit},
                                                         {OSSL_FUNC_SIGNATURE_DIGEST_SIGN, (void (*)(void))KeylessSigDigestSign},
                                                         {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY_INIT, (void (*)(void))KeylessSigDigestVerifyInit},
                                                         {OSSL_FUNC_SIGNATURE_DIGEST_VERIFY, (void (*)(void))KeylessSigDigestVerify},
                                                         {OSSL_FUNC_SIGNATURE_GET_CTX_PARAMS, (void (*)(void))KeylessSigGetCtxParams},
                                                         {OSSL_FUNC_SIGNATURE_GETTABLE_CTX_PARAMS, (void (*)(void))KeylessSigGettableCtxParams},
                                                         {OSSL_FUNC_SIGNATURE_SET_CTX_PARAMS, (void (*)(void))KeylessSigSetCtxParams},
                                                         {OSSL_FUNC_SIGNATURE_SETTABLE_CTX_PARAMS, (void (*)(void))KeylessSigSettableCtxParams},
                                                         {OSSL_FUNC_SIGNATURE_DUPCTX, (void (*)(void))KeylessSigDupctx},
                                                         {0, NULL}};

/* Export algorithm list */
const OSSL_ALGORITHM keylessSignatureAlgorithms[] = {
        {"RSA", "provider=keyless", keylessRsaSigFunctions, "Keyless RSA"}, {"ECDSA", "provider=keyless", keylessEcdsaSigFunctions, "Keyless ECDSA"}, {NULL, NULL, NULL, NULL}};
