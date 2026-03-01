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

//! CHECKER      Use TLAB for string from chars allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testCreateStringFromChars"
//! EVENT        /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for string from chars allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*CreateStringFromChars.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testCreateStringFromChars"
//! EVENT        /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for string from string allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testCreateStringFromString"
//! EVENT        /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for string from string allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*CreateStringFromString.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testCreateStringFromString"
//! EVENT        /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for String.getChars() allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testGetCharsFromString"
//! EVENT        /TlabAlloc,.*,64/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for String.getChars() allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*GetCharsFromString.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testGetCharsFromString"
//! EVENT        /TlabAlloc,.*,64/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for String.getBytes() allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testGetBytesFromString"
//! EVENT        /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,32/

//! CHECKER      Use TLAB for String.getBytes() allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*GetBytesFromString.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testGetBytesFromString"
//! EVENT        /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,32/

//! CHECKER      Use TLAB for String.substring() allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testStringSubstring"
//! EVENT        /TlabAlloc,.*,32/
//! EVENT_NEXT   /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,56/

//! CHECKER      Use TLAB for String.substring() allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*StringSubstring.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testStringSubstring"
//! EVENT        /TlabAlloc,.*,32/
//! EVENT_NEXT   /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,56/

//! CHECKER      Use TLAB for String.concat() allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testStringConcat"
//! EVENT_NEXT   /TlabAlloc,.*,40/
//! EVENT_NEXT   /TlabAlloc,.*,56/
//! EVENT_NEXT   /TlabAlloc,.*,64/

/* No AOT test for the concat() test as it requires AOT'ing of etsstdlib */

//! CHECKER      Use TLAB for StringBuilder.append() allocations
//! RUN          options: "--no-async-jit=false --compiler-enable-jit=true --compiler-enable-tlab-events=true --compiler-check-final=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testStringBuilderAppend"
//! EVENT        /TlabAlloc,.*,24/
//! EVENT_NEXT   /TlabAlloc,.*,32/
//! EVENT_NEXT   /TlabAlloc,.*,48/
//! EVENT_NEXT   /TlabAlloc,.*,24/
//! EVENT_NEXT   /TlabAlloc,.*,48/

//! CHECKER      Use TLAB for StringBuilder.append() allocations AOT
//! RUN_PAOC     options: "--compiler-enable-tlab-events=true --compiler-regex=.*StringBuilderAppend.*"
//! RUN          options: "--compiler-enable-tlab-events=true", env: "LD_LIBRARY_PATH=../../../../../../lib", entry: "string_tlab_allocations.ETSGLOBAL::testStringBuilderAppend"
//! EVENT        /TlabAlloc,.*,24/
//! EVENT_NEXT   /TlabAlloc,.*,32/
//! EVENT_NEXT   /TlabAlloc,.*,48/
//! EVENT_NEXT   /TlabAlloc,.*,24/
//! EVENT_NEXT   /TlabAlloc,.*,48/

native function compileMethod(str: String) : int;
const UChars: char[] = [
    c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',
    c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0',c'\u00b0'
];

const UBytes: byte[] = [
    176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte,
    176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte, 176 as byte
];

const UCString: String =
    "\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0";

const UCString1: String =
    "\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0\u00b0";

const CChars: char[] = [
    c'0', c'0', c'0', c'0', c'0', c'0', c'0', c'0',
    c'0', c'0', c'0', c'0', c'0', c'0', c'0', c'0',
    c'0', c'0', c'0', c'0', c'0', c'0', c'0', c'0'
];

const CBytes: byte[] = [
    48, 48, 48, 48, 48, 48, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48
];

const CString: String  = "000000000000000000000000";
const CString1: String = "000000000000";
const CString2: String = "111111111111";
const CString3: String = "222222222222";
const CString4: String = "333333333333";
const Concat2String: String = "000000000000111111111111";
const Concat3String: String = "000000000000111111111111222222222222";
const Concat4String: String = "000000000000111111111111222222222222333333333333";

function CreateStringFromChars(): int {

    let str = new String(CChars);
    if (str != CString) {
        return 1;
    }
    let ustr = new String(UChars);
    if (ustr != UCString) {
        return 1;
    }

    return 0;
}

function testCreateStringFromChars() : int {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::CreateStringFromChars");
    return CreateStringFromChars();
}

function CreateStringFromString(): int {
    let str = new String(new String(CString1) + new String(CString1));
    if (str != CString) {
        return 1;
    }

    let ustr = new String(new String(UCString1) + new String(UCString1));
    if (ustr != UCString) {
        return 1;
    }

    return 0;
}

function StringSubstring(): int {
    let str = Concat4String.substring(0, 12);
    if (str != CString1) {
        return 1;
    }
    str = Concat4String.substring(0, 24);
    if (str != Concat2String) {
        return 1;
    }
    str = Concat4String.substring(0, 36);
    if (str != Concat3String) {
        return 1;
    }
    return 0;
}

function testStringSubstring() : int {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::StringSubstring");
    return StringSubstring();
}

function testCreateStringFromString() : int {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::CreateStringFromString");
    return CreateStringFromString();
}

function compareByteArrays(a: byte[], b: byte[]) {
    for (let i: int = 0; i < a.length; ++i) {
        if (a[i] != b[i]) {
            return 1;
        }
    }
    return 0;
}

function compareCharArrays(a: char[], b: char[]) {
    for (let i: int = 0; i < a.length; ++i) {
        if (a[i] != b[i]) {
            return 1;
        }
    }
    return 0;
}

function GetCharsFromString() : int {
    let chars1: char[] = CString.getChars(0, CString.getLength() - 1);
    if (compareCharArrays(chars1, CChars) != 0) {
        return 1;
    }
    let chars2: char[] = UCString.getChars(0, UCString.getLength() - 1);
    if (compareCharArrays(chars2, UChars) != 0) {
        return 1;
    }
    return 0;
}

function testGetCharsFromString() {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::GetCharsFromString");
    return GetCharsFromString();
}

function GetBytesFromString() : int {
    let bytes: byte[] = CString.getBytes(0, CString.getLength() - 1);
    if (compareByteArrays(bytes, CBytes) != 0) {
        return 1;
    }

    let ubytes: byte[] = UCString.getBytes(0, UCString.getLength() - 1);
    if (compareByteArrays(ubytes, UBytes) != 0) {
        return 2;
    }
    return 0;
}

function testGetBytesFromString() : int {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::GetBytesFromString");
    return GetBytesFromString();
}

function StringConcat() : int {
    let str: String;
    str = CString1.concat(CString2);
    if (str != Concat2String) {
        return 1;
    }
    str = CString1.concat(CString2, CString3);
    if (str != Concat3String) {
        return 2;
    }

    str = CString1.concat(CString2, CString3, CString4);
    if (str != Concat4String) {
        return 3;
    }

    return 0;
}

function testStringConcat() : int {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::StringConcat");
    return StringConcat();
}

function StringBuilderAppend(): int {
    let sb: StringBuilder = new StringBuilder();
    sb.append(0xffffffffff);
    sb.append(true);
    sb.append('C');
    sb.append("A string");
    console.println(sb.toString())
    return 0;
}

function testStringBuilderAppend() : int {
    loadLibrary("jitinterface");
    compileMethod("string_tlab_allocations.ETSGLOBAL::StringBuilderAppend");
    return StringBuilderAppend();
}
