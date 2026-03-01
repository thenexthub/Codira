/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <openssl/asn1.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/safestack.h>
#include <openssl/x509v3.h>
#include "api.h"
#include "securec.h"

/**
 * The iteraction result with CODE.
 */

struct RawX509Cert {
    const uint8_t* content;
    size_t size;
};

struct RawX509CertArray {
    struct RawX509Cert* buffer;
    size_t size;
};

struct X509CertInfo {
    const char* serialNumber;
    const char* notBefore;
    const char* notAfter;
    const char* altNames;
    const char* keyUsage;
    const char* extKeyUsage;
};

// Clone the parse result from X509 cert.
static void* DataClone(const uint8_t* source, size_t size)
{
    void* temp = (void*)malloc((uint32_t)sizeof(char) * size);
    if (temp == NULL) {
        return NULL;
    }
    if (memcpy_s(temp, size, source, size) != EOK) {
        free(temp);
        return NULL;
    }
    return temp;
}

// only support version v3
#define CODE_VERSION_TYPE 2

static int AddExt(X509* cert, int type, const char* value, DynMsg* dynMsg)
{
    if (strlen(value) == 0) {
        return 1;
    }
    X509V3_CTX ctx;
    ctx.db = NULL;
    if (dynMsg && dynMsg->found == false) {
        return 0;
    }
    X509_EXTENSION* ex = DYN_X509V3_EXT_conf_nid(NULL, &ctx, type, value, dynMsg);
    int ret = DYN_X509_add_ext(cert, ex, -1, dynMsg);
    if (ret <= 0) {
        DYN_X509_EXTENSION_free(ex, dynMsg);
        return ret;
    }
    DYN_X509_EXTENSION_free(ex, dynMsg);
    return 1;
}

extern void* DYN_CODECreateCert(void* pubKey, void* priKey, void* issuer, void* subject, void* digest,
    struct X509CertInfo* certInfo, DynMsg* dynMsg)
{
    X509* cert = DYN_X509_new(dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return NULL;
    }
    if (DYN_X509_set_version(cert, CODE_VERSION_TYPE, dynMsg) <= 0) {
        return NULL;
    }

    // set serial number
    ASN1_INTEGER* serial = DYN_ASN1_INTEGER_new(dynMsg);
    BIGNUM* bn = DYN_BN_new(dynMsg);
    if (DYN_BN_hex2bn(&bn, certInfo->serialNumber, dynMsg) <= 0) {
        DYN_BN_free(bn, dynMsg);
        DYN_ASN1_INTEGER_free(serial, dynMsg);
        return NULL;
    }
    serial = DYN_BN_to_ASN1_INTEGER(bn, serial, dynMsg);
    if (DYN_X509_set_serialNumber(cert, serial, dynMsg) <= 0) {
        DYN_BN_free(bn, dynMsg);
        DYN_ASN1_INTEGER_free(serial, dynMsg);
        return NULL;
    }
    DYN_BN_free(bn, dynMsg);
    DYN_ASN1_INTEGER_free(serial, dynMsg);

    // set vaildity time
    ASN1_TIME* tm = DYN_ASN1_TIME_new(dynMsg);
    if (DYN_ASN1_TIME_set_string(tm, certInfo->notBefore, dynMsg) <= 0) {
        DYN_ASN1_TIME_free(tm, dynMsg);
        return NULL;
    }
    if (DYN_X509_set1_notBefore(cert, tm, dynMsg) <= 0) {
        DYN_ASN1_TIME_free(tm, dynMsg);
        return NULL;
    }
    if (DYN_ASN1_TIME_set_string(tm, certInfo->notAfter, dynMsg) <= 0) {
        DYN_ASN1_TIME_free(tm, dynMsg);
        return NULL;
    }
    if (DYN_X509_set1_notAfter(cert, tm, dynMsg) <= 0) {
        DYN_ASN1_TIME_free(tm, dynMsg);
        return NULL;
    }
    DYN_ASN1_TIME_free(tm, dynMsg);

