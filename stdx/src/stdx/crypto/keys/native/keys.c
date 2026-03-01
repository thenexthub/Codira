/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "securec.h"
#include "api.h"
#include "opensslSymbols.h"

#define CODEKEYS_FAIL (-1)
#define CODEKEYS_OK 1
#define KEY_BUFFER_SIZE 65536

extern int32_t DYN_CODE_KEYS_OAEPSetting(
    EVP_PKEY_CTX* ctx, const char* label, const EVP_MD* md, const EVP_MD* mgf, DynMsg* dynMsg)
{
    if (DYN_EVP_PKEY_CTX_set_rsa_oaep_md(ctx, md, dynMsg) <= 0 ||
        DYN_EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, mgf, dynMsg) <= 0) {
        return CODEKEYS_FAIL;
    }

    // if label is NULL, or length is 0, EVP_PKEY_CTX_set0_rsa_oaep_label would clear the original label in ctx
    if (NULL == label || strlen(label) == 0) {
        // a bug in openssl3 (https://github.com/openssl/openssl/issues/21288)
        // EVP_PKEY_CTX_set0_rsa_oaep_label behavior wrong when accept NULL
        // a walkaround fix, pLabel will be freed in EVP_PKEY_CTX_set0_rsa_oaep_label
        char* pLabel = DYN_CRYPTO_malloc(1, "", 0, dynMsg);
        if (!pLabel) {
            return CODEKEYS_FAIL;
        }
        pLabel[0] = '\0';
        int ret = DYN_EVP_PKEY_CTX_set0_rsa_oaep_label(ctx, pLabel, 0, dynMsg);
        if (ret <= 0) {
            DYN_CRYPTO_free(pLabel, dynMsg);
            return CODEKEYS_FAIL;
        }
        return CODEKEYS_OK;
    }

    size_t labelLen = strlen(label);
    if (labelLen > INT32_MAX) {
        return CODEKEYS_FAIL;
    }

    char* labelSsl = DYN_CRYPTO_malloc(labelLen + 1, "", 0, dynMsg);
    if (!labelSsl) {
        return CODEKEYS_FAIL;
    }

    if (DYN_OPENSSL_strlcpy(labelSsl, label, labelLen + 1, dynMsg) != labelLen) {
        DYN_CRYPTO_free(labelSsl, dynMsg);
        return CODEKEYS_FAIL;
    }

    if (DYN_EVP_PKEY_CTX_set0_rsa_oaep_label(ctx, labelSsl, (int)(labelLen), dynMsg) <= 0) {
        DYN_CRYPTO_free(labelSsl, dynMsg);
        return CODEKEYS_FAIL;
    }

    // labelSsl has been freed in func EVP_PKEY_CTX_set0_rsa_oaep_label if return 1
    return CODEKEYS_OK;
}

static int DYN_CodePemPasswordCb(char* buf, int size, int rwflag, void* userdata)
{
    // userdata = password zero terminated string
    if (!userdata) {
        return 0;
    }
    size_t len = strlen((const char*)userdata) + 1;
    if (memcpy_s(buf, (size_t)(unsigned int)(size), userdata, len) == EOK) {
        return (int)len;
    }
    return 0;
}

static BIO* InitBioWithPem(const void* pem, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    if (!X509CheckNotNull(exception, (void*)(pem), "pem", dynMsg)) {
        return NULL;
    }
    if (!X509CheckOrFillException(exception, length < (size_t)INT_MAX, "length < (size_t)INT_MAX", dynMsg)) {
        return NULL;
    }
    if (!X509CheckOrFillException(exception, length > 0, "length > 0", dynMsg)) {
        return NULL;
    }
    BIO* mem = DYN_BIO_new_mem(dynMsg);
    if (!mem) {
        X509HandleError(exception, "Failed to create BIO for PEM", dynMsg);
        return NULL;
    }
    int writeSize = (int)length;
    if (DYN_BIO_write(mem, pem, writeSize, dynMsg) != writeSize) {
        X509HandleError(exception, "Failed to write PEM to BIO", dynMsg);
        DYN_BIO_vfree(mem, dynMsg);
        return NULL;
    }
    return mem;
}

