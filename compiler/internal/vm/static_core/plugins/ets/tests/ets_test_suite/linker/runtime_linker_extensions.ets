/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class A { }

class DummyRuntimeLinker extends RuntimeLinker {
    private parent: RuntimeLinker | undefined = undefined;
    private placeholderClass: Class;

    public constructor(placeholderClass: Class, parent: RuntimeLinker | undefined = undefined) {
        this.parent = parent;
        this.placeholderClass = placeholderClass;
    }

    public override findAndLoadClass(clsName: string, init: boolean): Class {
        if (this.parent) {
            const optClass = this.parent!.findAndLoadClass(clsName, init);
            if (optClass) {
                return optClass;
            }
        }
        return this.placeholderClass;
    }
}

class Test {
    static stdCoreObjectName = "std.core.Object";
    static stdCoreObjectWrongName1 = "std/core/Object";
    static stdCoreObjectWrongName2 = "Lstd/core/Object;";
    static stdCoreObjectWrongName3 = "Lstd.core.Object;";
    static stdCoreObjectWrongName4 = "[D";
    static additionalClass1Name = "additional1.AdditionalClass1";
    static additionalClass2Name = "additional2.AdditionalClass2";

    static testDummyLinker() {
        const placeholderClass = Class.of(new A());
        const testLoadObject = (runtimeLinker: RuntimeLinker) => {
            const cls = runtimeLinker.loadClass(Test.stdCoreObjectName);
            arktest.assertEQ(cls, placeholderClass, "dummy linker must always return class A");
        };

        const dummyLinker = new DummyRuntimeLinker(placeholderClass);
        testLoadObject(dummyLinker);

        const abcLinker = new AbcRuntimeLinker(dummyLinker, []);
        testLoadObject(abcLinker);
    }

    private static testEmptyLinker(linker: RuntimeLinker) {
        const cls1 = linker.loadClass(Test.stdCoreObjectName);
        arktest.assertEQ(cls1, Class.of(new Object()));
        // Object must be returned from BootRuntimeLinker
        arktest.assertEQ(cls1.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker");
        const cls2 = getBootRuntimeLinker().loadClass(Test.stdCoreObjectName);
        arktest.assertEQ(cls1, cls2, "loadClass must be idempotent");
    }

    static testEmptyAbcRuntimeLinker() {
        const linker = new AbcRuntimeLinker(undefined, []);
        Test.testEmptyLinker(linker);
    }

    static testEmptyMemoryRuntimeLinker() {
        const linker = new MemoryRuntimeLinker(undefined, []);
        Test.testEmptyLinker(linker);
    }

    private static native loadFile(path: string): FixedArray<byte> | undefined;

    private static getAdditionalFilesList(): string[] {
        let procManager = new StdProcess.ProcessManager();
        const abcFiles = procManager.getEnvironmentVar("ADDITIONAL_ABC_FILES");
        arktest.assertLT(0, abcFiles.length, "this test must have additional abc files")
        return abcFiles.split(":");
    }

    private static loadRawFiles(paths: string[]): Array<FixedArray<byte>> {
        const rawFiles = new Array<FixedArray<byte>>(paths.length);
        for (let i = 0; i < rawFiles.length; i++) {
            const raw = Test.loadFile(paths[i]);
            arktest.assertNE(raw, undefined, "failed to load file " + paths[i]);
            rawFiles[i] = raw!;
        }
        return rawFiles;
    }

    private static testOnAdditionalFiles(linker: RuntimeLinker) {
        const objectClass = linker.loadClass(Test.stdCoreObjectName);
        arktest.assertEQ(objectClass, Class.of(new Object()));
        arktest.assertEQ(objectClass.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker");

        // Load class from "additional1.ets"
        const cls1 = linker.loadClass(Test.additionalClass1Name, true);
        arktest.assertNE(cls1, undefined, "AdditionalClass1 must be found");
        arktest.assertEQ(cls1.getLinker(), linker, "AdditionalClass1 must be loaded by custom linker");

        // Load class from "additional2.ets"
        // As side effect, this must load "additional3.ets" and execute code from it
        const cls2 = linker.loadClass(Test.additionalClass2Name, true);
        arktest.assertNE(cls2, undefined, "AdditionalClass2 must be found");
        arktest.assertEQ(cls2.getLinker(), linker, "AdditionalClass2 must be loaded by custom linker");

        // Check that RuntimeLinker loads into an isolated context
        const abcLinker2 = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList());
        // Load firstly "AdditionalClass2", which must trigger load of "AdditionalClass1"
        const otherLinkerClass2 = abcLinker2.loadClass(Test.additionalClass2Name, true);
        arktest.assertNE(otherLinkerClass2, undefined);
        arktest.assertEQ(otherLinkerClass2.getLinker(), abcLinker2, "linker contexts must be isolated");
        arktest.assertNE(cls2, otherLinkerClass2, "classes from different contexts must not conflict");
        const otherLinkerClass1 = abcLinker2.loadClass(Test.additionalClass1Name, true);
        arktest.assertNE(otherLinkerClass1, undefined);
        arktest.assertEQ(otherLinkerClass1.getLinker(), abcLinker2, "linker contexts must be isolated");
        arktest.assertNE(cls1, otherLinkerClass1, "classes from different contexts must not conflict");
    }

    static testAbcRuntimeLinker() {
        const linker = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList());
        Test.testOnAdditionalFiles(linker);
    }

