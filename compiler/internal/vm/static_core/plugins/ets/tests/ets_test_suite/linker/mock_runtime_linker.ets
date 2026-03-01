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

class MockAbcRuntimeLinker extends AbcRuntimeLinker {
    private originalToMock: Map<string, string>

    constructor(paths: string[], originalToMock: Map<string, string>) {
        super(undefined, paths)
        this.originalToMock = originalToMock
    }

    protected final findAndLoadClass(clsName: string, init: boolean): Class | undefined {
        const mockName = this.originalToMock.get(clsName)
        return super.findAndLoadClass(mockName ?? clsName, init)
    }
}

function getAppFilesList(): string[] {
    let procManager = new StdProcess.ProcessManager();
    const abcFiles = procManager.getEnvironmentVar("APP_ABC_FILES");
    arktest.assertLT(0, abcFiles.length, "this test must have additional abc files")
    return abcFiles.split(":");
}

function main() {
    const mockMap = new Map<string, string>()
    mockMap.set("a.A", "mock_a.A")
    mockMap.set("a.ETSGLOBAL", "mock_a.ETSGLOBAL")
    const linker = new MockAbcRuntimeLinker(getAppFilesList(), mockMap)

    const cls = linker.loadClass("app.ETSGLOBAL")
    const result = cls.getStaticMethod("app")!.invoke()
    arktest.assertEQ(result, "mock class")
}
