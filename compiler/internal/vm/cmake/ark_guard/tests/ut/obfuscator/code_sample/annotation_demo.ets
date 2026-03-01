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

@Retention("RUNTIME")
@interface MyAnno {
    name: string
    attrs: number[]
}
@MyAnno({name: "someName", attrs: [1, 2]})
class A {}
let my: A = new A()


@interface ClassPreamble {
    authorName: string
    revision: number = 1
}
@interface Empty{}

@ClassPreamble({authorName: "John", revision: 2})
class C1 {/*body*/}
let c1 = new C1();

@ClassPreamble({authorName: "Bob"})
class C2 {/*body*/}
let c2 = new C2();

@Empty()
class C3 {/*body*/}
let c3 = new C3();

@interface deprecated{
    fromVersion: string
}

@deprecated("5.18")
function foo() {}
foo();

@deprecated({fromVersion: "5.18"})
function goo() {}
goo();

// SOURCE, “BYTECODE” “RUNTIME”
@Retention({policy: "SOURCE"})
@interface SrcAnno {}

@Retention({policy: "BYTECODE"})
@interface BtcAnno {}

@SrcAnno
@BtcAnno
class annoCls1 {}
let var1 = new annoCls1();

// short form:
@interface SpecialCall {/*some fields*/}
class specialCls {
    @SpecialCall
    name: string = '';

    @SpecialCall
    foo() {
        console.log('specialCls test ' + this.name);
    }
}

let cls1 = new specialCls();
cls1.name = 'test';
cls1.foo();

@interface ListAnnotation {
    element1: string[]
}

@ListAnnotation({element1: ['s1', 's2']})
class ListAnnotationClass {}