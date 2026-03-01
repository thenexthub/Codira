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

class Foo {
    public castInferType () {
        let x = () => {} as () => void
        return x
    }
    public castInferType1 () {
        let x = (a : Int) => { return 1 } as  (a : Int) => Int
        return x
    }
}
class ChildFoo extends Foo {
    public castNarrowing () {
        let a = (a : Foo) => { return a} as (a : ChildFoo) => ChildFoo
        return a
    }
    public castWidening () {
        let a = (a : ChildFoo) => { return a} as (a : Foo) => ChildFoo
        return a
    }
}

function main () {
    let foo = new Foo()
    let child = new ChildFoo()
    let x1 = foo.castInferType()
    arktest.assertTrue(x1() instanceof Int)

    let y = child.castNarrowing()
    arktest.assertTrue(y(child) instanceof ChildFoo)
    let y1 = child.castWidening()
    arktest.assertTrue(y1(foo) instanceof ChildFoo)
}
