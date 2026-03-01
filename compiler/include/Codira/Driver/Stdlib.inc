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
 * This file lists paths to std module bitcode files, which is included by headers.
 */
#ifdef STDLIB
STDLIB(CORE, "std", "core")
STDLIB(BINARY, "std", "binary")
STDLIB(IO, "std", "io")
STDLIB(MATH, "std", "math")
STDLIB(OVERFLOW, "std", "overflow")
STDLIB(RUNTIME, "std", "runtime")
STDLIB(CONVERT, "std", "convert")
STDLIB(RANDOM, "std", "random")
STDLIB(COLLECTION, "std", "collection")
STDLIB(UNICODE, "std", "unicode")
STDLIB(SORT, "std", "sort")
STDLIB(ARGOPT, "std", "argopt")
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
STDLIB(AST, "std", "ast")
STDLIB(INTEROP, "std", "interop")
#endif
STDLIB(TIME, "std", "time")
STDLIB(SYNC, "std", "sync")
STDLIB(COLLECTION.CONCURRENT, "std", "collection.concurrent")
STDLIB(NET, "std", "net")
STDLIB(REGEX, "std", "regex")
STDLIB(UNITTEST.COMMON, "std", "unittest.common")
STDLIB(UNITTEST.PROP_TEST, "std", "unittest.prop_test")
STDLIB(UNITTEST.DIFF, "std", "unittest.diff")
STDLIB(MATH.NUMERIC, "std", "math.numeric")
STDLIB(FS, "std", "fs")
STDLIB(UNITTEST.MOCK.INTERNAL, "std", "unittest.mock.internal")
STDLIB(UNITTEST.MOCK, "std", "unittest.mock")
STDLIB(REFLECT, "std", "reflect")
STDLIB(REF, "std", "ref")
STDLIB(STD.CRYPTO, "std", "crypto")
STDLIB(CRYPTO.DIGEST, "std", "crypto.digest")
STDLIB(CRYPTO.CIPHER, "std", "crypto.cipher")
STDLIB(CONSOLE, "std", "console")
STDLIB(DATABASE, "std", "database")
STDLIB(DATABASE.SQL, "std", "database.sql")
STDLIB(POSIX, "std", "posix")
STDLIB(PROCESS, "std", "process")
STDLIB(ENV, "std", "env")
STDLIB(OBJECTPOOL, "std", "objectpool")
STDLIB(UNITTEST, "std", "unittest")

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
STDLIB(DERIVING.API, "std", "deriving.api")
STDLIB(DERIVING.RESOLVE, "std", "deriving.resolve")
STDLIB(DERIVING.IMPL, "std", "deriving.impl")
STDLIB(DERIVING.BUILTINS, "std", "deriving.builtins")
STDLIB(DERIVING, "std", "deriving")
STDLIB(UNITTEST.TESTMACRO, "std", "unittest.testmacro")
STDLIB(UNITTEST.MOCK.MOCKMACRO, "std", "unittest.mock.mockmacro")
#endif // CODIRA_CODEGEN_CODENATIVE_BACKEND

STDLIB_TOPPKG(STD, "std")

#ifndef BACKEND_BC
STDLIB_ROOTPKG(STD, "std")
STDLIB(FFI.PYTHON, "std", "ffi.python")
// This will cause `codestack` cannot be linked, because missing symbol `LLVMFuzzerRunDriver`.
STDLIB(FUZZ, "fuzz", "fuzz")
#endif
#endif
