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

export namespace reflect.internals {

/**
 * Unstable API, for internal stdlib use only
 * contains various method for types manipulations, casts, checks etc.
 */
export final class Types {
    private readonly static BOXED_DOUBLE = Class.from<Double>()
    private readonly static BOXED_BOOLEAN = Class.from<Boolean>()
    private readonly static BOXED_INT = Class.from<Int>()
    private readonly static BOXED_LONG = Class.from<Long>()
    private readonly static BOXED_BYTE = Class.from<Byte>()
    private readonly static BOXED_SHORT = Class.from<Short>()
    private readonly static BOXED_FLOAT = Class.from<Float>()
    private readonly static BOXED_CHAR = Class.from<Char>()

    private constructor() {}

    public static identity_cast<T>(array: ArrayLike<T>): ArrayLike<T> {
        return array
    }

    public static asFixedArray<E>(obj: object): FixedArray<E> {
        return obj as FixedArray<E>
    }

    public static isAssignableFrom(lhsType: Class, rhsType: Class): boolean {
        return (rhsType.isSubtypeOf(lhsType))
            || (Types.isNumericType(lhsType) && Types.isNumericType(rhsType))
            || (Types.isBooleanType(lhsType) && Types.isBooleanType(rhsType))
            || (Types.isCharType(lhsType) && Types.isCharType(rhsType))
    }

    public static isDoubleType(type: Class): boolean {
        return type === Types.BOXED_DOUBLE || type === Class.PRIMITIVE_DOUBLE
    }

    public static isBooleanType(type: Class): boolean {
        return type === Types.BOXED_BOOLEAN || type === Class.PRIMITIVE_BOOLEAN
    }

    public static isIntType(type: Class): boolean {
        return type === Types.BOXED_INT || type === Class.PRIMITIVE_INT
    }

    public static isLongType(type: Class): boolean {
        return type === Types.BOXED_LONG || type === Class.PRIMITIVE_LONG
    }

    public static isByteType(type: Class): boolean {
        return type === Types.BOXED_BYTE || type === Class.PRIMITIVE_BYTE
    }

    public static isShortType(type: Class): boolean {
        return type === Types.BOXED_SHORT || type === Class.PRIMITIVE_SHORT
    }

    public static isFloatType(type: Class): boolean {
        return type === Types.BOXED_FLOAT || type === Class.PRIMITIVE_FLOAT
    }

    public static isCharType(type: Class): boolean {
        return type === Types.BOXED_CHAR || type === Class.PRIMITIVE_CHAR
    }

    public static isPrimitiveType(type: Class): boolean {
        return Types.isNumericType(type) || Types.isBooleanType(type)
    }

    public static isNumericType(type: Class): boolean {
        return Types.isDoubleType(type)
            || Types.isIntType(type)
            || Types.isLongType(type)
            || Types.isByteType(type)
            || Types.isShortType(type)
            || Types.isFloatType(type)
    }
}

export native function createFixedArray(component: Class, length: int): Object

} // namespace reflect.internals
