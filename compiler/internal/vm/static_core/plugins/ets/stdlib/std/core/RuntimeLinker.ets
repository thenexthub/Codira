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

package std.core;

/**
 * @class RuntimeLinker base class.
 * All classes deriving from RuntimeLinker must implement findClass.
 */
export abstract class RuntimeLinker {
    private classLinkerCtxPtr: long

    protected constructor() {
        this.initializeContext()
    }

    /**
     * Searches the already loaded class in this linker's context.
     *
     * @param clsName name of the class.
     *
     * @returns Class if it was found, undefined otherwise.
     */
    public final native findLoadedClass(clsName: string): Class | undefined

    /**
     * Loads class into this linker's context.
     *
     * @param clsName name of the class.
     *
     * @param init indicator whether class must be initialized.
     *
     * @returns The found class.
     *
     * @throws LinkerClassNotFoundError if the class was not found or errors happened during class initialization.
     */
    public final loadClass(clsName: string, init: boolean = false): Class {
        const optClass = this.loadClassSafe(clsName, init)
        if (optClass) {
            return optClass
        }
        throw new LinkerClassNotFoundError(clsName)
    }

    public final getType(td: TypeDescriptor): Type | undefined {
        // Load class with lazy initialization
        const optClass = this.loadClassSafe(td, false)
        if (optClass === undefined) {
            return undefined
        }
        return Type.resolve(optClass.getDescriptor(), this)
    }

    /**
     * Find not yet loaded class and load it into this linker.
     *
     * @param clsName name of the class.
     *
     * @param init indicator whether class must be initialized.
     *
     * @returns Class if it was found, undefined otherwise.
     *
     * @throws errors happened during class initialization.
     */
    protected abstract findAndLoadClass(clsName: string, init: boolean): Class | undefined

    /**
     * Version of loadClass which does not throw LinkerClassNotFoundError.
     */
    protected final loadClassSafe(clsName: string, init: boolean): Class | undefined {
        let optClass = this.findLoadedClass(clsName)
        if (optClass) {
            if (init) {
                // No-op if class is already initialized
                optClass.initialize()
            }
            return optClass
        }
        return this.findAndLoadClass(clsName, init)
    }

    /**
     * Initialization of runtime internal structs.
     */
    private native initializeContext(): void
}

/**
 * Loads library from specified path.
 *
 * @param path to library to load
 */
export native function loadLibrary(path: String): void

/**
 * use the permission check hook when loadLibrary 
 * @param path to library to load
 * @param fileName The name of the file, for example: @ohos.bundle.d.ts
 */
export native function loadLibraryWithPermissionCheck(path: String, fileName: String): void