static int DecryptKeyImpl(const void* keyBody, size_t keyLength, const EVP_CIPHER* cipher,
    const unsigned char* decryptKey, const unsigned char* iv, unsigned char* outputBuffer, int bufferLength,
    ExceptionData* exception, DynMsg* dynMsg)
{
    if (keyBody == NULL) {
        X509HandleError(exception, "No key body provided", dynMsg);
        return 0;
    }
    if (keyLength > (size_t)INT_MAX) {
        X509HandleError(exception, "Encrypted key is too long", dynMsg);
        return 0;
    }

    EVP_CIPHER_CTX* ctx = DYN_EVP_CIPHER_CTX_new(dynMsg);
    int result = 1;
    if (ctx == NULL) {
        result = 0;
        X509HandleError(exception, "Failed to allocate cipher context", dynMsg);
    }
    if (result != 0) {
        result = DYN_EVP_DecryptInit_ex(ctx, cipher, NULL, decryptKey, iv, dynMsg);
    }
    int decryptedTotal = 0;
    if (result != 0) {
        int decrypted = bufferLength;
        result = DYN_EVP_DecryptUpdate(ctx, outputBuffer, &decrypted, keyBody, (int)keyLength, dynMsg);
        decryptedTotal = decrypted;
    }
    if (result != 0) {
        int decrypted = bufferLength - decryptedTotal;
        result = DYN_EVP_DecryptFinal_ex(ctx, outputBuffer + decryptedTotal, &decrypted, dynMsg);
        decryptedTotal += decrypted;
    }
    DYN_EVP_CIPHER_CTX_free(ctx, dynMsg);
    if (result == 0) {
        decryptedTotal = 0;
        X509HandleError(exception, "Failed to decrypt private key", dynMsg);
    }

    return decryptedTotal;
}

static EVP_PKEY* DecodePrivateKey(const void* keyBody, long keySize, ExceptionData* exception, DynMsg* dynMsg)
{
    const unsigned char* dataptr = (const unsigned char*)keyBody;
    EVP_PKEY* pkey = DYN_d2i_AutoPrivateKey(NULL, &dataptr, keySize, dynMsg);
    if (pkey == NULL) {
        X509HandleError(exception,
            "Failed to load private key, it's either corrupted, password is wrong or the format is unsupported",
            dynMsg);
    }
    return pkey;
}

