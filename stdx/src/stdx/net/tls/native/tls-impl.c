/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <string.h>
#include "securec.h"
#include "api.h"
#include "opensslSymbols.h"

static const char* TLS_HANDSHAKE_FAILED_SERVER = "TLS handshake failed (server)";
static const char* TLS_HANDSHAKE_FAILED_CLIENT = "TLS handshake failed (client)";

static int g_exceptionDataIndex = -1; // initialized int SslInit

static BIO* CreateBio(ExceptionData* exception, DynMsg* dynMsg)
{
    BIO_METHOD* method = CODE_TLS_BIO_GetMethod(exception, dynMsg);
    if (method == NULL) { // return NULL if dynMsg->found == false
        return NULL;
    }

    BIO* mem = DYN_BIO_new(method, dynMsg);
    if (!mem) {
        return NULL;
    }

    return mem;
}

static void PutExceptionData(SSL* ssl, const ExceptionData* exception, DynMsg* dynMsg)
{
    if (ssl != NULL && g_exceptionDataIndex != -1) {
        (void)DYN_SSL_set_ex_data(ssl, g_exceptionDataIndex, (void*)exception, dynMsg);
    }
}

static void RemoveExceptionData(SSL* ssl, DynMsg* dynMsg)
{
    if (ssl != NULL && g_exceptionDataIndex != -1) {
        (void)DYN_SSL_set_ex_data(ssl, g_exceptionDataIndex, NULL, dynMsg);
    }
}

// check dynamic func outside
static void InfoCallback(const SSL* ssl, int type, int val)
{
    if ((type & SSL_CB_READ_ALERT) && g_exceptionDataIndex != -1) {
        ExceptionData* exception = (ExceptionData*)DYN_SSL_get_ex_data(ssl, g_exceptionDataIndex, NULL);
        if (exception != NULL) {
            HandleAlertError(exception, DYN_SSL_alert_desc_string_long(val, NULL), DYN_SSL_alert_type_string(val, NULL), NULL);
        }
    }
}

static int CheckParams(const SSL* ssl, const char* buffer, int size, ExceptionData* exception, DynMsg* dynMsg)
{
    if (!exception) {
        return -1;
    }
    if (!buffer) {
        HandleError(exception, "buffer shouldn't be NULL", dynMsg);
        return -1;
    }
    if (!ssl) {
        HandleError(exception, "SSL shouldn't be NULL", dynMsg);
        return -1;
    }
    if (size <= 0) {
        HandleError(exception, "buffer size should be positive", dynMsg);
        return -1;
    }
    return 0;
}

static BIO* MapInputBio(SSL* ssl, void* rawInput, size_t rawInputSize, int rawInputLast, ExceptionData* exception, DynMsg* dynMsg)
{
    BIO* inputBio = DYN_SSL_get_rbio(ssl, dynMsg);
    if (inputBio == NULL) {
        HandleError(exception, "SSL instance has no read BIO", dynMsg);
        return NULL;
    }

    // return -1 if find dynamic function failed
    if (CODE_TLS_BIO_Map(inputBio, rawInput, rawInputSize, rawInputLast, exception, dynMsg) == CODETLS_FAIL) {
        return NULL;
    }

    return inputBio;
}

static BIO* MapOutputBio(SSL* ssl, void* rawOutput, size_t rawOutputSize, ExceptionData* exception, DynMsg* dynMsg)
{
    BIO* outputBio = DYN_SSL_get_wbio(ssl, dynMsg);
    if (dynMsg && dynMsg->found == false) {
        return NULL;
    }
    if (outputBio == NULL) {
        HandleError(exception, "SSL instance has no write BIO", dynMsg);
        return NULL;
    }

    if (CODE_TLS_BIO_Map(outputBio, rawOutput, rawOutputSize, 0, exception, dynMsg) == CODETLS_FAIL) {
        return NULL;
    }

    return outputBio;
}