    // set x509_name
    if (subject != NULL && DYN_X509_set_subject_name(cert, subject, dynMsg) <= 0) {
        return NULL;
    }
    if (DYN_X509_set_issuer_name(cert, issuer, dynMsg) <= 0) {
        return NULL;
    }

    // set public key
    if (DYN_X509_set_pubkey(cert, pubKey, dynMsg) <= 0) {
        return NULL;
    }

    // set extensions// set extensions
    if (AddExt(cert, NID_subject_alt_name, certInfo->altNames, dynMsg) <= 0) {
        return NULL;
    }
    if (AddExt(cert, NID_key_usage, certInfo->keyUsage, dynMsg) <= 0) {
        return NULL;
    }
    if (AddExt(cert, NID_ext_key_usage, certInfo->extKeyUsage, dynMsg) <= 0) {
        return NULL;
    }

    // sign certificate with digest
    if (DYN_X509_sign(cert, priKey, digest, dynMsg) <= 0) {
        return NULL;
    }

    return (void*)cert;
}

extern void DYN_CODECertFree(void* cert, DynMsg* dynMsg)
{
    DYN_X509_free((void*)cert, dynMsg);
}

extern void DYN_CODEKeyFree(void* key, DynMsg* dynMsg)
{
    DYN_EVP_PKEY_free((void*)key, dynMsg);
}

extern void* DYN_CODEGetPubKeyPtr(const unsigned char** data, long length, DynMsg* dynMsg)
{
    return (void*)DYN_d2i_PUBKEY(NULL, data, length, dynMsg);
}

extern void* DYN_CODEGetPriKeyPtr(const unsigned char** data, long length, DynMsg* dynMsg)
{
    return (void*)DYN_d2i_AutoPrivateKey(NULL, data, length, dynMsg);
}

extern int DYN_CODEGetCertLen(void* cert, unsigned char** out, DynMsg* dynMsg)
{
    return DYN_i2d_X509(cert, out, dynMsg);
}

static size_t GetSizeInGeneralNamesByType(GENERAL_NAMES* subjectAltNames, int cnt, int type, DynMsg* dynMsg)
{
    size_t size = 0;
    for (int i = 0; i < cnt; i++) {
        GENERAL_NAME* name = DYN_OPENSSL_sk_value((void*)subjectAltNames, i, dynMsg);

        if (name->type == type) {
            size++;
        }
    }
    return size;
}

static void StoreResultByType(
    GENERAL_NAMES* subjectAltNames, int type, struct StringArrayResult* result, DynMsg* dynMsg)
{
    if (subjectAltNames == NULL) {
        return;
    }
    int cnt = DYN_OPENSSL_sk_num((void*)subjectAltNames, dynMsg);
    size_t validCnt = GetSizeInGeneralNamesByType(subjectAltNames, cnt, type, dynMsg);
    if (validCnt == 0) {
        return;
    }
    result->buffer = (char**)malloc(sizeof(char*) * validCnt);
    if (result->buffer == NULL) {
        return;
    }
    result->size = validCnt;
    int index = 0;
    // parse string names
    for (int i = 0; i < cnt; i++) {
        GENERAL_NAME* name = DYN_OPENSSL_sk_value((void*)subjectAltNames, i, dynMsg);
        if (name->type == type) {
            size_t len = (size_t)(unsigned int)(DYN_ASN1_STRING_length(name->d.dNSName, dynMsg) + 1);
            result->buffer[index++] = (char*)DataClone(DYN_ASN1_STRING_get0_data(name->d.dNSName, dynMsg), len);
        }
    }
}

static void StoreIPResult(GENERAL_NAMES* subjectAltNames, struct ByteArrayResult* result, DynMsg* dynMsg)
{
    if (subjectAltNames == NULL) {
        return;
    }
    int cnt = DYN_OPENSSL_sk_num((void*)subjectAltNames, dynMsg);
    size_t validCnt = GetSizeInGeneralNamesByType(subjectAltNames, cnt, GEN_IPADD, dynMsg);
    if (validCnt == 0) {
        return;
    }
    result->buffer = (struct ByteResult*)malloc(sizeof(struct ByteResult) * validCnt);
    if (result->buffer == NULL) {
        return;
    }
    result->size = validCnt;
    int index = 0;
    // parse IP names
    for (int i = 0; i < cnt; i++) {
        GENERAL_NAME* name = DYN_OPENSSL_sk_value((void*)subjectAltNames, i, dynMsg);
        if (name->type == GEN_IPADD) {
            size_t len = (size_t)(unsigned int)DYN_ASN1_STRING_length(name->d.iPAddress, dynMsg);
            result->buffer[index].size = len;
            result->buffer[index++].buffer =
                (uint8_t*)DataClone(DYN_ASN1_STRING_get0_data(name->d.iPAddress, dynMsg), len);
        }
    }
}

