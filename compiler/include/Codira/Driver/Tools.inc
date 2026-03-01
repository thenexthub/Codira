/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file lists external binary tools to be used, which is included by headers.
 */

#ifdef TOOL
#ifdef _WIN32
TOOL(CODEC, FRONTEND, "codec.exe")
TOOL(CLANG, BACKEND, "clang.exe")
TOOL(OPT, BACKEND, "opt.exe")
TOOL(LLC, BACKEND, "llc.exe")
TOOL(LLD, OTHER, "ld.lld.exe")
TOOL(LD64_LLD, OTHER, "ld64.lld.exe")
TOOL(LD, BACKEND, "ld.exe")
TOOL(AR, OTHER, "ar.exe")
TOOL(LLVM_AR, OTHER, "llvm-ar.exe")
TOOL(OBJCOPY, OTHER, "objcopy.exe")
TOOL(LLVM_OBJCOPY, OTHER, "llvm-objcopy.exe")
TOOL(CMD, OTHER, "cmd.exe")
TOOL(DSYMUTIL, OTHER, "dsymutil.exe")
TOOL(CODESIGN, OTHER, "codesign.exe")
TOOL(STRIP, BACKEND, "strip.exe")
#else
TOOL(CODEC, FRONTEND, "codec")
TOOL(CLANG, BACKEND, "clang")
TOOL(OPT, BACKEND, "opt")
TOOL(LLC, BACKEND, "llc")
TOOL(LD, BACKEND, "ld")
TOOL(LLD, OTHER, "ld.lld")
TOOL(LD64_LLD, OTHER, "ld64.lld")
TOOL(AR, OTHER, "ar")
TOOL(LLVM_AR, OTHER, "llvm-ar")
TOOL(OBJCOPY, OTHER, "objcopy")
TOOL(LLVM_OBJCOPY, OTHER, "llvm-objcopy")
TOOL(DSYMUTIL, OTHER, "dsymutil")
TOOL(CODESIGN, OTHER, "codesign")
TOOL(STRIP, BACKEND, "strip")
#endif
#endif