static int SslReadFailed(SSL* ssl, int rc, ExceptionData* exception, DynMsg* dynMsg)
{
    if ((DYN_SSL_get_shutdown(ssl, dynMsg) & SSL_SENT_SHUTDOWN) != 0) {
        return CODETLS_EOF;
    }
    if (rc == 0) {
        // it is returned in all unknown cases
        BIO* bio = DYN_SSL_get_rbio(ssl, dynMsg);
        if (bio != NULL && DYN_BIO_eof(bio, dynMsg) != 0) {
            return CODETLS_EOF;
        }
    }

    HandleError(exception, "TLS failed to read data", dynMsg);
    return CODETLS_FAIL;
}

static int SslRead(SSL* ssl, char* buffer, int size, ExceptionData* exception, DynMsg* dynMsg)
{
    if (CheckParams(ssl, buffer, size, exception, dynMsg) != 0) {
        return CODETLS_FAIL;
    }

    DYN_ERR_clear_error(dynMsg);
    PutExceptionData(ssl, exception, dynMsg);
    int rc = DYN_SSL_read(ssl, buffer, size, dynMsg);
    RemoveExceptionData(ssl, dynMsg);

    if (rc <= 0) {
        int error = DYN_SSL_get_error(ssl, rc, dynMsg);
        switch (error) {
            case SSL_ERROR_ZERO_RETURN:
                return CODETLS_EOF;
            case SSL_ERROR_WANT_READ:
                return CODETLS_NEED_READ;
            case SSL_ERROR_WANT_WRITE:
                return CODETLS_NEED_WRITE;
            default:
                return SslReadFailed(ssl, rc, exception, dynMsg);
        }
    }

    return rc;
}

/**
 * Pass encrypted rawInput:rawInputSize to OpenSSL also providing rawOutput:rawOutputSize for writing
 * and get decrypted data to dataBuffer:dataBufferSize
 * updating dataBytesRead (to dataBuffer), rawBytesConsumed (from rawInput) and
 * rawBytesProduced (to rawOutput) correspondingly
 * returns: CODETLS_OK | CODETLS_EOF | CODETLS_AGAIN | CODETLS_FAIL
 */
extern int CODE_TLS_DYN_SslRead(SSL* ssl, char* dataBuffer, int dataBufferSize, void* rawInput, size_t rawInputSize, int rawInputLast, void* rawOutput, size_t rawOutputSize,
                              size_t* dataBytesRead, size_t* rawBytesConsumed, size_t* rawBytesProduced, ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_FAIL(exception, dynMsg);
    NOT_NULL_OR_FAIL(exception, dataBuffer, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawInput, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawOutput, dynMsg);
    NOT_NULL_OR_FAIL(exception, dataBytesRead, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesConsumed, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesProduced, dynMsg);
    CHECK_OR_FAIL(exception, dataBufferSize > 0, dynMsg);

    *dataBytesRead = 0;
    *rawBytesConsumed = 0;
    *rawBytesProduced = 0;

    BIO* inputBio = MapInputBio(ssl, rawInput, rawInputSize, rawInputLast, exception, dynMsg);
    if (inputBio == NULL) {
        return CODETLS_FAIL;
    }

    BIO* outputBio = MapOutputBio(ssl, rawOutput, rawOutputSize, exception, dynMsg);
    if (outputBio == NULL) {
        return CODETLS_FAIL;
    }

    int result = SslRead(ssl, dataBuffer, dataBufferSize, exception, dynMsg);

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int inputConsumed = CODE_TLS_BIO_Unmap(inputBio, rawInputLast, exception, dynMsg);
    if (inputConsumed > 0) {
        *rawBytesConsumed = (size_t)inputConsumed;
    }

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int outputProduced = CODE_TLS_BIO_Unmap(outputBio, 0, exception, dynMsg);
    if (outputProduced > 0) {
        *rawBytesProduced = (size_t)outputProduced;
    }

    if (result > 0) {
        *dataBytesRead = (size_t)result;
        return CODETLS_OK;
    }

    return result;
}

static int SSlWrite(SSL* ssl, char* buffer, int size, ExceptionData* exception, DynMsg* dynMsg)
{
    if (CheckParams(ssl, buffer, size, exception, dynMsg) != 0) {
        return CODETLS_FAIL;
    }

    DYN_ERR_clear_error(dynMsg);
    PutExceptionData(ssl, exception, dynMsg);
    int rc = DYN_SSL_write(ssl, buffer, size, dynMsg);
    RemoveExceptionData(ssl, dynMsg);

    if (rc <= 0) {
        int error = DYN_SSL_get_error(ssl, rc, dynMsg);
        switch (error) {
            case SSL_ERROR_WANT_READ:
                return CODETLS_NEED_READ;
            case SSL_ERROR_WANT_WRITE:
                return CODETLS_NEED_WRITE;
            default:
                HandleError(exception, "TLS failed to write data", dynMsg);
                return CODETLS_FAIL;
        }
    }

    return rc;
}

