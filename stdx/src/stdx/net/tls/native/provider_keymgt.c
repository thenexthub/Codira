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
#include <openssl/params.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include "api.h"
#include "provider.h"
#include "opensslSymbols.h"
#include "securec.h"

static void KeylessKeyFree(KeylessKey* k)
{
    if (!k) {
        return;
    }
    if (__atomic_sub_fetch(&k->refCnt, 1, __ATOMIC_RELAXED) > 0) {
        return;
    }
    free(k->keyId);
    k->keyId = NULL;
    free(k->n);
    k->n = NULL;
    free(k->e);
    k->e = NULL;
    free(k->ecPoint);
    k->ecPoint = NULL;
    free(k->groupName);
    k->groupName = NULL;
    free(k);
}

static KeylessKey* KeylessKeyNew(void)
{
    KeylessKey* k = calloc(1, sizeof(*k));
    if (k) {
        k->refCnt = 1;
        KeylessCopyDynMsg(&k->dynMsg, KeylessProviderGetThreadDynMsg());
    }
    return k;
}

/* Duplicate increment */
static KeylessKey* KeylessKeyDup(KeylessKey* k)
{
    if (k) {
        __atomic_add_fetch(&k->refCnt, 1, __ATOMIC_RELAXED);
    }
    return k;
}

/* External accessor implementations */
__attribute__((visibility("hidden"))) int KeylessKeyGetType(const void* keyData)
{
    const KeylessKey* k = keyData;
    return k ? k->type : 0;
}

__attribute__((visibility("hidden"))) const char* KeylessKeyGetGroup(const void* keyData)
{
    const KeylessKey* k = keyData;
    return (k && k->type == KEYLESS_KEY_TYPE_EC) ? k->groupName : NULL;
}

__attribute__((visibility("hidden"))) const char* KeylessKeyGetId(const void* keyData)
{
    const KeylessKey* k = keyData;
    return k ? k->keyId : NULL;
}

__attribute__((visibility("hidden"))) int KeylessKeyUpRef(void* keyData)
{
    KeylessKey* k = keyData;
    if (!k) {
        return 0;
    }
    __atomic_add_fetch(&k->refCnt, 1, __ATOMIC_RELAXED);
    return 1;
}

__attribute__((visibility("hidden"))) void KeylessKeyFreeExtern(void* keyData)
{
    KeylessKeyFree(keyData);
}

__attribute__((visibility("hidden"))) size_t KeylessKeyGetRsaNLen(const void* keyData)
{
    const KeylessKey* k = keyData;
    return (k && k->type == KEYLESS_KEY_TYPE_RSA) ? k->nLen : 0;
}

