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
@interface MyAnno1 {}

namespace Ns1 {
    namespace Ns2 {
    }

    function foo2(a: int, b: int) {
    }

    const m2 = 1;

    interface Interface2 {
        iField2: string;

        iMethod2();
    }

    @interface MyAnno2 {}

    @MyAnno1()
    @MyAnno2()
    class ClassB implements Interface2{
        @MyAnno2()
        static sField2 = 0;

        @MyAnno2()
        field2 = '';
        iField2 = '';

        static sMethod2(a: int){
        }

        @MyAnno2()
        method2() {
        }

        iMethod2() {
        }
    }

    enum Color2 {
        YELLOW,
    }
}