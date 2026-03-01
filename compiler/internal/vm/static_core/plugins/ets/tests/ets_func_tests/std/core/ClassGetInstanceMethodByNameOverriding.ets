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

function checkExists(holder: Class, name: String, signature?: FixedArray<Class>) {
    arktest.assertNE(holder.getInstanceMethod(name, signature), undefined)
}

function checkError(holder: Class, name: String,  signature?: FixedArray<Class>) {
    arktest.expectError(() => {holder.getInstanceMethod(name, signature)})
}

function checkNotExists(holder: Class, name: String, signature?: FixedArray<Class>) {
    arktest.assertEQ(holder.getInstanceMethod(name, signature), undefined)
}

function checkReturnType(holder: Class, name: String, returnType: Class, signature?: FixedArray<Class>) {
    arktest.assertEQ(holder.getInstanceMethod(name, signature)!.getReturnType(), returnType)
}

namespace Basic {
    export class C {}

    export class Base {
        public methodFromBaseButOverloadedInDerived(): void {}
        public methodFromBase(): void {}
        public methodFromBase(i: C): void {}
        public methodFromBaseButOverriddenInDerived(b: Base) {}
    }

    export class Derived extends Base {
        public methodFromBaseButOverloadedInDerived(i: C) {}
        public methodFromDerived(): void {}
        public methodFromDerived(i: C): void {}
        public methodFromBaseButOverriddenInDerived(b: Derived) {}
    }
}

function testBasicOverriding() {
    let bas = Class.from<Basic.Base>()
    let der = Class.from<Basic.Derived>()
    let baseArray: FixedArray<Class> = [bas]
    let derivedArray: FixedArray<Class> = [der]
    let cArray: FixedArray<Class> = [Class.from<Basic.C>()]

    checkExists(bas, "methodFromBaseButOverloadedInDerived")
    checkExists(bas, "methodFromBaseButOverloadedInDerived", [])
    checkNotExists(bas, "methodFromBaseButOverloadedInDerived", cArray)
    checkError(bas, "methodFromBase")
    checkExists(bas, "methodFromBase", [])
    checkExists(bas, "methodFromBase", cArray)
    checkNotExists(bas, "methodFromBase", baseArray)
    checkExists(bas, "methodFromBaseButOverriddenInDerived")
    checkExists(bas, "methodFromBaseButOverriddenInDerived", baseArray)
    checkNotExists(bas, "methodFromBaseButOverriddenInDerived", [])
    checkNotExists(bas, "methodFromDerived")
    checkNotExists(bas, "methodFromDerived", cArray)

    checkExists(der, "methodFromBaseButOverloadedInDerived")
    checkNotExists(der, "methodFromBaseButOverloadedInDerived", [])
    checkExists(der, "methodFromBaseButOverloadedInDerived", cArray)
    checkError(der, "methodFromDerived")
    checkExists(der, "methodFromDerived", [])
    checkExists(der, "methodFromDerived", cArray)
    checkNotExists(der, "methodFromDerived", baseArray)
    checkExists(der, "methodFromBaseButOverriddenInDerived")
    checkExists(der, "methodFromBaseButOverriddenInDerived", derivedArray)
    checkNotExists(der, "methodFromBaseButOverriddenInDerived", baseArray)
    checkNotExists(der, "methodFromBaseButOverriddenInDerived", [])
    checkNotExists(der, "methodFromBase")
    checkNotExists(der, "methodFromBase", cArray)
}

namespace Erasure {
    export class Base {
        public undefinedMethod(): undefined {
            return undefined
        }

        public baseUndefinedMethod(): undefined {
            return undefined
        }
    }

    export class Derived extends Base {
        public undefinedMethod(): undefined {
            return undefined
        }
    }
}

function checkErasure() {
    let bas = Class.from<Erasure.Base>()
    let der = Class.from<Erasure.Derived>()
    let und = Class.from<Object>()

    checkExists(bas, "undefinedMethod")
    checkExists(der, "undefinedMethod")
    checkExists(bas, "baseUndefinedMethod")
    checkNotExists(der, "baseUndefinedMethod")
    checkExists(bas, "undefinedMethod", [])
    checkExists(der, "undefinedMethod", [])
    checkExists(bas, "baseUndefinedMethod", [])
    checkNotExists(der, "baseUndefinedMethod", [])
    checkReturnType(bas, "undefinedMethod", und)
    checkReturnType(der, "undefinedMethod", und)
    checkReturnType(bas, "baseUndefinedMethod", und)
    checkReturnType(bas, "undefinedMethod", und, [])
    checkReturnType(der, "undefinedMethod", und, [])
    checkReturnType(bas, "baseUndefinedMethod", und, [])
}

function main(): int {
    let ts = new arktest.ArkTestsuite("Basic overriding test")
    ts.addTest("testBasicOverriding", testBasicOverriding)
    ts.addTest("checkErasure", checkErasure)
    return ts.run()
}