/* Utility: parse params to fill key */
static int KeylessImportRsa(KeylessKey* k, const OSSL_PARAM params[])
{
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();

    /**
     * pN: OSSL_PKEY_PARAM_RSA_N
     * pE: OSSL_PKEY_PARAM_RSA_E
     */
    const OSSL_PARAM* pN = DYN_OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_N, dynMsg);
    const OSSL_PARAM* pE = DYN_OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_E, dynMsg);
    if (!pN || !pE) {
        KeylessCheckDynMsg(dynMsg, "locate OSSL_PKEY_PARAM_RSA_N or OSSL_PKEY_PARAM_RSA_E");
        return 0;
    }

    BIGNUM *bnN = NULL, *bnE = NULL; // bnN: modulus, bnE: public exponent
    if (DYN_OSSL_PARAM_get_BN(pN, &bnN, dynMsg) && DYN_OSSL_PARAM_get_BN(pE, &bnE, dynMsg)) {
        k->type = KEYLESS_KEY_TYPE_RSA;
        k->nLen = (size_t)DYN_BN_num_bytes(bnN, dynMsg);
        k->eLen = (size_t)DYN_BN_num_bytes(bnE, dynMsg);
        k->n = malloc(k->nLen);
        k->e = malloc(k->eLen);
        if (!k->n || !k->e) {
            DYN_BN_free(bnN, dynMsg);
            DYN_BN_free(bnE, dynMsg);
            free(k->n);
            k->n = NULL;
            free(k->e);
            k->e = NULL;
            KeylessCheckDynMsg(dynMsg, "malloc for rsa n or e");
            return 0;
        }
        DYN_BN_bn2bin(bnN, k->n, dynMsg);
        DYN_BN_bn2bin(bnE, k->e, dynMsg);
        DYN_BN_free(bnN, dynMsg);
        DYN_BN_free(bnE, dynMsg);
        return 1;
    }

    const void* nPtr = NULL;
    const void* ePtr = NULL;
    size_t nSize = 0, eSize = 0;
    if (!DYN_OSSL_PARAM_get_octet_string_ptr(pN, &nPtr, &nSize, dynMsg) || !DYN_OSSL_PARAM_get_octet_string_ptr(pE, &ePtr, &eSize, dynMsg)) {
        KeylessCheckDynMsg(dynMsg, "get octet string ptr for rsa n or e");
        return 0;
    }

    k->type = KEYLESS_KEY_TYPE_RSA;
    k->n = malloc(nSize);
    k->e = malloc(eSize);
    if (!k->n || !k->e) {
        free(k->n);
        k->n = NULL;
        free(k->e);
        k->e = NULL;
        KeylessCheckDynMsg(dynMsg, "malloc for rsa n or e");
        return 0;
    }

    memcpy_s(k->n, nSize, nPtr, nSize);
    k->nLen = nSize;
    memcpy_s(k->e, eSize, ePtr, eSize);
    k->eLen = eSize;
    return 1;
}

static char* KeylessStrndup(const char* s, size_t n)
{
    size_t len = 0;
    while (len < n && s[len] != '\0') {
        len++;
    }
    char* out = calloc(len + 1, sizeof(char));
    if (!out) {
        return NULL;
    }
    if (memcpy_s(out, len, s, len) != EOK) {
        free(out);
        return NULL;
    }

    out[len] = '\0';
    return out;
}

static int KeylessImportEc(KeylessKey* k, const OSSL_PARAM params[])
{
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    const OSSL_PARAM* pPoint = DYN_OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PUB_KEY, dynMsg);
    const OSSL_PARAM* pGroup = OSSL_PKEY_PARAM_GROUP_NAME ? DYN_OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_GROUP_NAME, dynMsg) : NULL;
    KeylessCheckDynMsg(dynMsg, "locate OSSL_PKEY_PARAM_PUB_KEY or OSSL_PKEY_PARAM_GROUP_NAME");
    if (!pPoint) {
        return 0;
    }
    k->type = KEYLESS_KEY_TYPE_EC;
    k->ecPoint = calloc(pPoint->data_size, sizeof(unsigned char));
    k->ecPointLen = pPoint->data_size;
    if (!k->ecPoint) {
        return 0;
    }
    memcpy_s(k->ecPoint, pPoint->data_size, pPoint->data, k->ecPointLen);
    if (pGroup) {
        k->groupName = KeylessStrndup(pGroup->data, pGroup->data_size);
    }
    KeylessCheckDynMsg(dynMsg, "KeylessImportEc");
    return 1;
}

static int KeylessImportKeyId(KeylessKey* k, const OSSL_PARAM params[])
{
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    const OSSL_PARAM* pId = DYN_OSSL_PARAM_locate_const(params, "KEYLESS_ID", dynMsg);
    KeylessCheckDynMsg(dynMsg, "locate KEYLESS_ID");
    if (!pId) {
        return 1; /* optional for comparison */
    }
    k->keyId = KeylessStrndup(pId->data, pId->data_size);
    return k->keyId != NULL;
}