/**
 * Pass encrypted rawInput:rawInputSize to OpenSSL also providing rawOutput:rawOutputSize for writing
 * and put user data dataBuffer:dataBufferSize to be encrypted and sent
 * updating dataBytesWritten (from dataBuffer), rawBytesConsumed (from rawInput) and
 * rawBytesProduced (to rawOutput) correspondingly
 * returns: CODETLS_OK | CODETLS_EOF | CODETLS_AGAIN | CODETLS_FAIL
 */
extern int CODE_TLS_DYN_SslWrite(SSL* ssl, char* dataBuffer, int dataBufferSize, void* rawInput, size_t rawInputSize, int rawInputLast, void* rawOutput, size_t rawOutputSize,
                               size_t* dataBytesWritten, size_t* rawBytesConsumed, size_t* rawBytesProduced, ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_FAIL(exception, dynMsg);
    NOT_NULL_OR_FAIL(exception, dataBuffer, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawInput, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawOutput, dynMsg);
    NOT_NULL_OR_FAIL(exception, dataBytesWritten, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesConsumed, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesProduced, dynMsg);
    CHECK_OR_FAIL(exception, dataBufferSize > 0, dynMsg);

    *rawBytesConsumed = 0;
    *rawBytesProduced = 0;
    *dataBytesWritten = 0;

    BIO* inputBio = MapInputBio(ssl, rawInput, rawInputSize, rawInputLast, exception, dynMsg);
    if (inputBio == NULL) {
        return CODETLS_FAIL;
    }

    BIO* outputBio = MapOutputBio(ssl, rawOutput, rawOutputSize, exception, dynMsg);
    if (outputBio == NULL) {
        return CODETLS_FAIL;
    }

    int result = SSlWrite(ssl, dataBuffer, dataBufferSize, exception, dynMsg);

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int inputConsumed = CODE_TLS_BIO_Unmap(inputBio, rawInputLast, exception, dynMsg);
    if (inputConsumed > 0) {
        *rawBytesConsumed = (size_t)inputConsumed;
    }

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int outputProduced = CODE_TLS_BIO_Unmap(outputBio, 0, exception, dynMsg);
    if (outputProduced > 0) {
        *rawBytesProduced = (size_t)outputProduced;
    }

    if (result > 0) {
        *dataBytesWritten = (size_t)result;
        return CODETLS_OK;
    }

    return result;
}

extern void CODE_TLS_DYN_SslInit(DynMsg* dynMsg)
{
    DYN_OPENSSL_init(dynMsg);

    int index = DYN_CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_SSL, 0, (void*)"ExceptionData pointer", NULL, NULL, NULL, dynMsg);

    g_exceptionDataIndex = index;
}

static int SetServerDefaults(SSL_CTX* ctx, ExceptionData* exception, DynMsg* dynMsg)
{
    // SSL_OP_NO_TICKET is set to emulate TLS1.2 behaviour with TLS1.3 so session reuse works the same
    // see https://github.com/openssl/openssl/issues/11039
    // this is a workaround and should be replaced with the proper fix
    // it's less efficient but safe

    /* 禁用 TLS1.0, TLS1.1 以及重协商 */
    DYN_SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_RENEGOTIATION | SSL_OP_NO_TICKET, dynMsg);

    /* 设置默认的 TLS1.2 加密套 */
    int ret = DYN_SSL_CTX_set_cipher_list(ctx,
                                          "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:"
                                          "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:"
                                          "DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384",
                                          dynMsg);
    if (ret <= 0) {
        HandleError(exception, "TLS failed to configure ciphers: SSL_CTX_set_cipher_list() failed", dynMsg);
        return CODETLS_FAIL;
    }

    /* 设置默认的 TLS1.3 加密套 */
    ret = DYN_SSL_CTX_set_ciphersuites(ctx, "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256", dynMsg);
    if (ret <= 0) {
        HandleError(exception, "TLS failed to configure ciphers: SSL_CTX_set_ciphersuites() failed", dynMsg);
        return CODETLS_FAIL;
    }

    return CODETLS_OK;
}

