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

package escompat;

export interface Cloneable {
	clone(): Cloneable;
}

// deepcopy can be used to copy a instance deeply, like builtins types, class with 
// default constructor. If the instance is or contain a class without default 
// constructor, `Cloneable` can be used to help to implement copying deeply. At the
// same time, DeepCopy supports cloning of subclasses that inherit from the parent
// class provided that the clone method needs to be overridden and if the clone method
// is not overridden in the parent and child classes an error will be thrown at runtime

export function deepcopy<T>(src: T | null | undefined, depth: int = -1): T | null | undefined {
	const maxDepth = (depth == -1) ? Int.MAX_VALUE : depth
	return new DeepCloner(maxDepth).clone<T>(src)
}

type Types = reflect.internals.Types

type EntryType = [reflect.InstanceField, Any]
type NullishEntryType = EntryType | null | undefined

class DeepCloner {
	private maxDepth: int
	private currentDepth: int = 0

	private static readonly OBJ_ENTRY_FIELD: int = 0
	private static readonly OBJ_ENTRY_VALUE: int = 1

	private copies = new Map<Object, Object>()

	constructor(maxDepth: int) {
		this.maxDepth = maxDepth
	}

	clone<T>(src: T | null | undefined): T | null | undefined {
		if (src === null) {
			return null
		} else if (src === undefined) {
			return undefined
		} else if (src instanceof String) {
			// strings are immutable, cloning reference is enough here
			// because source string instance can't be changed through the cloned reference
			return src as T
		} else if (src instanceof Boolean) {
			return Boolean.valueOf(src) as T
		} else if (src instanceof Byte) {
			return Byte.valueOf(src) as T
		} else if (src instanceof Char) {
			return Char.valueOf(src) as T
		} else if (src instanceof Short) {
			return Short.valueOf(src) as T
		} else if (src instanceof Int) {
			return Int.valueOf(src) as T
		} else if (src instanceof Long) {
			return Long.valueOf(src) as T
		} else if (src instanceof Float) {
			return Float.valueOf(src) as T
		} else if (src instanceof Double) {
			return Double.valueOf(src) as T
		} else if (src instanceof BaseEnum) {
			return src as T
		} else if (src instanceof Date) {
			return new Date(src) as T
		} else {
			const existingCopy = this.copies.get(src as Object)
			if (existingCopy !== undefined) {
				return existingCopy as T
			}

			if (src instanceof Array) {
				return this.cloneDynamicArray(src) as T
			} else if (src instanceof Set) {
				return this.cloneSet(src) as T
			} else if (src instanceof Map) {
				return this.cloneMap(src) as T
			} else if (src instanceof Function) {
				return src as T
			} else if (src instanceof Object) {
				const srcType = Class.of(src)
				if (srcType.isFixedArray()) {
					if (src instanceof FixedArray<boolean>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<byte>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<char>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<short>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<int>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<long>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<float>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else if (src instanceof FixedArray<double>) {
						return ((this.currentDepth + 1 > this.maxDepth) ? src : copyOf(src)) as T
					} else {
						const componentType = srcType.getFixedArrayComponentType()!
						return this.cloneFixedArray(Types.asFixedArray(src), componentType) as T
					}
				} else {
					return this.cloneObject(src, srcType) as T
				}
			} else {
				throw new Error("unsupported source object type: " + Class.ofAny(src)?.getName())
			}
		}
	}

	private getEmptyConstructor(type: Class): reflect.Constructor | undefined {
		for (const ctor of type.getConstructors()) {
			if (ctor.getParametersNum() == 0) {
				return ctor
			}
		}
		return undefined
	}

	private hasOverrideClone(type: Class): boolean {
		for (const method of type.getInstanceMethods()) {
			if (method.getName() == "clone") {
				return true
			}
		}
		return false
	}

	private getObjectEntries(obj: Object): NullishEntryType[] {
		const entries = new Array<NullishEntryType>()

		let cls = Class.of(obj)
	    const classHierarchy = new Array<Class>()
	    while (cls.getSuper() !== undefined) {
	        classHierarchy.push(cls)
	        cls = cls.getSuper()!
	    }

	    const classHierarchyLength = classHierarchy.length
	    for (let i = classHierarchyLength - 1; i >= 0; i--) {
	        const currentCls = classHierarchy[i]
	        const clsFields = currentCls.getInstanceFields()
	        for (const field of clsFields) {
	            const entry: NullishEntryType = [field, field.getValue(obj)]
	            entries.push(entry)
	        }
	    }

		return entries
	}

	private cloneObject(obj: Object, objType: Class): Object {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return obj
		}

		if (obj instanceof Cloneable) {
			if (this.hasOverrideClone(objType)) {
				let objClone = obj as Cloneable
				return objClone.clone()
			}
			throw new Error(`class ${objType.getName()} doesn't override clone`)
		}

		const emptyCtor = this.getEmptyConstructor(objType)
		if (emptyCtor === undefined) {
			throw new Error(`class ${objType.getName()} doesn't have default constructor`)
		}

		// this won't work for strings due to string special constructor mechanics
		// but string instance cloning is not handled here
		const objCopy = emptyCtor.createInstance() as object
		this.copies.set(obj, objCopy)

		const objEntries = this.getObjectEntries(obj)
		for (const objEntry of objEntries) {
			const field = objEntry![DeepCloner.OBJ_ENTRY_FIELD]
			const fieldValCopy = this.clone<Object>(objEntry![DeepCloner.OBJ_ENTRY_VALUE] as (Object | undefined))
			const fieldValCopyType = Class.ofAny(fieldValCopy)

			if (fieldValCopyType !== undefined && reflect.internals.Types.isAssignableFrom(field.getType(), fieldValCopyType)) {
				field.setValue(objCopy, fieldValCopy)
			}
		}

		this.currentDepth--

		return objCopy
	}

	private cloneFixedArray<E>(array: FixedArray<E>, componentType: Class): FixedArray<E> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return array
		}

		const arrayLength = array.length

		const arrayCopy = reflect.internals.createFixedArray(componentType, arrayLength) as FixedArray<E>
		this.copies.set(array, arrayCopy)

		for (let i = 0; i < arrayLength; i++) {
			const arrayElementCopy = this.clone<E>(array[i]) as E
			arrayCopy[i] = arrayElementCopy
		}

		this.currentDepth--

		return arrayCopy
	}

	private cloneDynamicArray<E>(array: Array<E>): Array<E> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return array
		}

		const arrayCopy = new Array<E>()
		this.copies.set(array, arrayCopy)

		array.forEach((elem: E) => {
		 	const elemCopy = this.clone<E>(elem)
		 	arrayCopy.push(elemCopy as E)
		})

		this.currentDepth--

		return arrayCopy
	}

	private cloneSet<E>(set: Set<E>): Set<E> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return set
		}

		const setCopy = new Set<E>()
		this.copies.set(set, setCopy)

		set.forEach((elem: E) => {
			const elemCopy = this.clone<E>(elem)
			setCopy.add(elemCopy as E)
		})

		this.currentDepth--

		return setCopy
	}

	private cloneMap<K, V>(map: Map<K, V>): Map<K, V> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return map
		}

		const mapCopy = new Map<K, V>()
		this.copies.set(map, mapCopy)

		map.forEach((val: V, key: K) => {
			const keyCopy: K = this.clone<K>(key) as K
			const valCopy: V = this.clone<V>(val) as V

			mapCopy.set(keyCopy, valCopy)
		})

		this.currentDepth--

		return mapCopy
	}
}
