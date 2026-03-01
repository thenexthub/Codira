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

function testDecodeUriSpecialChars(): void {
    let code1 = decodeURI("%23%24%26%2B%2C%2F%3A%3B%3D%3F%40")
    arktest.assertEQ(code1, "%23%24%26%2B%2C%2F%3A%3B%3D%3F%40")
    let code2 = decodeURI("%23%24%26%2b%2c%2f%3a%3b%3d%3f%40")
    arktest.assertEQ(code2, "%23%24%26%2b%2c%2f%3a%3b%3d%3f%40")
    let code3 = decodeURI("%23%24%26%2b%2C%2F%3a%3b%3D%3F%40")
    arktest.assertEQ(code3, "%23%24%26%2b%2C%2F%3a%3b%3D%3F%40")
}

function testDecodeUri(): void {
    let s1 = decodeURI("Hello%20World")
    arktest.assertEQ(s1, "Hello World")
    let s2 = decodeURI("Test%21")
    arktest.assertEQ(s2, "Test!")
}

function testDecodeUriNotAscii(): void {
    let ss1 = decodeURI("%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82")
    arktest.assertEQ(ss1, "Привет")
    let ss2 = decodeURI("%E4%BD%A0%E5%A5%BD")
    arktest.assertEQ(ss2, "你好")
}

function testDecodeUriBoundary(): void {
    arktest.assertEQ(decodeURI("%00"), "\x00")
    arktest.assertEQ(decodeURI("%7F"), "\x7F")
    arktest.assertEQ(decodeURI("%C2%80"), "\x80")
    arktest.assertEQ(decodeURI("%DF%BF"), "\u07FF")
    arktest.assertEQ(decodeURI("%E0%A0%80"), "\u0800")
    arktest.assertEQ(decodeURI("%EF%BF%BF"), "\uFFFF")
}

function main() {
    testDecodeUriSpecialChars();
    testDecodeUri();
    testDecodeUriNotAscii();
    testDecodeUriBoundary();
}