// for those encrypted keys that are not PKCS8 we need to decrypt them manually
static EVP_PKEY* DecryptKey(
    const void* keyBody, size_t keyLength, EncryptedKeyParams* params, ExceptionData* exception, DynMsg* dynMsg)
{
    if (params->cipherName == NULL) {
        X509HandleError(exception, "cipherName is NULL", dynMsg);
        return NULL;
    }
    const EVP_CIPHER* cipher = DYN_EVP_get_cipherbyname(params->cipherName, dynMsg);
    if (cipher == NULL) {
        X509HandleError(exception, "Unknown cipher", dynMsg);
        return NULL;
    }
    if (params->password == NULL) {
        X509HandleError(exception, "No password specified for decryption", dynMsg);
        return NULL;
    }
    size_t passwordLength = strlen(params->password);
    if (passwordLength > (size_t)INT_MAX) {
        X509HandleError(exception, "Password is too long", dynMsg);
        return NULL;
    }

    // this is not exactly perfect but EVP_CIPHER_iv_length() function is not portable
    // across openssl versions 1.x and 3.x so we can't use it to check for the particular size
    // and keys having cut-corrupted IVs may pass here but will cause failures
    // at later stages producing less descriptive errors that is acceptable
    // When we pass IV to openssl, we can't specify it's size because it implies that it should be
    // already normalized so we HAVE to copy to a buffer of the proper size.
    // If we pass the original pointer to openssl directly, we may pass garbage or even cause crash
    unsigned char ivCopy[EVP_MAX_IV_LENGTH] = {};
    if (memcpy_s((void*)ivCopy, sizeof(ivCopy), params->iv, params->ivLength) != EOK) {
        X509HandleError(exception, "IV length is wrong", dynMsg);
        return NULL;
    }

    // the key that is used for private key decryption
    // it is constructed from the password and IV
    // this is NOT our resulting key: it's a key for decrypting key
    unsigned char decryptKey[EVP_MAX_KEY_LENGTH] = {};

    // we are passing IV -> salt and NULL -> iv: this is intentionally
    // using MD5 shouldn't cause concerns here as the impact is limited since we are using IV as salt
    // it also required for pbk and can't be replaced as it's a part of the spec
    if (DYN_EVP_BytesToKey(cipher, DYN_EVP_md5(dynMsg), ivCopy, (const unsigned char*)params->password,
            (int)passwordLength, 1, decryptKey, NULL, dynMsg) == 0) {
        X509HandleError(exception, "Failed to construct decryption key", dynMsg);
        return NULL;
    }
    (void)memset_s((void*)params->password, passwordLength, 0, passwordLength);
    unsigned char* decryptedKeyBuffer = DYN_OPENSSL_secure_malloc(KEY_BUFFER_SIZE, dynMsg);
    if (decryptedKeyBuffer == NULL) {
        X509HandleError(exception, "Failed to allocate buffer for key decryption", dynMsg);
        return NULL;
    }
    EVP_PKEY* pkey = NULL;
    int bytesDecrypted = DecryptKeyImpl(
        keyBody, keyLength, cipher, decryptKey, ivCopy, decryptedKeyBuffer, KEY_BUFFER_SIZE, exception, dynMsg);
    if (bytesDecrypted > 0) {
        pkey = DecodePrivateKey(decryptedKeyBuffer, bytesDecrypted, exception, dynMsg);
    }
    DYN_OPENSSL_cleanse(decryptKey, sizeof(decryptKey), dynMsg);
    DYN_OPENSSL_secure_free(decryptedKeyBuffer, dynMsg);
    return pkey;
}

static EVP_PKEY* LoadEncryptedKey(
    const void* keyBody, size_t length, EncryptedKeyParams* params, ExceptionData* exception, DynMsg* dynMsg)
{
    if (!X509CheckNotNull(exception, (void*)(keyBody), "keyBody", dynMsg)) {
        return NULL;
    }

    if (!X509CheckOrFillException(exception, length > 0, "length > 0", dynMsg)) {
        return NULL;
    }

    if (!X509CheckNotNull(exception, (void*)(params), "params", dynMsg)) {
        return NULL;
    }

    if (params == NULL) {
        return 0;
    }

    if (params->iv != NULL && params->ivLength > 0 && params->cipherName != NULL) {
        return DecryptKey(keyBody, length, params, exception, dynMsg);
    }

    BIO* mem = InitBioWithPem(keyBody, length, exception, dynMsg);
    if (mem == NULL) {
        return 0;
    }

    size_t passwordLength = strlen(params->password);
    EVP_PKEY* pkey = DYN_d2i_PKCS8PrivateKey_bio(mem, NULL, DYN_CodePemPasswordCb, (void*)params->password, dynMsg);
    if (pkey == NULL) {
        X509HandleError(
            exception, "Failed to load private key, the password is incorrect or the key is corrupted", dynMsg);
    }
    DYN_BIO_vfree(mem, dynMsg);
    (void)memset_s((void*)params->password, passwordLength, 0, passwordLength);

    return pkey;
}

// returns true if the key is for sure a PKCS8 encrypted key
// or false when uncertain
static bool IsEncryptedPkcs8(X509_SIG* p8, DynMsg* dynMsg)
{
    const X509_ALGOR* algorithm = NULL;
    const ASN1_OCTET_STRING* str = NULL;
    DYN_X509_SIG_get0(p8, (const void**)&algorithm, (const void**)&str, dynMsg);
    if (algorithm != NULL) {
        const ASN1_OBJECT* algOid;
        int paramtype;
        const void* param;
        DYN_X509_ALGOR_get0((const void**)&algOid, &paramtype, (const void**)&param, algorithm, dynMsg);
        (void)paramtype;
        (void)param;

        int algNid = DYN_OBJ_obj2nid(algOid, dynMsg);
        if (algNid == NID_pbes2 || algNid == NID_id_scrypt) {
            return true;
        }
    }

    return false;
}