static void GetExtensionResultFromName(GENERAL_NAMES* subjectAltNames, int type, void* result, DynMsg* dynMsg)
{
    switch (type) {
        case GEN_EMAIL:
        case GEN_DNS:
            StoreResultByType(subjectAltNames, type, (struct StringArrayResult*)result, dynMsg);
            break;
        case GEN_IPADD:
            StoreIPResult(subjectAltNames, (struct ByteArrayResult*)result, dynMsg);
            break;
        default:
            break;
    }
}

static void GetX509ExtensionByNameType(
    const unsigned char* derBlob, size_t length, void* result, int type, DynMsg* dynMsg)
{
    if (memset_s(result, X509_RESULT_SIZE, 0, X509_RESULT_SIZE) != 0) {
        return;
    }

    X509* cert = DYN_d2i_X509(NULL, &derBlob, (long)length, dynMsg);
    if (cert == NULL) {
        return;
    }
    GENERAL_NAMES* subjectAltNames =
        (GENERAL_NAMES*)DYN_X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return;
    }
    GetExtensionResultFromName(subjectAltNames, type, result, dynMsg);
    if (subjectAltNames != NULL) {
        DynPopFree(subjectAltNames, "GENERAL_NAME_free", dynMsg);
    }
    DYN_X509_free(cert, dynMsg);
}

// Get DNS names in X509 extension.
extern void DYN_CODEGetX509DnsNames(
    const unsigned char* derBlob, size_t length, struct StringArrayResult* result, DynMsg* dynMsg)
{
    GetX509ExtensionByNameType(derBlob, length, result, GEN_DNS, dynMsg);
}

// Get email addresses in X509 extension.
extern void DYN_CODEGetX509EmailAddresses(
    const unsigned char* derBlob, size_t length, struct StringArrayResult* result, DynMsg* dynMsg)
{
    GetX509ExtensionByNameType(derBlob, length, result, GEN_EMAIL, dynMsg);
}

// Get IP addresses in X509 extension.
extern void DYN_CODEGetX509IpAddresses(
    const unsigned char* derBlob, size_t length, struct ByteArrayResult* result, DynMsg* dynMsg)
{
    GetX509ExtensionByNameType(derBlob, length, result, GEN_IPADD, dynMsg);
}

// Get key usage in X509 extension.
extern uint16_t DYN_CODEGetX509KeyUsage(const unsigned char* derBlob, size_t length, DynMsg* dynMsg)
{
    uint16_t keyUsage = 0;
    X509* cert = DYN_d2i_X509(NULL, &derBlob, (long)length, dynMsg);
    if (cert == NULL) {
        return keyUsage;
    }
    ASN1_BIT_STRING* usage = NULL;
    if ((usage = DYN_X509_get_ext_d2i(cert, NID_key_usage, NULL, NULL, dynMsg)) != NULL) {
        if (usage->length > 0) {
            keyUsage |= usage->data[0];
        }
        if (usage->length > 1) {
            const uint16_t bitS8 = 8;
            keyUsage |= (uint16_t)usage->data[1] << bitS8;
        }
        DYN_ASN1_BIT_STRING_free(usage, dynMsg);
    }
    DYN_X509_free(cert, dynMsg);
    // Process the KU_DECIPHER_ONLY  -> 0x01xx for cangjie.
    if ((keyUsage & (uint16_t)(KU_DECIPHER_ONLY)) != 0) {
        const uint16_t setMask = 0x0100;
        const uint16_t clearMask = 0x00FF;
        keyUsage = (keyUsage & clearMask) | setMask;
    }
    return keyUsage;
}

