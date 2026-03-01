/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "opensslSymbols.h"
#include "api.h"

enum TlsProtoVersion {
    CODETLS_PROTO_VERSION_1_2 = 0, /* TLS1.2 */
    CODETLS_PROTO_VERSION_1_3 = 1, /* TLS1.3 */
    CODETLS_PROTO_VERSION_BUTT,
};

extern const char* CODE_TLS_DYN_GetVersion(SSL* ssl, DynMsg* dynMsg)
{
    if (ssl == NULL) {
        return NULL;
    }

    return DYN_SSL_get_version(ssl, dynMsg);
}

extern int CODE_TLS_DYN_SetProtoVersions(SSL_CTX* ctx, enum TlsProtoVersion min, enum TlsProtoVersion max, DynMsg* dynMsg)
{
    int ret;
    int sslVersionMin;
    int sslVersionMax;

    if (ctx == NULL) {
        return 0;
    }

    if (max < min || max >= CODETLS_PROTO_VERSION_BUTT) {
        return -1;
    }

    if (min == CODETLS_PROTO_VERSION_1_2) {
        sslVersionMin = TLS1_2_VERSION;
    } else {
        sslVersionMin = TLS1_3_VERSION;
    }

    if (max == CODETLS_PROTO_VERSION_1_2) {
        sslVersionMax = TLS1_2_VERSION;
    } else {
        sslVersionMax = TLS1_3_VERSION;
    }

    ret = DYN_SSL_CTX_set_min_proto_version(ctx, sslVersionMin, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return -1;
    }
    if (ret <= 0) {
        return ret;
    }

    ret = DYN_SSL_CTX_set_max_proto_version(ctx, sslVersionMax, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return -1;
    }
    if (ret <= 0) {
        return ret;
    }

    return 1;
}
