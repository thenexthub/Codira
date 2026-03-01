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
  * A WeakMap is a collection of key/value pairs whose keys must be objects,
  * with values of any arbitrary type, and which does not create strong references to its keys.
  * That is, an object's presence as a key in a WeakMap does not prevent the object from being garbage collected.
  * Once an object used as a key has been collected,
  * its corresponding values in any WeakMap become candidates for garbage collection as well
  */
export final class WeakMap<K extends Object, V> {
    private readonly keyRefToEntry = new Map<WeakRef<K>, WeakMapEntry<V>>
    private readonly keyHashToKeyRefs = new Map<Int, Array<WeakRef<K>>>()

    private readonly registry: FinalizationRegistry<WeakRef<K>>

    private static readonly KEY = 0
    private static readonly VAL = 1

    toString(): String {
        return "[object WeakMap]";
    }

    /**
     * The WeakMap() constructor creates WeakMap objects.
     */
    constructor(entries?: ([K, V])[] | null) {
        this.registry = new FinalizationRegistry<WeakRef<K>>((keyRef: WeakRef<K>) => {
            const entry = this.keyRefToEntry.get(keyRef)
            if (entry === undefined) {
                return
            }

            this.keyRefToEntry.delete(keyRef)

            const keyRefs = this.keyHashToKeyRefs.get(entry.keyHash)
            if (keyRefs !== undefined) {
                this.deleteKeyRef(keyRefs, keyRef, entry.keyHash)
            }
        })

        if (entries != null) {
            for (const entry of entries) {
                this.set(entry[0] as K, entry[1] as V)
            }
        }
    }

    /**
     * The WeakMap() constructor creates WeakMap objects.
     */
    constructor(entries: Iterable<[K, V]>) {
        this(null)

        iteratorForEach<[K, V]>(entries.$_iterator(), (entry: [K, V]) => {
            this.set(entry[0] as K, entry[1] as V)
        })
    }

    /**
     * The set() method adds a new element with a specified key
     * and value to a WeakMap object.
     *
     * @throws TypeError if key is not garbage-collectable
     */
    set(key: K, val: V): WeakMap<K, V> {
        if (!WeakMap.validateKey(key)) {
            throw new TypeError(`Invalid value used as weak map key: ${key}`)
        } 
        
        const keyHash = Runtime.getHashCode(key)

        const keyRefs = this.keyHashToKeyRefs.get(keyHash)
        if (keyRefs !== undefined) {
            const keyRef = this.findKeyRef(keyRefs, key)
            if (keyRef !== undefined) {
                this.keyRefToEntry.set(keyRef, new WeakMapEntry<V>(keyHash, val))
            } else {
                const newKeyRef = this.addEntry(key, keyHash, val)
                keyRefs.push(newKeyRef)
            }
        } else {
            const newKeyRef = this.addEntry(key, keyHash, val)

            const newKeyRefs = new Array<WeakRef<K>>(newKeyRef)
            this.keyHashToKeyRefs.set(keyHash, newKeyRefs)
        }

        return this
    }

    /**
     * The has() method returns a boolean indicating whether
     * an element with the specified key exists in the WeakMap
     * object or not.
     */
    has(key: K): boolean {
        const keyHash = Runtime.getHashCode(key)

        const keyRefs = this.keyHashToKeyRefs.get(keyHash)
        if (keyRefs === undefined) {
            return false
        }

        const keyRef = this.findKeyRef(keyRefs, key)
        if (keyRef === undefined) {
            return false
        }

        return this.keyRefToEntry.has(keyRef)
    }

    /**
     * The get() method returns a specified element from
     * a WeakMap object.
     */
    get(key: K): V | undefined {
        const keyHash = Runtime.getHashCode(key)

        const keyRefs = this.keyHashToKeyRefs.get(keyHash)
        if (keyRefs === undefined) {
            return undefined
        }

        const keyRef = this.findKeyRef(keyRefs, key)
        if (keyRef === undefined) {
            return undefined
        }

        const entry = this.keyRefToEntry.get(keyRef)
        if (entry === undefined) {
            return undefined
        }

        return entry.value
    }

    /**
     * The delete() method removes the specified element from
     * a WeakMap object.
     */
    delete(key: K): boolean {
        const keyHash = Runtime.getHashCode(key)

        const keyRefs = this.keyHashToKeyRefs.get(keyHash)
        if (keyRefs === undefined) {
            return false
        }

        const keyRef = this.findKeyRef(keyRefs, key)
        if (keyRef === undefined) {
            return false
        }

        // actual removal
        this.keyRefToEntry.delete(keyRef)

        this.deleteKeyRef(keyRefs, keyRef, keyHash)

        return true
    }

    private findKeyRef(keyRefs: ReadonlyArray<WeakRef<K>>, key: K): WeakRef<K> | undefined {
        return keyRefs.find((ref: WeakRef<K>) => key === ref.deref())
    }

    private addEntry(key: K, keyHash: int, val: V): WeakRef<K> {
        const newKeyRef = new WeakRef<K>(key)
        this.keyRefToEntry.set(newKeyRef, new WeakMapEntry<V>(keyHash, val))

        this.registry.register(key, newKeyRef)

        return newKeyRef
    }

    private deleteKeyRef(keyRefs: Array<WeakRef<K>>, keyRef: WeakRef<K>, keyHash: int): void {
        if (keyRefs.length == 0) {
            return
        }

        if (keyRefs.length == 1) {
            keyRefs.pop()
        } else {
            const keyRefPos = keyRefs.indexOf(keyRef).toInt()
            if (keyRefPos == -1) {
                return
            }

            if (keyRefPos != keyRefs.length - 1) {
                const lastRefPos = keyRefs.length - 1
                // overwriting found key ref with the last key ref in the list
                keyRefs[keyRefPos] = keyRefs[lastRefPos]
            }

            keyRefs.pop()
        }

        if (keyRefs.length == 0) {
            this.keyHashToKeyRefs.delete(keyHash)
        }
    }

    private native static validateKey(key: Object): boolean
}

class WeakMapEntry<V> {
    private readonly _keyHash: int
    private readonly _value: V

    constructor(keyHash: int, value: V) {
        this._keyHash = keyHash
        this._value = value
    }

    get keyHash(): int {
        return this._keyHash
    }

    get value(): V {
        return this._value
    }
}