// The Value of Extended Key Usage for Codira
#define CODE_ANY_KEY 0
#define CODE_SERVER_AUTH 1
#define CODE_CLIENT_AUTH 2
#define CODE_EMAIL_PROTECTION 3
#define CODE_CODE_SIGNING 4
#define CODE_OCSP_SIGNING 5
#define CODE_TIME_STAMPING 6

static void StoreExtKeyUsageResult(EXTENDED_KEY_USAGE* extusage, struct UInt16Result* result, DynMsg* dynMsg)
{
    if (extusage == NULL) {
        return;
    }
    int cnt = DYN_OPENSSL_sk_num((void*)extusage, dynMsg);
    if (cnt <= 0) {
        return;
    }
    size_t size = (size_t)(unsigned int)cnt;
    result->buffer = (uint16_t*)malloc(sizeof(uint16_t) * size);
    if (result->buffer == NULL) {
        return;
    }
    result->size = size;
    for (int i = 0; i < cnt; i++) {
        switch (DYN_OBJ_obj2nid(DYN_OPENSSL_sk_value((void*)extusage, i, dynMsg), dynMsg)) {
            case NID_server_auth:
                result->buffer[i] = CODE_SERVER_AUTH;
                break;
            case NID_client_auth:
                result->buffer[i] = CODE_CLIENT_AUTH;
                break;
            case NID_email_protect:
                result->buffer[i] = CODE_EMAIL_PROTECTION;
                break;
            case NID_code_sign:
                result->buffer[i] = CODE_CODE_SIGNING;
                break;
            case NID_OCSP_sign:
                result->buffer[i] = CODE_OCSP_SIGNING;
                break;
            case NID_time_stamp:
                result->buffer[i] = CODE_TIME_STAMPING;
                break;
            default:
                result->buffer[i] = CODE_ANY_KEY;
        }
    }
}

// Get ext key usage in X509 extension.
extern void DYN_CODEGetX509ExtKeyUsage(
    const unsigned char* derBlob, size_t length, struct UInt16Result* result, DynMsg* dynMsg)
{
    result->size = 0;
    result->buffer = NULL;
    X509* cert = DYN_d2i_X509(NULL, &derBlob, (long)length, dynMsg);
    if (cert == NULL) {
        return;
    }
    EXTENDED_KEY_USAGE* extusage = NULL;
    extusage = DYN_X509_get_ext_d2i(cert, NID_ext_key_usage, NULL, NULL, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return;
    }
    StoreExtKeyUsageResult(extusage, result, dynMsg);
    if (extusage != NULL) {
        DYN_OPENSSL_sk_free((void*)extusage, dynMsg);
    }
    DYN_X509_free(cert, dynMsg);
}

static int X509StoreRawCert(X509_STORE* chains, const unsigned char** raw, size_t size, DynMsg* dynMsg)
{
    X509* root = DYN_d2i_X509(NULL, raw, (long)size, dynMsg);
    if (root == NULL) {
        return -1;
    }
    // Copy root into chains
    int status = DYN_X509_STORE_add_cert(chains, root, dynMsg);
    DYN_X509_free(root, dynMsg);
    return status;
}