// returns true if the key is for sure a PKCS8 encrypted key
// or false when uncertain
static bool IsEncryptedPkcs8Key(const void* keyBody, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    bool encrypted = false;
    BIO* mem = InitBioWithPem(keyBody, length, exception, dynMsg);
    if (mem == NULL) {
        return false;
    }
    X509_SIG* p8 = DYN_d2i_PKCS8_bio(mem, NULL, dynMsg);
    if (p8 != NULL) {
        encrypted = IsEncryptedPkcs8(p8, dynMsg);
        DYN_X509_SIG_free(p8, dynMsg);
    }
    DYN_BIO_vfree(mem, dynMsg);
    return encrypted;
}

static EVP_PKEY* LoadPrivateKey(const void* keyBody, size_t keySize, ExceptionData* exception, DynMsg* dynMsg)
{
    if (!X509CheckNotNull(exception, (void*)(keyBody), "keyBody", dynMsg)) {
        return NULL;
    }
    if (!X509CheckOrFillException(exception, keySize > 0, "keySize > 0", dynMsg)) {
        return NULL;
    }
    if (IsEncryptedPkcs8Key(keyBody, keySize, exception, dynMsg)) {
        X509HandleError(exception, "Failed to load private key, no password specified for encrypted PKCS8 key", dynMsg);
        return NULL;
    }
    if (!X509CheckOrFillException(exception, keySize < (size_t)LONG_MAX, "keySize < (size_t)LONG_MAX", dynMsg)) {
        return NULL;
    }
    return DecodePrivateKey(keyBody, (long)keySize, exception, dynMsg);
}

// this does always produce PKCS8 keys
static int32_t EncryptPrivateKey(EVP_PKEY* key, const char* password, char** resultBody, size_t* resultSize,
    ExceptionData* exception, DynMsg* dynMsg)
{
    *resultBody = NULL;
    *resultSize = 0;
    size_t passwordLength = strlen(password);
    if (passwordLength > (size_t)INT_MAX) {
        X509HandleError(exception, "Password is too long", dynMsg);
        return CODE_FAIL;
    }
    PKCS8_PRIV_KEY_INFO* p8info = DYN_EVP_PKEY2PKCS8(key, dynMsg);
    if (p8info == NULL) {
        X509HandleError(exception, "Failed to convert key to PKCS8", dynMsg);
        return CODE_FAIL;
    }
    // the openssl CLI tool does always use this by default
    EVP_CIPHER* cipher = (EVP_CIPHER*)DYN_EVP_aes_256_cbc(dynMsg);
    X509_ALGOR* algorithm = DYN_PKCS5_pbe2_set_iv(cipher, PKCS5_DEFAULT_ITER, NULL, 0, NULL, -1, dynMsg);
    if (algorithm == NULL) {
        X509HandleError(exception, "Failed to configure encyption for PKCS8 encryption", dynMsg);
        return CODE_FAIL;
    }
    X509_SIG* p8 = DYN_PKCS8_set0_pbe(password, (int)passwordLength, p8info, algorithm, dynMsg);
    if (p8 == NULL) {
        X509HandleError(exception, "Failed to configure encyption for PKCS8 encryption", dynMsg);
        return CODE_FAIL;
    }
    BIO* mem = DYN_BIO_new_mem(dynMsg);
    if (!mem) {
        X509HandleError(exception, "Failed to create BIO for PEM", dynMsg);
        return CODE_FAIL;
    }
    int result = DYN_i2d_PKCS8_bio(mem, p8, dynMsg);
    BUF_MEM* ptr = NULL;
    long getMemResult = DYN_BIO_get_mem_ptr(mem, &ptr, dynMsg);
    if (result == 1 && getMemResult == 1 && ptr != NULL && ptr->data != NULL && ptr->length > 0) {
        *resultBody = DYN_OPENSSL_memdup(ptr->data, ptr->length, dynMsg);
        *resultSize = ptr->length;
    }
    DYN_BIO_vfree(mem, dynMsg);
    DYN_PKCS8_PRIV_KEY_INFO_free(p8info, dynMsg);
    DYN_X509_SIG_free(p8, dynMsg);
    if (result == 0) {
        X509HandleError(exception, "Failed to encode encrypted private key", dynMsg);
        return CODE_FAIL;
    }

    return CODE_OK;
}

