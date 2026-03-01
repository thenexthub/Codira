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

package std.core;

/**
 * @class RuntimeLinker backed by in-memory ABC files.
 */
export class MemoryRuntimeLinker extends AbcRuntimeLinker {
    public constructor(parentLinker: RuntimeLinker | undefined, rawAbcFiles: Array<FixedArray<byte>>) {
        super(parentLinker, [])
        const abcFiles : FixedArray<AbcFile | undefined> = new FixedArray<AbcFile | undefined>(Double.toInt(rawAbcFiles.length))
        for (let i = 0; i < rawAbcFiles.length; i++) {
            abcFiles[i] = AbcFile.loadFromMemory(this, rawAbcFiles[i])
        }
        this.addAbcFiles(abcFiles)
    }
}