// Certificate Der Blob
extern int DYN_CODEVerifyX509Cert(struct RawX509Cert* rawCert, struct RawX509CertArray* rawRoots,
    struct RawX509CertArray* rawIntermediates, DynMsg* dynMsg)
{
    // The rawCert, rawRoots and rawIntermediates is not null guard by cangjie side.
    X509* cert = DYN_d2i_X509(NULL, &rawCert->content, (long)rawCert->size, dynMsg);
    if (cert == NULL) {
        return -1;
    }
    int status = 0;
    // Create certificate store and add CA and intermediate certificates
    X509_STORE* chains = DYN_X509_STORE_new(dynMsg);
    if (chains == NULL) {
        status = -1;
    }
    if (status != -1) {
        // Add root certs
        for (size_t i = 0; i < rawRoots->size; i++) {
            if (X509StoreRawCert(chains, &rawRoots->buffer[i].content, rawRoots->buffer[i].size, dynMsg) != 1) {
                status = -1;
                break;
            }
        }
    }
    if (status != -1) {
        // Add intermediate certs
        for (size_t i = 0; i < rawIntermediates->size; i++) {
            if (X509StoreRawCert(
                    chains, &rawIntermediates->buffer[i].content, rawIntermediates->buffer[i].size, dynMsg) != 1) {
                status = -1;
                break;
            }
        }
    }
    X509_STORE_CTX* ctx = NULL;
    if (status != -1) {
        // Create verification context and set certificate store
        ctx = DYN_X509_STORE_CTX_new(dynMsg);
        if (DYN_X509_STORE_CTX_init(ctx, chains, cert, NULL, dynMsg) != 1) {
            status = -1;
        }
    }
    if (status != -1) {
        // Verify certificate
        status = DYN_X509_verify_cert(ctx, dynMsg);
    }
    // Clean up
    if (ctx != NULL) {
        DYN_X509_STORE_CTX_free(ctx, dynMsg);
    }
    if (chains != NULL) {
        DYN_X509_STORE_free(chains, dynMsg);
    }
    DYN_X509_free(cert, dynMsg);
    return status;
}

static X509_EXTENSION* GetSubjectExtention(STACK_OF(X509_EXTENSION) * extlist, DynMsg* dynMsg)
{
    X509_EXTENSION* extension;
    ASN1_OBJECT* object;
    int num = DYN_OPENSSL_sk_num((void*)extlist, dynMsg), nid;
    for (int i = 0; i < num; i++) {
        extension = DYN_OPENSSL_sk_value((void*)extlist, i, dynMsg);
        object = DYN_X509_EXTENSION_get_object(extension, dynMsg);
        nid = DYN_OBJ_obj2nid(object, dynMsg);
        if (nid == NID_subject_alt_name) {
            return extension;
        }
    }
    return NULL;
}

static void GetX509ReqExtensionByNameType(
    const unsigned char* derBlob, size_t length, void* result, int type, DynMsg* dynMsg)
{
    if (memset_s(result, X509_RESULT_SIZE, 0, X509_RESULT_SIZE) != 0) {
        return;
    }
    X509_REQ* req = DYN_d2i_X509_REQ(NULL, &derBlob, (long)length, dynMsg);
    if (req == NULL) {
        return;
    }
    STACK_OF(X509_EXTENSION)* extlist = DYN_X509_REQ_get_extensions(req, dynMsg);
    if (extlist == NULL) {
        DYN_X509_REQ_free(req, dynMsg);
        return;
    }
    X509_EXTENSION* extension = GetSubjectExtention(extlist, dynMsg);
    if (extension != NULL) {
        GENERAL_NAMES* subjectAltNames = DYN_X509V3_EXT_d2i(extension, dynMsg);
        if (subjectAltNames != NULL) {
            GetExtensionResultFromName(subjectAltNames, type, result, dynMsg);
            DynPopFree(subjectAltNames, "GENERAL_NAME_free", dynMsg);
        }
    }
    DynPopFree(extlist, "X509_EXTENSION_free", dynMsg);
    DYN_X509_REQ_free(req, dynMsg);
}

// Get DNS names in X509_REQ extension.
extern void DYN_CODEGetX509CsrDnsNames(
    const unsigned char* derBlob, size_t length, struct StringArrayResult* result, DynMsg* dynMsg)
{
    GetX509ReqExtensionByNameType(derBlob, length, result, GEN_DNS, dynMsg);
}

// Get email addresses in X509_REQ extension.
extern void DYN_CODEGetX509CsrEmailAddresses(
    const unsigned char* derBlob, size_t length, struct StringArrayResult* result, DynMsg* dynMsg)
{
    GetX509ReqExtensionByNameType(derBlob, length, result, GEN_EMAIL, dynMsg);
}

// Get IP addresses in X509_REQ extension.
extern void DYN_CODEGetX509CsrIpAddresses(
    const unsigned char* derBlob, size_t length, struct ByteArrayResult* result, DynMsg* dynMsg)
{
    GetX509ReqExtensionByNameType(derBlob, length, result, GEN_IPADD, dynMsg);
}