static int SetClientDefaults(SSL_CTX* ctx, ExceptionData* exception, DynMsg* dynMsg)
{
    /* 默认禁用不安全的加密套 */
    int ret = DYN_SSL_CTX_set_cipher_list(ctx, "DEFAULT:!aNULL:!eNULL:!MD5:!3DES:!DES:!RC4:!IDEA:!SEED:!aDSS:!SRP:!PSK", dynMsg);
    if (ret <= 0) {
        HandleError(exception, "TLS failed to configure ciphers: SSL_CTX_set_cipher_list() failed", dynMsg);
        return CODETLS_FAIL;
    }

    /* 默认客户端需要校验服务端的证书 */
    DYN_SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL, dynMsg);

    (void)DYN_SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH | SSL_SESS_CACHE_NO_INTERNAL, dynMsg);
    DYN_SSL_CTX_sess_set_new_cb(ctx, NewSessionCallback, dynMsg);

    return CODETLS_OK;
}

static unsigned long DefaultOptions(void)
{
    unsigned long options = SSL_OP_ALL | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION | SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE;
    options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
    return options;
}

extern SSL_CTX* CODE_TLS_DYN_CreateContext(int server, void (*keylogCallback)(const SSL*, const char*), ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_RETURN(exception, NULL, dynMsg);

    const SSL_METHOD* method;
    if (server != 0) {
        method = DYN_TLS_server_method(dynMsg);
    } else {
        method = DYN_TLS_client_method(dynMsg);
    }

    SSL_CTX* ctx = DYN_SSL_CTX_new(method, dynMsg);
    if (!ctx) {
        HandleError(exception, "TLS failed to create context", dynMsg);
        return NULL;
    }

    (void)DYN_SSL_CTX_set_options(ctx, DefaultOptions(), dynMsg);

    if (!LoadDynForInfoCallback(dynMsg)) {
        return NULL;
    }

    DYN_SSL_CTX_set_info_callback(ctx, InfoCallback, dynMsg);

    if (keylogCallback != NULL) {
        DYN_SSL_CTX_set_keylog_callback(ctx, keylogCallback, dynMsg);
    }

    if (DYN_SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION, dynMsg) != 1) {
        HandleError(exception, "OpenSSL SSL_CTX_set_min_proto_version() failed", dynMsg);
        DYN_SSL_CTX_free(ctx, dynMsg);
        return NULL;
    }

    if (DYN_SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION, dynMsg) != 1) {
        HandleError(exception, "OpenSSL SSL_CTX_set_max_proto_version() failed", dynMsg);
        DYN_SSL_CTX_free(ctx, dynMsg);
        return NULL;
    }

    /**
     * Set default locations for trusted CA certificates, including the default path and file name.
     * The default certificate path is "certs" under the OpenSSL default path, and the default certificate name is "cert.pem".
     * The default certificate path can be changed through the environment variable "SSL_CERT_DIR".
     * The default certificate name can be changed through the environment variable "SSL_CERT_FILE".
     */
    if (DYN_SSL_CTX_set_default_verify_paths(ctx, dynMsg) != 1) {
        HandleError(exception, "OpenSSL SSL_CTX_set_default_verify_paths() failed", dynMsg);
        DYN_SSL_CTX_free(ctx, dynMsg);
        return NULL;
    }

    long mode = SSL_MODE_AUTO_RETRY | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_RELEASE_BUFFERS;
    (void)DYN_SSL_CTX_set_mode(ctx, mode, dynMsg);

    int configResult;
    if (server != 0) {
        configResult = SetServerDefaults(ctx, exception, dynMsg);
    } else {
        configResult = SetClientDefaults(ctx, exception, dynMsg);
    }

    if (configResult == CODETLS_FAIL) {
        DYN_SSL_CTX_free(ctx, dynMsg);
        return NULL;
    }

    return ctx;
}

