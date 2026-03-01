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

export namespace reflect {

/**
 * Represents constructor of class
 */
export final class Constructor extends Method {
    private constructor() { throw new Error("Constructor constructor called") }

    public equals(other: Constructor): boolean {
        return this.methodPtr == other.methodPtr
    }

    /**
     * Creates an instance of owner class
     *
     * @param args consructor parameters
     *
     * @returns Created instance.
     *
     * @throws OutofMemoryError if there is no memory to allocate new instance.
     *
     * @throws TypeError if there is mismatch in arguments number or types
     */
    public createInstance(args?: FixedArray<Any>): Any {
        return this.createInstanceInternal(args);
    }

    private native createInstanceInternal(args?: FixedArray<Any>): Any
}

} // namespace reflect
