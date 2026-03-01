/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

export final class Runtime {

    /**
     * Returns a hash code for the Object.
     *
     * @param o to calculate hash code from
     *
     * @returns hash code
     */
    public static native getHashCode(o: Object): int;

    /**
     * Returns a hash code for the Object. The hash code algorithm  depends
     * on the actual type of the input parameter. Special algorithm is used for 
     * Byte, Short, Int, Long, Char, String. In other cases the result is same
     * as getHashCode.
     *
     * @param o to calculate hash code from
     *
     * @returns hash code
     */
    public static native getHashCodeByValue(o: Object): long;

    /**
     * Check if it is a little-endian platform
     *
     * @returns true - little-endian, false - otherwise
     */
    public native static isLittleEndianPlatform(): boolean;

    /**
     * Checks if two references point to a same object/value.
     *
     * @param o1 first object to compare
     *
     * @param o2 second object to compare
     *
     * @returns true if Objects are equal
     */
    public static native isSameReference(o1: Any, o2: Any): boolean;

    // Deprecated.
    native static __getTypeInfo(o: Any): Class|undefined;

    // Internal stubs to create a ClassCastError during failed cast events
    static native failedTypeCastExclUndefinedStub(source: Any, target: Class | undefined): ClassCastError
    static native failedTypeCastInclUndefinedStub(source: Any, target: Class | undefined): ClassCastError

    // Internal stub to create a FixedArray of the specified type and length
    private native static allocSameTypeArray<T extends FixedArray<Any>>(type: Class, length: int): T;

    // Internal stub to create a FixedArray with elements prepended in front of the original array
    static addFrontElementsStub(original: FixedArray<Any>, front: FixedArray<Any>): FixedArray<Any> {
        let res = Runtime.allocSameTypeArray<FixedArray<Any>>(Class.of(original), original.length + front.length)
        for (let i: int = 0; i < front.length; ++i) {
            res[i] = front[i]
        }
        for (let i: int = 0; i < original.length; ++i) {
            res[i + front.length] = original[i]
        }
        return res
    }

    // Internal stub to create a FixedArray with several elements removed from front of the original array
    static removeFrontElementsStub(original: FixedArray<Any>, count: int): FixedArray<Any> {
        let res = Runtime.allocSameTypeArray<FixedArray<Any>>(Class.of(original), original.length - count)
        for (let i: int = count; i < original.length; ++i) {
            res[i - count] = original[i]
        }
        return res
    }

    // Internal stub to pad rest arguments array to handle possible argument underapplication
    static restArgumentsUnderapplicationStub(rest: FixedArray<Any>, required: int): FixedArray<Any> {
        if (rest.length >= required) {
            return rest
        }
        let newRest : FixedArray<Any> = new FixedArray<Any>(required)
        for (let i: int = 0; i < rest.length; ++i) {
            newRest[i] = rest[i]
        }
        for (let i: int = rest.length; i < required; ++i) {
            newRest[i] = undefined
        }
        return newRest
    }

    // Internal stub to handle a generic case of unsafeCall for std.core.Lambda... helper classes
    static lambdaUnsafeCallStub(fn: Function, r: FixedArray<Any>): Any {
        return __lambdaUnsafeCallStub(fn, r)
    }
}

// Deprecated. Narrow the value to type T or to default (object | null | undefined).
export function __narrowAny<T = (object | null | undefined)>(v: Any): T {
    return v as T
}