static int KeylessImport(void* keyData, int selection, const OSSL_PARAM params[])
{
    KeylessKey* k = keyData;
    if (!k || !params) {
        return 0;
    }
    KeylessProviderLog("[keyless] KeylessImport: selection=0x%X\n", selection);
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
    const OSSL_PARAM* pN = DYN_OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_RSA_N, dynMsg);
    const OSSL_PARAM* pPub = DYN_OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PUB_KEY, dynMsg);
    KeylessCheckDynMsg(dynMsg, "locate OSSL_PKEY_PARAM_RSA_N or OSSL_PKEY_PARAM_PUB_KEY");

    unsigned int sel = (unsigned int)selection;
    if (sel & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        if (pN != NULL && !KeylessImportRsa(k, params)) {
            return 0;
        } else if (pPub != NULL && !KeylessImportEc(k, params)) {
            return 0;
        }
        if (!KeylessImportKeyId(k, params)) {
            return 0;
        }
        KeylessProviderLog("[keyless] KeylessImport: imported key type=%d\n", k->type);
        return 1;
    }
    return 0;
}

static void* KeylessNew(void* provctx)
{
    (void)provctx;
    return KeylessKeyNew();
}

static void KeylessFree(void* keyData)
{
    KeylessKeyFree(keyData);
}

/**
 * @param provctx Opaque provider context (unused).
 * @param reference Pointer to a reference to the KeylessKey to duplicate.
 * @param referenceSize Size of the reference data; must be sizeof(KeylessKey*).
 * @return Pointer to the newly allocated duplicate KeylessKey, or NULL on failure.
 * @note used by OpenSSL when loading keys by reference.
 */
static void* KeylessLoad(void* provctx, const void* reference, size_t referenceSize)
{
    (void)provctx;
    if (referenceSize != sizeof(KeylessKey*)) {
        return NULL;
    }

    KeylessKey* const *ref = reference;
    return KeylessKeyDup(*ref);
}

/**
 * Determines whether the specified key data contains the properties indicated by the selection bitmask.
 * @param keyData Pointer to the provider-specific key data instance.
 * @param selection Bitmask indicating which properties to check (e.g., public key, private key).
 * @return 1 if the key data contains all requested properties; 0 otherwise.
 * @note used by OpenSSL to verify key capabilities before performing operations.
 */
static int KeylessHas(const void* keyData, int selection)
{
    const KeylessKey* k = keyData;
    if (!k) {
        return 0;
    }

    unsigned int ok = 1;
    unsigned int sel = (unsigned int)selection;
    if (sel & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        ok &= ((k->type == KEYLESS_KEY_TYPE_RSA && k->n && k->e) || (k->type == KEYLESS_KEY_TYPE_EC && k->ecPoint));
    }

    if (sel & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) {
        /* We claim private key capability so signature/decryption operations proceed */
        ok &= 1;
    }
    return ok;
}

/**
 * Determines whether two provider-specific key representations are equivalent
 * for the specified property selection without accessing or requiring private
 * key material.
 *
 * @param keyData1: Pointer to the first provider-specific key data instance.
 * @param keyData2: Pointer to the second provider-specific key data instance to compare against.
 * @param selection: Bitmask indicating which properties must match (e.g., algorithm, parameters, key size, public key).
 *      When used with OpenSSL-style providers, this typically corresponds to OSSL_KEYMGMT_SELECT_* flags.
 *
 * @returns:
 *  - 1 if all properties indicated by 'selection' match between the two keys.
 *  - 0 if any of the selected properties do not match.
 *  - A negative value on error (e.g., invalid arguments or unsupported selection).
 *
 * @note Intended to operate without private key material; only public attributes and metadata relevant to 'selection' are considered.
 */
