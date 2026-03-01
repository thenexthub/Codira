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
    fld: int = 11

    public bar(): int {
        return this.fld
    }
}

function fooType(): ClassType {
    return Type.of(new Foo()) as ClassType
}

function main(): void {
    const fooClass = fooType()
    const creator = new ClassTypeCreator("MyClass").addBaseType(fooClass)
    arktest.assertEQ( fooClass.getConstructorsNum(), 1)
    const ctor = fooClass.getConstructor(0)
    creator.addMethod(
        new MethodCreator("constructor")
            .addConstructor()
            .addBody(new CallableBodyMethod(ctor))
    )
    let body: (x: Foo) => int = (x: Foo): int => { return x.fld + 30 }
    creator.addMethod(
        new MethodCreator("bar")
        .addResult(IntType.VAL)
        .addBody(new CallableBodyFunction(body as Object))
    )

    const cls = creator.create()

    const f = cls.make() as Foo
    arktest.assertEQ( f.bar(), 41)
}
