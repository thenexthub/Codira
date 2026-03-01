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

class Parent {
    public booleanMethod(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string): boolean {
        return true;
    }

    public static staticBooleanMethod(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string): boolean {
        return false;
    }

    public static staticIntMethod(z: boolean, c: char, b: byte, s: short, i: int, l: long, f: float, d: double, r: string): int {
        return 24;
    }

    public static staticVoidParamMethod(): boolean {
        return true;
    }

    native bar(o: Object, m: long): void;
    native foo(): void;
    native baz(): void;
}

class Child extends Parent {
}

class Animal {
}