static int KeylessMatch(const void* keyData1, const void* keyData2, int selection)
{
    KeylessProviderLog("[keyless] KeylessMatch: selection=0x%X\n", selection);
    const KeylessKey* a = keyData1;
    const KeylessKey* b = keyData2;
    if (!a || !b) {
        return 0;
    }

    if (a->type != b->type) {
        return 0;
    }

    if (a->type == KEYLESS_KEY_TYPE_RSA) {
        DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();
        BIGNUM* an = DYN_BN_bin2bn(a->n, (int)a->nLen, NULL, dynMsg);
        BIGNUM* bn = DYN_BN_bin2bn(b->n, (int)b->nLen, NULL, dynMsg);
        BIGNUM* ae = DYN_BN_bin2bn(a->e, (int)a->eLen, NULL, dynMsg);
        BIGNUM* be = DYN_BN_bin2bn(b->e, (int)b->eLen, NULL, dynMsg);
        int ok = an && bn && ae && be && DYN_BN_cmp(an, bn, dynMsg) == 0 && DYN_BN_cmp(ae, be, dynMsg) == 0;
        DYN_BN_free(an, dynMsg);
        DYN_BN_free(bn, dynMsg);
        DYN_BN_free(ae, dynMsg);
        DYN_BN_free(be, dynMsg);
        KeylessCheckDynMsg(dynMsg, "KeylessMatch RSA");
        if (!ok) {
            return 0;
        }
    } else if (a->type == KEYLESS_KEY_TYPE_EC) {
        if (a->ecPointLen != b->ecPointLen) {
            return 0;
        }
        if (memcmp(a->ecPoint, b->ecPoint, a->ecPointLen) != 0) {
            return 0;
        }
        if ((a->groupName == NULL) != (b->groupName == NULL)) {
            return 0;
        }
        if (a->groupName && strcmp(a->groupName, b->groupName) != 0) {
            return 0;
        }
    }

    /* We do not hold private material locally, but selections may include it.
     * Returning success signals that remote storage would satisfy the match. */
    return 1;
}

static const char* DefaultDigestForKey(const KeylessKey* k)
{
    if (k->type == KEYLESS_KEY_TYPE_EC && k->groupName) {
        unsigned ob = GetEcOrderBitsFromGroup(k->groupName);
        if (ob >= 512) {
            return "SHA512";
        }
        if (ob >= 384) {
            return "SHA384";
        }
        return "SHA256"; /* P-256 and similar */
    }
    if (k->type == KEYLESS_KEY_TYPE_RSA) {
        unsigned rsaBits = (unsigned)(k->nLen * 8);
        /* @refer NIST SP 800-57 Part 1 Rev. 5 Table 2 */
        if (rsaBits >= 7680) {
            return "SHA512";
        }
        if (rsaBits >= 3072) {
            return "SHA384";
        }
        return "SHA256";
    }
    return "SHA256";
}

static int KeylessGetRsaParams(KeylessKey* keyData, OSSL_PARAM params[], OSSL_PARAM* p, DynMsg* dynMsg)
{
    if (!keyData || keyData->type != KEYLESS_KEY_TYPE_RSA) {
        return 0;
    }

    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_RSA_N, dynMsg))) {
        BIGNUM* bn = DYN_BN_bin2bn(keyData->n, (int)keyData->nLen, NULL, dynMsg);
        if (!bn || !DYN_OSSL_PARAM_set_BN(p, bn, dynMsg)) {
            DYN_BN_free(bn, dynMsg);
            return 0;
        }
        DYN_BN_free(bn, dynMsg);
    }
    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_RSA_E, dynMsg))) {
        BIGNUM* be = DYN_BN_bin2bn(keyData->e, (int)keyData->eLen, NULL, dynMsg);
        if (!be || !DYN_OSSL_PARAM_set_BN(p, be, dynMsg)) {
            DYN_BN_free(be, dynMsg);
            return 0;
        }
        DYN_BN_free(be, dynMsg);
    }

    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_MAX_SIZE, dynMsg))) {
        DYN_OSSL_PARAM_set_size_t(p, keyData->nLen, dynMsg);
    }

    unsigned int rsaBits = (unsigned int)(keyData->nLen * 8);
    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_BITS, dynMsg))) {
        DYN_OSSL_PARAM_set_uint(p, rsaBits, dynMsg);
    }
    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_SECURITY_BITS, dynMsg))) {
        /* @refer NIST SP 800-57 Part 1 Rev. 5 Table 2 */
        unsigned secureBits = (rsaBits >= 15360) ? 256u : (rsaBits >= 7680) ? 192u : (rsaBits >= 3072) ? 128u : (rsaBits >= 2048) ? 112u : 0u;
        if (secureBits) {
            DYN_OSSL_PARAM_set_uint(p, secureBits, dynMsg);
        }
    }
    return 1;
}