extern void* DYN_CODENameStackNew(DynMsg* dynMsg)
{
    STACK_OF(GENERAL_NAME)* names = NULL;
    names = (STACK_OF(GENERAL_NAME)*)DYN_OPENSSL_sk_new_null(dynMsg);
    return (void*)names;
}

extern void DYN_CODENameStackFree(void* names, DynMsg* dynMsg)
{
    DynPopFree((STACK_OF(GENERAL_NAME)*)names, "GENERAL_NAME_free", dynMsg);
}

extern int DYN_CODEAddName(void* names, int nid, char* value, DynMsg* dynMsg)
{
    GENERAL_NAME* n = DYN_a2i_GENERAL_NAME(NULL, NULL, NULL, nid, value, 0, dynMsg);
    return DYN_OPENSSL_sk_push((STACK_OF(GENERAL_NAME)*)names, n, dynMsg);
}

extern int DYN_CODEReqAddExtension(void* req, void* names, DynMsg* dynMsg)
{
    X509_EXTENSION* ex = DYN_X509V3_EXT_i2d(NID_subject_alt_name, 0, (STACK_OF(GENERAL_NAME)*)names, dynMsg);
    STACK_OF(X509_EXTENSION)* exts = (STACK_OF(X509_EXTENSION)*)DYN_OPENSSL_sk_new_null(dynMsg);
    int status = DYN_OPENSSL_sk_push(exts, ex, dynMsg);
    if (status <= 0) {
        return status;
    }
    status = DYN_X509_REQ_add_extensions(req, exts, dynMsg);
    DynPopFree(exts, "X509_EXTENSION_free", dynMsg);
    return status;
}

extern void* DYN_CODENameNew(DynMsg* dynMsg)
{
    return (void*)DYN_X509_NAME_new(dynMsg);
}

extern void DYN_CODENameFree(void* name, DynMsg* dynMsg)
{
    DYN_X509_NAME_free((X509_NAME*)name, dynMsg);
}

extern int DYN_CODEX509NameAddEntry(void* name, char* field, int nameType, char* str, DynMsg* dynMsg)
{
    return DYN_X509_NAME_add_entry_by_txt((X509_NAME*)name, field, nameType, (unsigned char*)str, -1, -1, 0, dynMsg);
}

extern void* DYN_CODEX509ReqNew(DynMsg* dynMsg)
{
    return (void*)DYN_X509_REQ_new(dynMsg);
}

extern void DYN_CODEX509ReqFree(void* req, DynMsg* dynMsg)
{
    DYN_X509_REQ_free(req, dynMsg);
}

extern int DYN_CODEGetX509ReqDer(void* req, char** out, DynMsg* dynMsg)
{
    return DYN_i2d_X509_REQ((X509_REQ*)req, (unsigned char**)out, dynMsg);
}

extern int DYN_CODEX509ReqSetSubject(void* req, void* name, DynMsg* dynMsg)
{
    return DYN_X509_REQ_set_subject_name((X509_REQ*)req, (X509_NAME*)name, dynMsg);
}

extern int DYN_CODEX509ReqSetPubkey(void* req, void* pkey, DynMsg* dynMsg)
{
    return DYN_X509_REQ_set_pubkey((X509_REQ*)req, (EVP_PKEY*)pkey, dynMsg);
}

extern int DYN_CODEX509ReqSign(void* req, void* pkey, void* md, DynMsg* dynMsg)
{
    return DYN_X509_REQ_sign((X509_REQ*)req, (EVP_PKEY*)pkey, (const EVP_MD*)md, dynMsg);
}

extern void* DYN_CODEGetNamePtr(const unsigned char** data, long length, DynMsg* dynMsg)
{
    return (void*)DYN_d2i_X509_NAME(NULL, data, length, dynMsg);
}

extern int DYN_CODEGetNameDer(void* name, unsigned char** out, DynMsg* dynMsg)
{
    return DYN_i2d_X509_NAME(name, out, dynMsg);
}
