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

package std.containers;

/**
 * A WeakSet is a collection of garbage-collectable values.  A value
 * in the WeakSet may only occur once. It is unique in the WeakSet's
 * collection.
 */
export final class WeakSet<K extends Object> {
    // NOTE(cheezzario) move creation to constructor
    private readonly elements: WeakMap<K, null> = new WeakMap<K, null>()

    toString(): String {
        return "[object WeakSet]";
    }

    /**
     * The WeakSet() constructor creates WeakSet objects.
     */
    constructor(elements?: K[] | null) {
        if (elements != null) {
            for (const element of elements) {
                this.add(element)
            }
        }
    }

    /**
     * The WeakSet() constructor creates WeakSet objects.
     */
    constructor(elements: Iterable<K>) {
        iteratorForEach<K>(elements.$_iterator(), (elem: K) => {
            this.add(elem)
        })
    }

    /**
     * The add() method appends a new object to the end of a WeakSet
     * object.
     */
    add(v: K): WeakSet<K>{
        this.elements.set(v, null)
        return this
    }

    /**
     * The has() method returns a boolean indicating whether an object
     * exists in a WeakSet or not.
     */
    has(v: K): boolean {
        return this.elements.has(v)
    }

    /**
     * The delete() method removes the specified element from
     * a WeakSet object.
     */
    delete(v: K): boolean {
        return this.elements.delete(v)
    }
}
