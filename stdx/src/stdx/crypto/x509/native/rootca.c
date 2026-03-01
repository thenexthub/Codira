/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifdef _WIN32
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#undef X509_NAME
#include "opensslSymbols.h"

extern const char* DYN_CODE_SystemRootCerts(DynMsg* dynMsg)
{
    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    X509* x509 = NULL;
    BIO* bio = DYN_BIO_new_mem(dynMsg);
    if (bio == NULL) {
        return NULL;
    }
    char* certStr = NULL;
    hStore = CertOpenSystemStore(0, "ROOT");
    if (hStore == NULL) {
        DYN_BIO_free(bio, dynMsg);
        return NULL;
    }
    while ((pCertContext = CertEnumCertificatesInStore(hStore, pCertContext)) != NULL) {
        x509 = DYN_d2i_X509(
            NULL, (const unsigned char**)&pCertContext->pbCertEncoded, pCertContext->cbCertEncoded, dynMsg);
        if (x509 == NULL) {
            fprintf(stderr, "Failed to convert certificate to X509 structure.\n");
            continue;
        }
        DYN_PEM_write_bio_X509(bio, x509, dynMsg);
        DYN_X509_free(x509, dynMsg);
    }
    certStr = (char*)malloc(DYN_BIO_pending(bio, dynMsg) + 1);
    if (certStr != NULL) {
        size_t length = DYN_BIO_pending(bio, dynMsg);
        DYN_BIO_read(bio, certStr, length, dynMsg);
        certStr[length] = 0;
    }
    DYN_BIO_free(bio, dynMsg);
    CertCloseStore(hStore, 0);
    return certStr;
}
#endif