static void KeylessGetEcParams(KeylessKey* keyData, OSSL_PARAM params[], OSSL_PARAM* p, DynMsg* dynMsg)
{
    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_PUB_KEY, dynMsg))) {
        DYN_OSSL_PARAM_set_octet_string(p, keyData->ecPoint, keyData->ecPointLen, dynMsg);
    }
    if (keyData->groupName && (p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_GROUP_NAME, dynMsg))) {
        DYN_OSSL_PARAM_set_utf8_string(p, keyData->groupName, dynMsg);
    }

    unsigned orderBits = GetEcOrderBitsFromGroup(keyData->groupName);
    if (orderBits) {
        if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_MAX_SIZE, dynMsg))) {
            size_t nbytes = orderBits / 8 + ((orderBits % 8) ? 1 : 0);
            /* Upper bound for ECDSA DER signature length. Two INTEGERs plus overhead; safe simple bound: 2*nbytes + 10. */
            size_t derSigMax = 2 * nbytes + 10;
            DYN_OSSL_PARAM_set_size_t(p, derSigMax, dynMsg);
        }

        /* Also expose BITS/SECURITY_BITS for EC keys to satisfy
         * OpenSSL key validity and size probes during handshake. */
        if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_BITS, dynMsg))) {
            DYN_OSSL_PARAM_set_uint(p, orderBits, dynMsg);
        }
        if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_SECURITY_BITS, dynMsg))) {
            unsigned secBits = 0u;
            /* Rough NIST-style mapping: 256->128, 384->192, 521->256 */
            if (orderBits >= 521u) {
                secBits = 256u;
            } else if (orderBits >= 384u) {
                secBits = 192u;
            } else if (orderBits >= 256u) {
                secBits = 128u;
            }
            if (secBits) {
                DYN_OSSL_PARAM_set_uint(p, secBits, dynMsg);
            }
        }
    } else {
        if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_MAX_SIZE, dynMsg))) {
            DYN_OSSL_PARAM_set_size_t(p, 160, dynMsg); /* > P-521 ECDSA DER max (~158B), 160B will be enough. */
        }
    }
}

static int KeylessGetParams(void* keyData, OSSL_PARAM params[])
{
    if (!keyData) {
        return 0;
    }

    KeylessKey* k = keyData;
    OSSL_PARAM* p = NULL;
    DynMsg* dynMsg = KeylessProviderGetThreadDynMsg();

    if (k->type == KEYLESS_KEY_TYPE_RSA) {
        if (KeylessGetRsaParams(k, params, p, dynMsg) == 0) {
            return 0;
        }
    } else if (k->type == KEYLESS_KEY_TYPE_EC) {
        KeylessGetEcParams(k, params, p, dynMsg);
    }

    if ((p = DYN_OSSL_PARAM_locate(params, "KEYLESS_ID", dynMsg))) {
        DYN_OSSL_PARAM_set_utf8_string(p, k->keyId, dynMsg);
    }

    if ((p = DYN_OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_DEFAULT_DIGEST, dynMsg))) {
        const char* md = DefaultDigestForKey(k);
        DYN_OSSL_PARAM_set_utf8_string(p, md, dynMsg);
    }

    KeylessCheckDynMsg(dynMsg, "KeylessGetParams");

    return 1;
}

