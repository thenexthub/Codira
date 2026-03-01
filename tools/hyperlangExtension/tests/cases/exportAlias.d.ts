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
declare function f(): void;

// export statements may occur before definition
export {
    f as aliasedFunc,
    anotherFunc,
    MyInterface,
    I2 as ExportedInterface,
    MyClass,
    C1 as ExportedClass,
    MY_VALUE,
    V1 as EXPORTED_VALUE,
    MyStringEnum,
    E1 as MyNumericEnum,
}

declare function anotherFunc(): void;

interface MyInterface {}

interface I2 {}

export interface I3 {}

class MyClass {}

class C1 {}

const MY_VALUE = 123;

const V1 = "abc";

enum MyStringEnum { A = "aaa" }

enum E1 { A = 111 }