extern void CODE_TLS_DYN_FreeContext(SSL_CTX* ctx, DynMsg* dynMsg)
{
    if (ctx != NULL) {
        DYN_SSL_CTX_free(ctx, dynMsg);
    }
}

extern SSL* CODE_TLS_DYN_CreateSsl(SSL_CTX* ctx, int server, ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_RETURN(exception, NULL, dynMsg);
    NOT_NULL_OR_RETURN(exception, ctx, NULL, dynMsg);

    SSL* ssl = DYN_SSL_new(ctx, dynMsg);
    if (ssl == NULL) {
        HandleError(exception, "SSL_new() failed", dynMsg);
        return NULL;
    }

    BIO* read = CreateBio(exception, dynMsg);
    if (read == NULL) {
        DYN_SSL_free(ssl, dynMsg);
        return NULL;
    }

    BIO* write = CreateBio(exception, dynMsg);
    if (write == NULL) {
        DYN_BIO_free(read, dynMsg);
        DYN_SSL_free(ssl, dynMsg);
        return NULL;
    }

    DYN_SSL_set_bio(ssl, read, write, dynMsg);

    if (server != 0) {
        DYN_SSL_set_accept_state(ssl, dynMsg);
    } else {
        DYN_SSL_set_connect_state(ssl, dynMsg);
    }

    return ssl;
}

extern void CODE_TLS_DYN_FreeSsl(SSL* ssl, DynMsg* dynMsg)
{
    if (ssl != NULL) {
        DYN_SSL_free(ssl, dynMsg);
    }
}

static int SslHandshakeFailed(SSL* ssl, ExceptionData* exception, DynMsg* dynMsg)
{
    const char* message;
    if (DYN_SSL_is_server(ssl, dynMsg) != 0) {
        message = TLS_HANDSHAKE_FAILED_SERVER;
    } else {
        message = TLS_HANDSHAKE_FAILED_CLIENT;
    }

    HandleError(exception, message, dynMsg);
    return CODETLS_FAIL;
}

static int SslHandshake(SSL* ssl, ExceptionData* exception, DynMsg* dynMsg)
{
    PutExceptionData(ssl, exception, dynMsg);
    int rc = DYN_SSL_do_handshake(ssl, dynMsg);
    RemoveExceptionData(ssl, dynMsg);

    if (rc == 1) {
        if (DYN_SSL_session_reused(ssl, dynMsg) == 1) {
            SSL_SESSION* session = DYN_SSL_get_session(ssl, dynMsg);
            if (session != NULL) {
                SessionReusedCallback(ssl, session);
            }
        }

        return CODETLS_OK;
    }
    if (rc == 0) {
        return SslHandshakeFailed(ssl, exception, dynMsg);
    }

    int error = DYN_SSL_get_error(ssl, rc, dynMsg);
    switch (error) {
        case SSL_ERROR_WANT_READ:
            return CODETLS_NEED_READ;
        case SSL_ERROR_WANT_WRITE:
            return CODETLS_NEED_WRITE;
        case SSL_ERROR_SSL:
            return SslHandshakeFailed(ssl, exception, dynMsg);
        default:
            return SslHandshakeFailed(ssl, exception, dynMsg);
    }
}

/**
 * Pass encrypted rawInput:rawInputSize to OpenSSL also providing rawOutput:rawOutputSize for writing
 * and try to do handshake updating dataBytesRead (to dataBuffer), rawBytesConsumed (from rawInput) and
 * rawBytesProduced (to rawOutput) correspondingly
 * returns: CODETLS_OK | CODETLS_EOF | CODETLS_AGAIN | CODETLS_FAIL
 */