static const OSSL_PARAM* KeylessGettableParams(void* provctx)
{
    static const OSSL_PARAM gettable[] = {OSSL_PARAM_utf8_string("KEYLESS_ID", NULL, 0),
                                          OSSL_PARAM_uint(OSSL_PKEY_PARAM_BITS, NULL),
                                          OSSL_PARAM_uint(OSSL_PKEY_PARAM_SECURITY_BITS, NULL),
                                          OSSL_PARAM_size_t(OSSL_PKEY_PARAM_MAX_SIZE, NULL),
                                          OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_DEFAULT_DIGEST, NULL, 0),
                                          OSSL_PARAM_END};
    return gettable;
}

/**
 * OSSL_PKEY_PARAM_RSA_N: BIGNUM for RSA modulus
 * OSSL_PKEY_PARAM_RSA_E: BIGNUM for RSA public exponent
 * OSSL_PKEY_PARAM_PUB_KEY: octet string for EC public point
 * OSSL_PKEY_PARAM_GROUP_NAME: UTF8 string for EC group name, like "prime256v1"
 * KEYLESS_ID: UTF8 string for key identifier
 */
static const OSSL_PARAM* KeylessImportTypes(int selection)
{
    static const OSSL_PARAM params[] = {OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_N, NULL, 0),
                                        OSSL_PARAM_BN(OSSL_PKEY_PARAM_RSA_E, NULL, 0),
                                        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, NULL, 0),
                                        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, NULL, 0),
                                        OSSL_PARAM_utf8_string("KEYLESS_ID", NULL, 0),
                                        OSSL_PARAM_END};

    unsigned int sel = (unsigned int)selection;
    if (sel & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        return params;
    }
    return NULL;
}

/*
 * Export key material to another keymgmt via callback. We only hold public
 * attributes plus a key identifier; export those along with KEYLESS_ID so
 * operations can locate remote capabilities.
 */
static int KeylessExport(void* keyData, int selection, OSSL_CALLBACK* exportCb, void* exportCbArg)
{
    KeylessKey* k = keyData;
    if (!k || !exportCb) {
        return 0;
    }

    OSSL_PARAM params[6]; /* max 5 params(@see KeylessExportTypes) + end */
    size_t idx = 0;
    unsigned int sel = (unsigned int)selection;
    if (sel & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        if (k->type == KEYLESS_KEY_TYPE_RSA) {
            params[idx++] = DYN_OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_RSA_N, k->n, k->nLen, KeylessProviderGetThreadDynMsg());
            params[idx++] = DYN_OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_RSA_E, k->e, k->eLen, KeylessProviderGetThreadDynMsg());
        } else if (k->type == KEYLESS_KEY_TYPE_EC) {
            params[idx++] = DYN_OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_PUB_KEY, k->ecPoint, k->ecPointLen, KeylessProviderGetThreadDynMsg());
            if (k->groupName) {
                params[idx++] = DYN_OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, k->groupName, 0, KeylessProviderGetThreadDynMsg());
            }
        }
        if (k->keyId) {
            params[idx++] = DYN_OSSL_PARAM_construct_utf8_string("KEYLESS_ID", k->keyId, 0, KeylessProviderGetThreadDynMsg());
        }
    }

    params[idx++] = DYN_OSSL_PARAM_construct_end(KeylessProviderGetThreadDynMsg());

    return exportCb(params, exportCbArg);
}

static const OSSL_PARAM* KeylessExportTypes(int selection)
{
    static const OSSL_PARAM export_public_union[] = {OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_RSA_N, NULL, 0),
                                                     OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_RSA_E, NULL, 0),
                                                     OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_PUB_KEY, NULL, 0),
                                                     OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, NULL, 0),
                                                     OSSL_PARAM_utf8_string("KEYLESS_ID", NULL, 0),
                                                     OSSL_PARAM_END};
    unsigned int sel = (unsigned int)selection;
    if (sel & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        return export_public_union;
    }
    return NULL;
}

/**
 * @brief Maps operation IDs to algorithm names for RSA keys.
 * @param operation_id The operation ID to query.
 * @return The corresponding algorithm name if supported; NULL otherwise.
 */
