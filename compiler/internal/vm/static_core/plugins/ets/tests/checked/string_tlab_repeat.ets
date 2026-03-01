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

/* the irtoc-based implementations that use TLAB-allocation fallback
 * to the corresponding c++ implementation in the case TLAB is not
 * available, to be sure a test is testing the irtoc implementation we
 * have to enforce the use of TLAB via --no-async-jit=false and ensure
 * the function is run in compiled mode by using compileMethod() */

//! CHECKER      Use TLAB for String.repeat() allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_repeat.ETSGLOBAL::testStringRepeat"

//! CHECKER      Use TLAB for String.repeat() allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*StringRepeat.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_repeat.ETSGLOBAL::testStringRepeat"

native function compileMethod(str: String) : int;

const Hello: String = 'Hello';
const HelloDup: String = 'HelloHelloHelloHelloHelloHelloHelloHello';
const Nature: String = '自然界';
const NatureDup: String = '自然界自然界自然界自然界自然界自然界自然界自然界';

function StringRepeat(): int {
    if (Hello.repeat(0) != "") {
        return 1;
    }
    if (Hello.repeat(1) != Hello) {
        return 2;
    }
    if (Hello.repeat(8) != HelloDup) {
        return 3;
    }

    if (Nature.repeat(0) != "") {
        return 10+1;
    }
    if (Nature.repeat(1) != Nature) {
        return 10+2;
    }
    if (Nature.repeat(8) != NatureDup) {
        return 10+3;
    }
    if ("".repeat(1) != "") {
        return 20;
    }

    return 0;
}

function testStringRepeat()
{
    loadLibrary("string_tlab_repeat");
    compileMethod("string_tlab_repeat.ETSGLOBAL::StringRepeat");
    return StringRepeat();
}
