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
class ClassA {
}

class ClassB {
}

class ClassC {
    public method1(v1: int, object: ClassA): ClassB {
        return new ClassB();
    }

    public method2(v1: int, object: ClassA | string): ClassB | string {
        if (object instanceof ClassA) {
            return new ClassB();
        } else {
            return 'hello';
        }
    }

    public method3(v1: int, aArray: ClassA[]): ClassB {
        return new ClassB();
    }
}