    static testMemoryRuntimeLinker() {
        const linker = new MemoryRuntimeLinker(undefined, Test.loadRawFiles(Test.getAdditionalFilesList()));
        Test.testOnAdditionalFiles(linker);
    }

    static testCustomAbcRuntimeLinker() {
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = Test.getAdditionalFilesList().slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path, idx) => { abcFiles[idx.toInt()] = path; });
        // Create linkers
        const placeholderClass = Class.of(new A());
        const baseLinker = new AbcRuntimeLinker(undefined, abcFiles);
        const linker = new DummyRuntimeLinker(placeholderClass, baseLinker);

        // Load firstly "AdditionalClass2", which must trigger load of "AdditionalClass1"
        // - "A" must be loaded instead of "AdditionalClass1"
        const cls2 = linker.loadClass(Test.additionalClass2Name, true);
        arktest.assertNE(cls2, undefined);
        arktest.assertEQ(cls2.getLinker(), baseLinker);
        const cls1 = linker.loadClass(Test.additionalClass1Name, true);
        arktest.assertEQ(cls1, placeholderClass);
        arktest.assertEQ(cls1.getLinker(), getBootRuntimeLinker());
    }

    static testClassInitializationFail() {
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = Test.getAdditionalFilesList().slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path, idx) => { abcFiles[idx.toInt()] = path; });
        // Create linker
        const linker = new AbcRuntimeLinker(undefined, abcFiles);

        // Load "AdditionalClass2" without initialization
        const cls2 = linker.loadClass(Test.additionalClass2Name, false);
        arktest.assertNE(cls2, undefined);
        arktest.assertEQ(cls2.getLinker(), linker);
        // Trigger class initialization, which will not find "AdditionalClass1"
        arktest.expectThrow(() => { linker.loadClass(Test.additionalClass2Name, true); },
            (e) => e instanceof LinkerUnresolvedClassError
        );
    }

    private static triggerGC() {
        arktest.expectNoThrow((): void => {
            GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
        });
    }

    /*
     * Creates unreferenced `AbcRuntimeLinker` and returns `WeakRef` to it.
     *
     * Notes:
     * Do not inline this method, otherwise local reference to `AbcRuntimeLinker` will survive and the test will fail.
     */
    private static createUnreferencedLinker(): WeakRef<AbcRuntimeLinker> {
        const abcLinker = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList())

        const objectClass = abcLinker.loadClass(Test.stdCoreObjectName)
        arktest.assertEQ(objectClass, Class.of(new Object()))
        arktest.assertEQ(objectClass.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker")

        return new WeakRef<AbcRuntimeLinker>(abcLinker)
    }

    /*
     * Creates `AbcRuntimeLinker`, loads `Class` which belongs to the linker and returns weak reference to it.
     *
     * This function is used to check that `RuntimeLinker` is NOT collected by GC when a `Class` corresponding
     * to that linker is loaded. This is ensured by `Class` holding a reference to its `RuntimeLinker`.
     *
     * Notes:
     * Do not inline this method, otherwise local reference to `AbcRuntimeLinker` will survive and the test will fail.
     */
    private static createReferencedLinker(): [WeakRef<AbcRuntimeLinker>, Class] {
        const abcLinker = new AbcRuntimeLinker(undefined, Test.getAdditionalFilesList());

        const objectClass = abcLinker.loadClass(Test.stdCoreObjectName);
        arktest.assertEQ(objectClass, Class.of(new Object()));
        arktest.assertEQ(objectClass.getLinker(), getBootRuntimeLinker(), "std.core.Object must be loaded by BootRuntimeLinker");

        // Load class from "additional1.ets"
        const cls1 = abcLinker.loadClass(Test.additionalClass1Name, true);
        arktest.assertNE(cls1, undefined, "AdditionalClass1 must be found");
        arktest.assertEQ(cls1.getLinker(), abcLinker, "AdditionalClass1 must be loaded by custom linker");

        // Return `cls1` to ensure that `AbcRuntimeLinker` is referenced
        return [new WeakRef<AbcRuntimeLinker>(abcLinker), cls1];
    }

    static testAbcRuntimeLinkerUnload() {
        const unreferencedLinker = Test.createUnreferencedLinker()
        Test.triggerGC()
        arktest.assertEQ(unreferencedLinker.deref(), undefined, "Unreferenced AbcRuntimeLinker must be collected")

        const linkerAndClass = Test.createReferencedLinker()
        const referencedLinker = linkerAndClass[0]
        Test.triggerGC()
        arktest.assertNE(referencedLinker.deref(), undefined, "Referenced AbcRuntimeLinker must NOT be collected")
    }

    private static testAddAbcFiles(linker: AbcRuntimeLinker, newFiles: string[]) {
        // Load "AdditionalClass2" without initialization
        const cls2 = linker.loadClass(Test.additionalClass2Name, false);
        arktest.assertNE(cls2, undefined);
        arktest.assertEQ(cls2.getLinker(), linker);

        // Add the missing file
        linker.addAbcFiles(newFiles);

        // Trigger class initialization, which must not fail
        linker.loadClass(Test.additionalClass2Name, true);
    }

    static testRuntimeLinkerAddAbcFiles() {
        const allAbcFiles = Test.getAdditionalFilesList();
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = allAbcFiles.slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path, idx) => { abcFiles[idx.toInt()] = path; });
        // Create linker
        const linker = new AbcRuntimeLinker(undefined, abcFiles);
        Test.testAddAbcFiles(linker, [allAbcFiles[0]]);
    }

    static testMemoryRuntimeLinkerAddAbcFiles() {
        const allAbcFiles = Test.getAdditionalFilesList();
        // Cut out "additional1.ets" from the ABC files list
        const reducedAbcFiles = allAbcFiles.slice(1);
        const abcFiles = new string[reducedAbcFiles.length];
        reducedAbcFiles.forEach((path, idx) => { abcFiles[idx.toInt()] = path; });
        // Create linker
        const linker = new MemoryRuntimeLinker(undefined, Test.loadRawFiles(abcFiles));
        Test.testAddAbcFiles(linker, [allAbcFiles[0]]);
    }

    public static addSingleAbcFile(linker: AbcRuntimeLinker, path: string): void {
        linker.addAbcFiles([path]);
    }

    static testConcurrentAddAbcFiles() {
        const linker = new AbcRuntimeLinker(undefined, []);
        const allAbcFiles = Test.getAdditionalFilesList();
        const jobs = new Array<Job<void>>();
        // Add all files except "additional1.ets"
        for (let i = 1; i < allAbcFiles.length; i++) {
            jobs.push(launch<void, (l: AbcRuntimeLinker, p: string) => void>(Test.addSingleAbcFile, linker, allAbcFiles[i]));
        }
        // Try to load a class from "additional1.ets"
        arktest.expectThrow(() => { linker.loadClass(Test.additionalClass1Name, true); },
            (e) => e instanceof LinkerClassNotFoundError
        );
        jobs.forEach((p: Job<void>): void => { p.Await(); });
        Test.addSingleAbcFile(linker, allAbcFiles[0]);
        // Now class must be found
        arktest.expectNoThrow(() => { linker.loadClass(Test.additionalClass1Name, true); });
    }

    static testAbcRuntimeLinkerLoadErrorPath() {
        const path: string[] = ["null"];
        arktest.expectThrow(() => { new AbcRuntimeLinker(undefined, path) },
            (e) => e instanceof AbcFileNotFoundError
        );
    }

    static testWrongClassName() {
        const linker = new AbcRuntimeLinker(undefined, []);

        const errorCheck = (e: Error) => { return (e instanceof LinkerClassNotFoundError) }
        arktest.expectThrow(() => { linker.loadClass(Test.stdCoreObjectWrongName1) }, errorCheck);
        arktest.expectThrow(() => { linker.loadClass(Test.stdCoreObjectWrongName2) }, errorCheck);
        arktest.expectThrow(() => { linker.loadClass(Test.stdCoreObjectWrongName3) }, errorCheck);
        arktest.expectThrow(() => { linker.loadClass(Test.stdCoreObjectWrongName4) }, errorCheck);

        const memoryLinker = new MemoryRuntimeLinker(undefined, []);
        arktest.expectThrow(() => { memoryLinker.loadClass(Test.stdCoreObjectWrongName1) }, errorCheck);
        arktest.expectThrow(() => { memoryLinker.loadClass(Test.stdCoreObjectWrongName2) }, errorCheck);
        arktest.expectThrow(() => { memoryLinker.loadClass(Test.stdCoreObjectWrongName3) }, errorCheck);
        arktest.expectThrow(() => { memoryLinker.loadClass(Test.stdCoreObjectWrongName4) }, errorCheck);
    }

    static test() {
        Test.testDummyLinker();
        Test.testEmptyAbcRuntimeLinker();
        Test.testEmptyMemoryRuntimeLinker();
        Test.testAbcRuntimeLinker();
        Test.testMemoryRuntimeLinker();
        Test.testClassInitializationFail();
        Test.testAbcRuntimeLinkerUnload();
        Test.testAbcRuntimeLinkerLoadErrorPath();
        Test.testRuntimeLinkerAddAbcFiles();
        Test.testMemoryRuntimeLinkerAddAbcFiles();
        Test.testConcurrentAddAbcFiles();
        Test.testWrongClassName();
    }
}

function main() {
    loadLibrary("runtime_linker_extensions-shared-lib");
    Test.test();
}