/**
 * Encrypt private key located at keyBody:length using the specified password (required)
 * and put the resulting ecrypted key to a new allocated memory and put
 * it's pointer and size to resultBody and resultSize (required).
 *
 * The resultBody content should be explicitly freed by caller.
 *
 * @return CODE_OK or CODE_FAIL
 */
extern int32_t DYN_CODEX509EncryptPrivateKey(char* keyBody, size_t keySize, const char* password, char** resultBody,
    size_t* resultSize, ExceptionData* exception, DynMsg* dynMsg)
{
    int ret = CODE_OK;
    if (exception == NULL) {
        return CODE_FAIL;
    };
    X509ExceptionClear(exception, dynMsg);
    if (!X509CheckNotNull(exception, (void*)(resultBody), "resultBody", dynMsg) ||
        !X509CheckNotNull(exception, (void*)(resultSize), "resultSize", dynMsg) ||
        !X509CheckNotNull(exception, (void*)(password), "password", dynMsg) ||
        !X509CheckOrFillException(exception, password[0] != 0, "password[0] != 0", dynMsg)) {
        return CODE_FAIL;
    }
    *resultBody = NULL;
    *resultSize = 0;
    // we expect input key is unencrypted
    EVP_PKEY* key = LoadPrivateKey(keyBody, keySize, exception, dynMsg);
    if (key == NULL) {
        return CODE_FAIL;
    }
    ret = EncryptPrivateKey(key, password, resultBody, resultSize, exception, dynMsg);
    DYN_EVP_PKEY_free(key, dynMsg);
    return ret;
}

/**
 * Try to briefly describe the provided key: key kind and bits strength.
 * @return allocated memory (should be freed) or NULL if faied
 */
extern const char* DYN_CODEX509DescribePrivateKey(
    const void* keyBody, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return NULL;
    };
    X509ExceptionClear(exception, dynMsg);
    if (!X509CheckNotNull(exception, (void*)(keyBody), "keyBody", dynMsg)) {
        return NULL;
    }
    if (!X509CheckOrFillException(exception, length > 0, "length > 0", dynMsg)) {
        return NULL;
    }
    EVP_PKEY* key = LoadPrivateKey(keyBody, length, exception, dynMsg);
    if (!key) {
        return NULL;
    }
    const char* message = X509DescribePrivateKey(key, dynMsg);
    DYN_EVP_PKEY_free(key, dynMsg);

    return message;
}

/**
 * Decrypt private key located at keyBody:length using the specified password (required)
 * and put the resulting decrypted key to a new allocated memory and put
 * it's pointer and size to resultBody and resultSize (required).
 * Optionally, it does describe the key and put the allocated text to the place pointed
 * by description. If the description pointer is NULL, no description will be generated.
 *
 * Both resultBody and description (if not NULL) should be explicitly freed by caller.
 * @return CODE_OK or CODE_FAIL
 */