static const char* KeylessRsaQueryOpName(int operation_id)
{
    if (operation_id == OSSL_OP_SIGNATURE || operation_id == OSSL_OP_ASYM_CIPHER) {
        KeylessProviderLog("[keyless] query operation id %d\n", operation_id);
        return "RSA";
    }
    return NULL;
}

/**
 * @brief Maps operation IDs to algorithm names for EC keys.
 * @param operation_id The operation ID to query.
 * @return The corresponding algorithm name if supported; NULL otherwise.
 */
static const char* KeylessEcQueryOpName(int operation_id)
{
    if (operation_id == OSSL_OP_SIGNATURE) {
        return "ECDSA";
    }
    return NULL;
}

static const OSSL_DISPATCH keylessRsaKeymgmtFunctions[] = {{OSSL_FUNC_KEYMGMT_NEW, (void (*)(void))KeylessNew},
                                                           {OSSL_FUNC_KEYMGMT_FREE, (void (*)(void))KeylessFree},
                                                           {OSSL_FUNC_KEYMGMT_LOAD, (void (*)(void))KeylessLoad},
                                                           {OSSL_FUNC_KEYMGMT_HAS, (void (*)(void))KeylessHas},
                                                           {OSSL_FUNC_KEYMGMT_MATCH, (void (*)(void))KeylessMatch},
                                                           {OSSL_FUNC_KEYMGMT_IMPORT, (void (*)(void))KeylessImport},
                                                           {OSSL_FUNC_KEYMGMT_GET_PARAMS, (void (*)(void))KeylessGetParams},
                                                           {OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS, (void (*)(void))KeylessGettableParams},
                                                           {OSSL_FUNC_KEYMGMT_IMPORT_TYPES, (void (*)(void))KeylessImportTypes},
                                                           {OSSL_FUNC_KEYMGMT_EXPORT, (void (*)(void))KeylessExport},
                                                           {OSSL_FUNC_KEYMGMT_EXPORT_TYPES, (void (*)(void))KeylessExportTypes},
                                                           {OSSL_FUNC_KEYMGMT_QUERY_OPERATION_NAME, (void (*)(void))KeylessRsaQueryOpName},
                                                           {0, NULL}};

static const OSSL_DISPATCH keylessEcKeymgmtFunctions[] = {{OSSL_FUNC_KEYMGMT_NEW, (void (*)(void))KeylessNew},
                                                          {OSSL_FUNC_KEYMGMT_FREE, (void (*)(void))KeylessFree},
                                                          {OSSL_FUNC_KEYMGMT_LOAD, (void (*)(void))KeylessLoad},
                                                          {OSSL_FUNC_KEYMGMT_HAS, (void (*)(void))KeylessHas},
                                                          {OSSL_FUNC_KEYMGMT_MATCH, (void (*)(void))KeylessMatch},
                                                          {OSSL_FUNC_KEYMGMT_IMPORT, (void (*)(void))KeylessImport},
                                                          {OSSL_FUNC_KEYMGMT_GET_PARAMS, (void (*)(void))KeylessGetParams},
                                                          {OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS, (void (*)(void))KeylessGettableParams},
                                                          {OSSL_FUNC_KEYMGMT_IMPORT_TYPES, (void (*)(void))KeylessImportTypes},
                                                          {OSSL_FUNC_KEYMGMT_QUERY_OPERATION_NAME, (void (*)(void))KeylessEcQueryOpName},
                                                          {0, NULL}};

/* Expose KEYMGMT algorithms */
const OSSL_ALGORITHM keylessKeymgmtAlgorithms[] = {{"RSA", "provider=keyless", keylessRsaKeymgmtFunctions, "Keyless RSA public key"},
                                                   {"EC", "provider=keyless", keylessEcKeymgmtFunctions, "Keyless EC public key"},
                                                   {NULL, NULL, NULL, NULL}};
