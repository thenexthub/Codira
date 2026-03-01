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

{% for case in cases %}

/*---
desc: Annotations can be applied to the following
        Top-level declarations;
        Class members or interface members;
        Type usage;
        Parameters;
        A lambda expression;
        Local declarations.
        Namespace
        Functions
tags: [compile-only]
---*/

{{ case.decl }}

@Retention("SOURCE")
@interface SomeAnnotation {
   field1: {{ case.type1 }} = {{ case.value1 }}
   field2: {{ case.type2 }} = {{ case.value2 }}
}

@Retention("SOURCE")
@interface Optional {
}

@SomeAnnotation({ field2: {{ case.value2 }} })
const c: string = "some_string"

@SomeAnnotation({field1: {{ case.value1 }} })
let l: number = 1

type SomeType<T> = FixedArray<T>

@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
const t: SomeType<number> = [100, 200]

let x = @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} }) () => { return 1 }

interface IReceiver {}
class Receiver implements IReceiver {}

let lrc = @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} }) (this: Receiver) => { return 1 }

@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
function foo(@SomeAnnotation a: int, @Optional b?: string) {
    return a;
}

@Retention("SOURCE")
@interface ReceiverFunctionAnnotation {}

@Retention("SOURCE")
@interface ReceiverParamAnnotation {}

@ReceiverFunctionAnnotation
function receiverFunction0(@ReceiverParamAnnotation this: IReceiver) {
}

@ReceiverFunctionAnnotation
function receiverFunction1(@ReceiverParamAnnotation this: Receiver) {
}

@Retention("SOURCE")
@interface NamespaceAnnotation {}

@NamespaceAnnotation
namespace annotatedNS {
}

@interface LocalFieldAnnotation {}

function goo(@Optional b: string = "Optional value") {
    let lvar: String = "Local variable with annotation in function"
}

@SomeAnnotation
class MyClass {

    @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
    constructor (p: number) {
    }

    @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
    field: int = 1

    @SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
    bar(@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} }) a: int, @Optional b?: string) {
        return a
    }

    foo() {
        let lvar: String = "Local variable with annotation in method"
    }
}

@SomeAnnotation
abstract class MyAbstractClass {
}

@SomeAnnotation({field1: {{ case.value1 }}, field2: {{ case.value2 }} })
interface MyInterface {}

{% endfor %}