extern int32_t CODEX509DecryptPrivateKey(const void* keyBody, size_t length, char** resultBody, size_t* resultSize,
    EncryptedKeyParams* params, const char** description, ExceptionData* exception, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return CODE_FAIL;
    };
    X509ExceptionClear(exception, dynMsg);

    if (!X509CheckNotNull(exception, (void*)(keyBody), "keyBody", dynMsg) ||
        !X509CheckNotNull(exception, (void*)(resultBody), "resultBody", dynMsg) ||
        !X509CheckNotNull(exception, (void*)(resultSize), "resultSize", dynMsg) ||
        !X509CheckNotNull(exception, (void*)(params), "params", dynMsg)) {
        return CODE_FAIL;
    }
    *resultBody = NULL;
    *resultSize = 0;
    if (description != NULL) {
        *description = NULL;
    }
    EVP_PKEY* key = LoadEncryptedKey(keyBody, length, params, exception, dynMsg);
    if (!key) {
        return CODE_FAIL;
    }
    BIO* buffer = DYN_BIO_new_mem(dynMsg);
    if (buffer == NULL) {
        DYN_EVP_PKEY_free(key, dynMsg);
        return CODE_FAIL;
    }
    int result = DYN_i2d_PKCS8PrivateKey_bio(buffer, key, NULL, NULL, 0, DYN_CodePemPasswordCb, NULL, dynMsg);
    if (result == 0) {
        X509HandleError(exception, "Failed to encode decrypted key to PKCS8", dynMsg);
    } else if (result == 1) {
        BUF_MEM* ptr = NULL;
        long rc = DYN_BIO_get_mem_ptr(buffer, &ptr, dynMsg);
        if (rc == 1 && ptr != NULL && ptr->data != NULL && ptr->length > 0) {
            *resultBody = DYN_OPENSSL_memdup(ptr->data, ptr->length, dynMsg);
            *resultSize = ptr->length;
        }
        if (description != NULL) {
            *description = X509DescribePrivateKey(key, dynMsg);
        }
    }
    DYN_BIO_vfree(buffer, dynMsg);
    DYN_EVP_PKEY_free(key, dynMsg);
    if (result == 1) {
        return CODE_OK;
    } else {
        return CODE_FAIL;
    }
}

extern int DYN_CODEX509CheckPrivateKey(SSL_CTX* ctx, const char* file, DynMsg* dynMsg)
{
    if (ctx == NULL) {
        return 0;
    }
    return DYN_SSL_CTX_check_private_key(ctx, dynMsg);
}

/**
 * Try to briefly load the provided key, to see if it's a vaild public key.
 */
extern int CODEX509DescribePublicKey(const void* keyBody, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return 0;
    }
    X509ExceptionClear(exception, dynMsg);

    if (!X509CheckNotNull(exception, (void*)(keyBody), "keyBody", dynMsg)) {
        return 0;
    }
    if (!X509CheckOrFillException(exception, length > 0, "length > 0", dynMsg)) {
        return 0;
    }
    const unsigned char* dataptr = (const unsigned char*)keyBody;
    X509_PUBKEY* xpKey = DYN_d2i_X509_PUBKEY(NULL, &dataptr, (long)length, dynMsg);
    if (xpKey == NULL) {
        X509HandleError(
            exception, "Failed to load public key, it's either corrupted, or the format is unsupported", dynMsg);
        return 0;
    }
    DYN_X509_PUBKEY_free(xpKey, dynMsg);
    return 1;
}

/**
 * Try to briefly load the provided key, to see if it's a vaild DH Parameters.
 */
extern int CODEX509DescribeDHParameters(const void* keyBody, size_t length, ExceptionData* exception, DynMsg* dynMsg)
{
    if (exception == NULL) {
        return 0;
    };
    X509ExceptionClear(exception, dynMsg);
    if (!X509CheckNotNull(exception, (void*)(keyBody), "keyBody", dynMsg)) {
        return 0;
    }
    if (!X509CheckOrFillException(exception, length > 0, "length > 0", dynMsg)) {
        return 0;
    }
    const unsigned char* dataptr = (const unsigned char*)keyBody;
    EVP_PKEY* xpKey = DYN_d2i_KeyParams(EVP_PKEY_DH, NULL, &dataptr, (long)length, dynMsg);
    if (xpKey == NULL) {
        X509HandleError(
            exception, "Failed to load DH Parameters, it's either corrupted, or the format is unsupported", dynMsg);
        return 0;
    }
    DYN_EVP_PKEY_free(xpKey, dynMsg);
    return 1;
}
