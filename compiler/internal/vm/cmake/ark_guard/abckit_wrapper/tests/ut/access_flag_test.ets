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
abstract class AbstractClass {
}

final class FinalClass {
}

class ClassA {
    public static method0() {
    }

    public method1() {
    }

    protected method2() {
    }

    private method3() {
    }

    internal method4() {
    }
}

abstract class ClassB {
    public final method0() {
    }

    public abstract method1();

    public async method2() {
    }

    public native method3(): void;
}

class ClassC {
    public static field0: int;
    public field1: int;
    protected field2: int;
    private field3: int;
    internal field4: int;
}