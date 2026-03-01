/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_DEFINE_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_DEFINE_H

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <securec.h>
#if defined(WINDOWS_PLATFORM)
#include <winsock2.h>
#include <windows.h>
#ifdef ERROR
#undef ERROR
#endif
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#endif
#include <unistd.h>

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_DEFINE_H
