//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//
// Unfortunately, even in our brave BoringSSL world, we have "functions" that are
// macros too complex for the clang importer. This file handles them.
#include "CNIOBoringSSLShims.h"

X509_EXTENSION *CNIOBoringSSLShims_sk_X509_EXTENSION_value(const STACK_OF(X509_EXTENSION) *sk, size_t i) {
    return sk_X509_EXTENSION_value(sk, i);
}

size_t CNIOBoringSSLShims_sk_X509_EXTENSION_num(const STACK_OF(X509_EXTENSION) *sk) {
    return sk_X509_EXTENSION_num(sk);
}

GENERAL_NAME *CNIOBoringSSLShims_sk_GENERAL_NAME_value(const STACK_OF(GENERAL_NAME) *sk, size_t i) {
    return sk_GENERAL_NAME_value(sk, i);
}

size_t CNIOBoringSSLShims_sk_GENERAL_NAME_num(const STACK_OF(GENERAL_NAME) *sk) {
    return sk_GENERAL_NAME_num(sk);
}

void *CNIOBoringSSLShims_SSL_CTX_get_app_data(const SSL_CTX *ctx) {
    return SSL_CTX_get_app_data(ctx);
}

int CNIOBoringSSLShims_SSL_CTX_set_app_data(SSL_CTX *ctx, void *data) {
    return SSL_CTX_set_app_data(ctx, data);
}

int CNIOBoringSSLShims_ERR_GET_LIB(uint32_t err) {
  return ERR_GET_LIB(err);
}

int CNIOBoringSSLShims_ERR_GET_REASON(uint32_t err) {
  return ERR_GET_REASON(err);
}