extern int CODE_TLS_DYN_SslHandshake(SSL* ssl, void* rawInput, size_t rawInputSize, int rawInputLast, void* rawOutput, size_t rawOutputSize, size_t* rawBytesConsumed,
                                   size_t* rawBytesProduced, ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_FAIL(exception, dynMsg);
    NOT_NULL_OR_FAIL(exception, ssl, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawInput, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawOutput, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesConsumed, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesProduced, dynMsg);

    *rawBytesProduced = 0;
    *rawBytesConsumed = 0;

    BIO* inputBio = MapInputBio(ssl, rawInput, rawInputSize, rawInputLast, exception, dynMsg);
    if (inputBio == NULL) {
        return CODETLS_FAIL;
    }

    BIO* outputBio = MapOutputBio(ssl, rawOutput, rawOutputSize, exception, dynMsg);
    if (outputBio == NULL) {
        return CODETLS_FAIL;
    }

    int result = SslHandshake(ssl, exception, dynMsg);
    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int inputConsumed = CODE_TLS_BIO_Unmap(inputBio, rawInputLast, exception, dynMsg);
    if (inputConsumed > 0) {
        *rawBytesConsumed = (size_t)inputConsumed;
    }

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int outputProduced = CODE_TLS_BIO_Unmap(outputBio, 0, exception, dynMsg);
    if (outputProduced > 0) {
        *rawBytesProduced = (size_t)outputProduced;
    }

    return result;
}

static int SslShutdown(SSL* ssl, ExceptionData* exception, DynMsg* dynMsg)
{
    NOT_NULL_OR_FAIL(exception, ssl, dynMsg);

    int result = DYN_SSL_shutdown(ssl, dynMsg);
    if (result == 1) {
        return CODETLS_OK;
    }

    if (result == 0) {
        unsigned int state = (unsigned int)DYN_SSL_get_shutdown(ssl, dynMsg);
        if ((state & SSL_SENT_SHUTDOWN) == 0) {
            return CODETLS_NEED_WRITE;
        }
        if ((state & SSL_RECEIVED_SHUTDOWN) == 0) {
            return CODETLS_NEED_READ;
        }
        return CODETLS_OK;
    }

    int error = DYN_SSL_get_error(ssl, result, dynMsg);
    switch (error) {
        case SSL_ERROR_WANT_READ:
            return CODETLS_NEED_READ;
        case SSL_ERROR_WANT_WRITE:
            return CODETLS_NEED_WRITE;
        default:
            if ((DYN_SSL_get_shutdown(ssl, dynMsg) & SSL_SENT_SHUTDOWN) != 0) {
                return CODETLS_OK;
            }
            HandleError(exception, "TLS shutdown failed", dynMsg);
            return CODETLS_FAIL;
    }
}

extern int CODE_TLS_DYN_SslShutdown(SSL* ssl, void* rawInput, size_t rawInputSize, int rawInputLast, void* rawOutput, size_t rawOutputSize, size_t* rawBytesConsumed,
                                  size_t* rawBytesProduced, ExceptionData* exception, DynMsg* dynMsg)
{
    EXCEPTION_OR_FAIL(exception, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawInput, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawOutput, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesConsumed, dynMsg);
    NOT_NULL_OR_FAIL(exception, rawBytesProduced, dynMsg);

    *rawBytesConsumed = 0;
    *rawBytesProduced = 0;

    BIO* inputBio = MapInputBio(ssl, rawInput, rawInputSize, rawInputLast, exception, dynMsg);
    if (inputBio == NULL) {
        return CODETLS_FAIL;
    }

    BIO* outputBio = MapOutputBio(ssl, rawOutput, rawOutputSize, exception, dynMsg);
    if (outputBio == NULL) {
        return CODETLS_FAIL;
    }

    int result = SslShutdown(ssl, exception, dynMsg);

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int inputConsumed = CODE_TLS_BIO_Unmap(inputBio, rawInputLast, exception, dynMsg);
    if (inputConsumed > 0) {
        *rawBytesConsumed = (size_t)inputConsumed;
    }

    // we may potentially loose exception data if failing to unmap (that is unlikely)
    int outputProduced = CODE_TLS_BIO_Unmap(outputBio, 0, exception, dynMsg);
    if (outputProduced > 0) {
        *rawBytesProduced = (size_t)outputProduced;
    }

    return result;
}

extern int CODE_TLS_DYN_SetClientSignatureAlgorithms(SSL_CTX* ctx, const unsigned char* sigalgs, ExceptionData* exception, DynMsg* dynMsg)
{
    if (ctx == NULL || sigalgs == NULL) {
        return -1;
    }

    if (DYN_SSL_CTX_set1_sigalgs_list(ctx, (const char*)sigalgs, dynMsg) != 1) {
        HandleError(exception, "Failed to set client signature algorithms.", dynMsg);
        return -1;
    }

    return 1;
}
