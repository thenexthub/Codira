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
 * @class ABC files based RuntimeLinker.
 */
export class AbcRuntimeLinker extends RuntimeLinker {
    private parentLinker: RuntimeLinker
    private abcFiles: FixedArray<AbcFile | undefined>
    private readonly sharedLibraryLoaders: FixedArray<AbcRuntimeLinker | undefined>

    public constructor(parentLinker: RuntimeLinker | undefined, paths: string[]) {
        super()
        this.parentLinker = parentLinker ?? getBootRuntimeLinker()
        this.abcFiles = new FixedArray<AbcFile | undefined>(paths.length)
        this.sharedLibraryLoaders = []
        for (let i = 0; i < paths.length; i++) {
            this.abcFiles[i] = AbcFile.loadAbcFile(this, paths[i])
        }
    }

    public constructor(parentLinker: RuntimeLinker | undefined, paths: string[], sharedLibraryLoaders: AbcRuntimeLinker[]) {
        super()
        this.parentLinker = parentLinker ?? getBootRuntimeLinker()
        this.abcFiles = new FixedArray<AbcFile | undefined>(paths.length)
        this.sharedLibraryLoaders = new FixedArray<AbcRuntimeLinker | undefined>(sharedLibraryLoaders.length)
        for (let i = 0; i < sharedLibraryLoaders.length; i++) {
            this.sharedLibraryLoaders[i] = sharedLibraryLoaders[i]
        }
        for (let i = 0; i < paths.length; i++) {
            this.abcFiles[i] = AbcFile.loadAbcFile(this, paths[i])
        }
    }

    /**
     * Adds new ABC files in the linker's context.
     *
     * @param paths to new ABC files.
     *
     * @throws errors happened on loading ABC file.
     */
    public addAbcFiles(paths: string[]): void {
        const newFiles : FixedArray<AbcFile | undefined> = new FixedArray<AbcFile | undefined>(paths.length)
        for (let i = 0; i < paths.length; i++) {
            newFiles[i] = AbcFile.loadAbcFile(this, paths[i])
        }
        this.addAbcFiles(newFiles)
    }

    /**
     * Find class in parent RuntimeLinker or within unloaded classes from ABC files.
     * The queried class could be already loaded - in this case it will be returned by parent.
     *
     * @param clsName name of the class.
     *
     * @param init indicator whether class must be initialized.
     *
     * @returns Class if it was found, undefined otherwise.
     *
     * @throws errors happened during class initialization.
     */
    protected override findAndLoadClass(clsName: string, init: boolean): Class | undefined {
        let optClass = this.parentLinker.loadClassSafe(clsName, init)
        if (optClass) {
            return optClass
        }
        for (let sharedLibraryLoader of this.sharedLibraryLoaders) {
            let sharedClass = sharedLibraryLoader!.loadClassSafe(clsName, init)
            if (sharedClass) {
                return sharedClass
            }
        }
        return this.loadClassFromAbcFiles(clsName, init)
    }

    /**
     * Adds new ABC files in the linker's context.
     *
     * @param newFiles ABC files loaded into current context.
     */
    protected final native addAbcFiles(newFiles: FixedArray<AbcFile | undefined>): void

    private native loadClassFromAbcFiles(clsName: string, init: boolean): Class | undefined
}
