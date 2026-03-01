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

package std.core;

export namespace proxy
{
export interface ReadonlyArrayProxyHandler<T> {
	get(target: ReadonlyArray<T>, name: string): Any

	get(target: ReadonlyArray<T>, index: int): T
}

export abstract class DefaultReadonlyArrayProxyHandler<T> implements ReadonlyArrayProxyHandler<T> {
	override get(target: ReadonlyArray<T>, name: string): Any {
		if (name == "length") {
			return target.length
		}

		throw new AssertionError("unexpected property: " + name)
	}

	override get(target: ReadonlyArray<T>, index: int): T {
		return target[index]
	}
}

class ReadonlyArrayProxy<T> implements ReadonlyArray<T> {
	private target: ReadonlyArray<T>
	private handler: ReadonlyArrayProxyHandler<T>

	constructor(target: ReadonlyArray<T>, handler: ReadonlyArrayProxyHandler<T>) {
		this.target = target
		this.handler = handler
	}

    override concat(...items: FixedArray<ConcatArray<T>>): Array<T> {
    	return this.target.concat(...items)
    }

    override entries(): IterableIterator<[number, T]> {
    	return this.target.entries()
    }

    override every(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean {
    	return this.target.every(predicate)
    }

    override filter(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): Array<T> {
    	return this.target.filter(predicate)
    }

    override find(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined {
    	return this.target.find(predicate)
    }

    override findLast(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): T | undefined {
    	return this.target.findLast(predicate)
    }

    override findIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int {
    	return this.target.findIndex(predicate)
    }

    override findLastIndex(predicate: (value: T, index: int, obj: ReadonlyArray<T>) => boolean): int {
    	return this.target.findLastIndex(predicate)
    }

    override forEach(action: (value: T, index: int, array: ReadonlyArray<T>) => void): void {
    	return this.target.forEach(action)
    }

    override join(separator?: string): string {
    	return this.target.join(separator)
    }

    override includes(searchElement: T, fromIndex?: int): boolean {
    	return this.target.includes(searchElement, fromIndex)
    }

    override indexOf(searchElement: T, fromIndex?: int): int {
    	return this.target.indexOf(searchElement, fromIndex)
    }

    override keys(): IterableIterator<number> {
    	return this.target.keys()
    }

    override lastIndexOf(searchElement: T): int {
        return this.target.lastIndexOf(searchElement)
    }

    override lastIndexOf(searchElement: T, fromIndex?: int): int {
    	return this.target.lastIndexOf(searchElement, fromIndex)
    }

    override map<U>(mapper: (value: T, index: int, array: ReadonlyArray<T>) => U): Array<U> {
    	return this.target.map<U>(mapper)
    }

    override reduce(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => T): T {
    	return this.target.reduce(reducer)
    }

    override reduce<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U {
    	return this.target.reduce<U>(reducer, initialValue)
    }

    override reduceRight(reducer: (previousValue: T, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => T): T {
    	return this.target.reduceRight(reducer)
    }

    override reduceRight<U>(reducer: (previousValue: U, currentValue: T, currentIndex: int, array: ReadonlyArray<T>) => U, initialValue: U): U {
    	return this.target.reduceRight<U>(reducer, initialValue)
    }

    override slice(start?: int, end?: int): Array<T> {
    	return this.target.slice(start, end)
    }

    override some(predicate: (value: T, index: int, array: ReadonlyArray<T>) => boolean): boolean {
    	return this.target.some(predicate)
    }

    override toLocaleString(): string {
    	return this.target.toLocaleString()
    }

    override toString(): string {
    	return this.target.toString()
    }

    override values(): IterableIterator<T> {
    	return this.target.values()
    }

    override at(index: int): T | undefined {
    	return this.target.at(index)
    }

    override $_get(index: int): T {
    	return this.handler.get(this.target, index)
    }

    get length(): int {
    	const length = this.handler.get(this.target, "length")
    	if (length instanceof Int) {
    		return length
    	}

        throw new TypeError("unexpected ReadonlyArray.length value type: " + Type.of(length))
    }
}
}
