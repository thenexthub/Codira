/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
  files: [module2.ets, module3.ets]
  arktsconfig: arktsconfig.json
---*/

import * as foo from "@foo/foo"
import * as bar from "@bar/bar"
import * as module2 from "./module2"
import * as module3 from "./module3"
import { Ability } from "@spawn/ability";

export class Dummy { }

export function dummy(): Dummy { return new Dummy() }

class UnionHolder {
    public native bootUnionHolder(): Dummy | foo.Dummy | string
    public native appUnionHolder(): Dummy | UnionHolder
}

function testLazyLoading() {
    const appLinker = Class.current().getLinker();

    // disabled as runtime verification is enabled
    const verifierEnabled: boolean = true;

    if (!verifierEnabled) {
        arktest.assertEQ(appLinker.findLoadedClass("@bar.bar.ETSGLOBAL"), undefined, "module must be loaded lazily");
    }

    arktest.assertFalse(bar.ClassLoadingChecker_init, "`@bar.bar.ClassLoadingChecker` must be not initialized");
    arktest.assertNE(appLinker.findLoadedClass("@bar.bar.ETSGLOBAL"), undefined,
        "module must be loaded after accessing `bar.ClassLoadingChecker_init`");

    if (!verifierEnabled) {
        arktest.assertEQ(appLinker.findLoadedClass("@bar.bar.ClassLoadingChecker"), undefined,
            "`@bar.bar.ClassLoadingChecker` must not be loaded yet");
    }

    const instance = new bar.ClassLoadingChecker();
    arktest.assertEQ(appLinker.findLoadedClass("@bar.bar.ClassLoadingChecker"), Class.of(instance));
    arktest.assertTrue(bar.ClassLoadingChecker_init, "`@bar.bar.ClassLoadingChecker` must be initialized");
}

function triggerGC() {
    // Check that garbage collection does not break user-defined RuntimeLinker
    arktest.expectNoThrow((): void => {
        const gcId = GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE);
        if(gcId > 0) {
        GC.waitForFinishGC(gcId);
    }
});
}

function getNameOfClass(o: Object): string {
    return Class.of(o).getName();
}

function getMethodByName(classType: ClassType, name: string): Method | undefined {
    const numOfMethods = classType.getMethodsNum();
    for (let i = 0; i < numOfMethods; i++) {
        const method = classType.getMethod(i);
        if (method.getName() == name) {
            return method;
        }
    }
    return undefined;
}

function testStdCoreType() {
    const obj = foo.dummy();
    arktest.assertTrue(obj instanceof foo.Dummy);
    const objName = getNameOfClass(obj);
    arktest.assertEQ(objName, "@foo.foo.Dummy");

    const objType = Type.of(obj);
    arktest.assertEQ(objType.getName(), objName, "names obtained via std.core.Type and std.core.Class must match");

    const classType = objType as ClassType;
    const objClass = Class.of(obj);
    arktest.assertEQ(classType.getName(), objClass.getName(), "Class objects must match");
    const classTypeFromLinker = objClass.getLinker().getType(objClass.getName());
    arktest.assertNE(classTypeFromLinker, undefined, "RuntimeLinker must correctly resolve class type '" + objClass.getName() + "'");
    arktest.assertTrue(classType.equals(classTypeFromLinker!), "RuntimeLinker must correctly resolve class type '" + objClass.getName() + "'");

    const method = getMethodByName(classType, "foo");
    arktest.assertNE(method, undefined, "method 'foo' must be in class @foo.foo.Dummy");
    arktest.assertTrue(method!.getType().hasReceiver());
    arktest.assertEQ((method!.getType().getReceiverType() as ClassType).getName(), classType.getName(), "method 'foo' must be declared in interface 'Base'");
}

class Handler extends proxy.DefaultProxyHandler<Map<string, string>> { };
function testStdCoreProxy() {
    const map = new Map<string, string>();

    const mapProxy = proxy.Proxy.create(map, new Handler());

    arktest.assertTrue(map === mapProxy, "std.core.Proxy must work correctly in application context");
}

function testArrayClass() {
    const arr: FixedArray<foo.Dummy> = new FixedArray<foo.Dummy>(0);
    const arrClass = Class.of(arr);
    arktest.assertNE(arrClass, undefined);

    const element = foo.dummy();
    const elementClass = Class.of(element);
    arktest.assertNE(elementClass, undefined);
    arktest.assertEQ(arrClass.getLinker(), elementClass.getLinker());
}

function testUnionClass() {
    const holder = Class.of(new UnionHolder())

    // May rewrite it with `Class.from<Dummy | foo.Dummy | string>` after implementation of the latter
    const bootUnionClass = holder.getInstanceMethod("bootUnionHolder")!.getReturnType()
    // Union is loaded into boot because it contains `std.core.String` as a constituent type
    arktest.assertEQ(bootUnionClass.getLinker(), getBootRuntimeLinker())

    // May rewrite it with `Class.from<Dummy | UnionHolder>` after implementation of the latter
    const appUnionClass = holder.getInstanceMethod("appUnionHolder")!.getReturnType()
    const appLinker = Class.of(dummy()).getLinker()
    arktest.assertEQ(appUnionClass.getLinker(), appLinker)
}

function testNames() {
    triggerGC()

    arktest.assertEQ(Class.current().getName(), "@app.module1.ETSGLOBAL")

    testLazyLoading()

    triggerGC()

    arktest.assertTrue(bar.dummy() instanceof bar.Dummy)
    arktest.assertEQ(getNameOfClass(bar.dummy()), "@bar.bar.Dummy")

    arktest.assertTrue(foo.dummy() instanceof foo.Dummy)
    arktest.assertEQ(getNameOfClass(foo.dummy()), "@foo.foo.Dummy")

    arktest.assertTrue(dummy() instanceof Dummy)
    arktest.assertEQ(getNameOfClass(dummy()), "@app.module1.Dummy")

    arktest.assertTrue(module2.dummy() instanceof module2.Dummy)
    arktest.assertEQ(getNameOfClass(module2.dummy()), "@app.module2.Dummy")

    arktest.assertTrue(module3.dummy() instanceof module3.Dummy)
    arktest.assertEQ(getNameOfClass(module3.dummy()), "@app.module3.Dummy")

    testStdCoreType()
    testStdCoreProxy()

    testArrayClass()
    testUnionClass()

    triggerGC()
}

class MainAbility implements Ability {
    override invoke() {
        testNames();
    }
}
