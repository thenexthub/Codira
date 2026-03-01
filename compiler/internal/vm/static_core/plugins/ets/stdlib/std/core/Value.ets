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

// Class Value

native function ValueAPIGetFieldBoolean(obj: Object, i: long): boolean

native function ValueAPIGetFieldByte(obj: Object, i: long): byte

native function ValueAPIGetFieldShort(obj: Object, i: long): short

native function ValueAPIGetFieldChar(obj: Object, i: long): char

native function ValueAPIGetFieldInt(obj: Object, i: long): int

native function ValueAPIGetFieldFloat(obj: Object, i: long): float

native function ValueAPIGetFieldDouble(obj: Object, i: long): double

native function ValueAPIGetFieldLong(obj: Object, i: long): long

native function ValueAPIGetFieldObject(obj: Object, i: long): Object


native function ValueAPIGetFieldByNameBoolean(obj: Object, name: String): boolean

native function ValueAPIGetFieldByNameByte(obj: Object, name: String): byte

native function ValueAPIGetFieldByNameShort(obj: Object, name: String): short

native function ValueAPIGetFieldByNameChar(obj: Object, name: String): char

native function ValueAPIGetFieldByNameInt(obj: Object, name: String): int

native function ValueAPIGetFieldByNameFloat(obj: Object, name: String): float

native function ValueAPIGetFieldByNameDouble(obj: Object, name: String): double

native function ValueAPIGetFieldByNameLong(obj: Object, name: String): long

native function ValueAPIGetFieldByNameObject(obj: Object, name: String): Object


native function ValueAPISetFieldBoolean(obj: Object, i: long, val: boolean): void

native function ValueAPISetFieldByte(obj: Object, i: long, val: byte): void

native function ValueAPISetFieldShort(obj: Object, i: long, val: short): void

native function ValueAPISetFieldChar(obj: Object, i: long, val: char): void

native function ValueAPISetFieldInt(obj: Object, i: long, val: int): void

native function ValueAPISetFieldFloat(obj: Object, i: long, val: float): void

native function ValueAPISetFieldDouble(obj: Object, i: long, val: double): void

native function ValueAPISetFieldLong(obj: Object, i: long, val: long): void

native function ValueAPISetFieldObject(obj: Object, i: long, val: Object): void


native function ValueAPISetFieldByNameBoolean(obj: Object, name: String, val: boolean): void

native function ValueAPISetFieldByNameByte(obj: Object, name: String, val: byte): void

native function ValueAPISetFieldByNameShort(obj: Object, name: String, val: short): void

native function ValueAPISetFieldByNameChar(obj: Object, name: String, val: char): void

native function ValueAPISetFieldByNameInt(obj: Object, name: String, val: int): void

native function ValueAPISetFieldByNameFloat(obj: Object, name: String, val: float): void

native function ValueAPISetFieldByNameDouble(obj: Object, name: String, val: double): void

native function ValueAPISetFieldByNameLong(obj: Object, name: String, val: long): void

native function ValueAPISetFieldByNameObject(obj: Object, name: String, val: Object): void

// Array Value
native function ValueAPIGetArrayLength(obj: Object): long

native function ValueAPISetElementBoolean(obj: Object, i: long, val: boolean): void

native function ValueAPISetElementByte(obj: Object, i: long, val: byte): void

native function ValueAPISetElementShort(obj: Object, i: long, val: short): void

native function ValueAPISetElementChar(obj: Object, i: long, val: char): void

native function ValueAPISetElementInt(obj: Object, i: long, val: int): void

native function ValueAPISetElementFloat(obj: Object, i: long, val: float): void

native function ValueAPISetElementDouble(obj: Object, i: long, val: double): void

native function ValueAPISetElementLong(obj: Object, i: long, val: long): void

native function ValueAPISetElementObject(obj: Object, i: long, val: Object): void


native function ValueAPIGetElementBoolean(obj: Object, i: long): boolean

native function ValueAPIGetElementByte(obj: Object, i: long): byte

native function ValueAPIGetElementShort(obj: Object, i: long): short

native function ValueAPIGetElementChar(obj: Object, i: long): char

native function ValueAPIGetElementInt(obj: Object, i: long): int

native function ValueAPIGetElementFloat(obj: Object, i: long): float

native function ValueAPIGetElementDouble(obj: Object, i: long): double

native function ValueAPIGetElementLong(obj: Object, i: long): long

native function ValueAPIGetElementObject(obj: Object, i: long): Object

export namespace reflect {

/**
 * Represents abstract value
 */
export abstract class Value extends Object {
    public static of(v: boolean): Value {
        return new BooleanValue(BooleanType.VAL, v)
    }

    public static of(v: byte): Value {
        return new ByteValue(ByteType.VAL, v)
    }

    public static of(v: char): Value {
        return new CharValue(CharType.VAL, v)
    }

    public static of(v: short): Value {
        return new ShortValue(ShortType.VAL, v)
    }

    public static of(v: int): Value {
        return new IntValue(IntType.VAL, v)
    }

    public static of(v: long): Value {
        return new LongValue(LongType.VAL, v)
    }

    public static of(v: float): Value {
        return new FloatValue(FloatType.VAL, v)
    }

    public static of(v: double): Value {
        return new DoubleValue(DoubleType.VAL, v)
    }

    // -----

    public static of(v: FixedArray<boolean>): Value {
        return new ArrayValue(ArrayType.BOOLEAN_VAL, v)
    }

    public static of(v: FixedArray<byte>): Value {
        return new ArrayValue(ArrayType.BYTE_VAL, v)
    }

    public static of(v: FixedArray<char>): Value {
        return new ArrayValue(ArrayType.CHAR_VAL, v)
    }

    public static of(v: FixedArray<short>): Value {
        return new ArrayValue(ArrayType.SHORT_VAL, v)
    }

    public static of(v: FixedArray<int>): Value {
        return new ArrayValue(ArrayType.INT_VAL, v)
    }

    public static of(v: FixedArray<long>): Value {
        return new ArrayValue(ArrayType.LONG_VAL, v)
    }

    public static of(v: FixedArray<float>): Value {
        return new ArrayValue(ArrayType.FLOAT_VAL, v)
    }

    public static of(v: FixedArray<double>): Value {
        return new ArrayValue(ArrayType.DOUBLE_VAL, v)
    }

    // -----

    /**
     * Returns value of this object
     *
     * @param o object instance
     *
     * @returns {@link Value} of this object
     */
    public static of(o: Any): Value {
        if (o instanceof Value) {
            return o as Value
        } else if (o instanceof Boolean) {
            return new BooleanValue(BooleanType.REF, o as Boolean)
        } else if (o instanceof Char) {
            return new CharValue(CharType.REF, o as Char)
        } else if (o instanceof Byte) {
            return new ByteValue(ByteType.REF, o as Byte)
        } else if (o instanceof Short) {
            return new ShortValue(ShortType.REF, o as Short)
        } else if (o instanceof Int) {
            return new IntValue(IntType.REF, o as Int)
        } else if (o instanceof Long) {
            return new LongValue(LongType.REF, o as Long)
        } else if (o instanceof Float) {
            return new FloatValue(FloatType.REF, o as Float)
        } else if (o instanceof Double) {
            return new DoubleValue(DoubleType.REF, o as Double)
        }

        let t = Type.of(o)
        if (t instanceof NullType) {
            return NullValue.INSTANCE
        } else if (t instanceof UndefinedType) {
            return UndefinedValue.INSTANCE
        } else if (t instanceof VoidType) {
            return VoidValue.INSTANCE
        } else if (t instanceof ClassType) {
            return new ClassValue(t as ClassType, o as object)
        } else if (t instanceof ArrayType) {
            return new ArrayValue(t as ArrayType, o as object)
        } else if (t instanceof StringType) {
            return new StringValue(t as StringType, o! as string)
        } else if (t instanceof LambdaType) {
            return new LambdaValue(t as LambdaType, o as object)
        } else if (t instanceof MethodType) {
            throw new Error("The MethodType cannot be instantiated")
        } else if (t instanceof EnumType) {
            throw new Error("Invalid EnumType")
        } else if (t instanceof UnionType) {
            throw new Error("Invalid UnionType")
        } else if (t instanceof TupleType) {
            throw new Error("Invalid TupleType")
        }
        throw new Error("Unknown type")
    }

    public abstract getType(): Type

    public abstract getData(): Any

    public abstract toPrint(depth: int): string
}

} // namespace reflect


/**
 * Represents value of object of class type
 */
export final class ClassValue extends reflect.Value {
    private typ: ClassType
    private data: Object

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return this.data
    }

    public constructor(typ: ClassType, data: Object) {
        this.typ = typ
        this.data = data
    }

    /**
     * Returns number of fields of this value
     *
     * @returns number of fields
     */
    public getFieldsNum(): long {
        return this.typ.getFieldsNum()
    }

    /**
     * Returns ith field of value
     *
     * @param i index
     *
     * @throws error when i greater then field's number
     *
     * @returns field {@link reflect.Value}
     */
    public getField(i: long): reflect.Value {
        let f = this.typ.getField(i)
        if (f.isStatic()) {
            throw new Error("Field must be not static")
        }
        let ft = f.getType()
        if (!ft.isReference()) {
            if (ft instanceof BooleanType) {
                return new BooleanValue(ft as BooleanType, ValueAPIGetFieldBoolean(this.data, i))
            } else if (ft instanceof ByteType) {
                return new ByteValue(ft as ByteType, ValueAPIGetFieldByte(this.data, i))
            } else if (ft instanceof ShortType) {
                return new ShortValue(ft as ShortType, ValueAPIGetFieldShort(this.data, i))
            } else if (ft instanceof CharType) {
                return new CharValue(ft as CharType, ValueAPIGetFieldChar(this.data, i))
            } else if (ft instanceof IntType) {
                return new IntValue(ft as IntType, ValueAPIGetFieldInt(this.data, i))
            } else if (ft instanceof FloatType) {
                return new FloatValue(ft as FloatType, ValueAPIGetFieldFloat(this.data, i))
            } else if (ft instanceof DoubleType) {
                return new DoubleValue(ft as DoubleType, ValueAPIGetFieldDouble(this.data, i))
            } else if (ft instanceof LongType) {
                return new LongValue(ft as LongType, ValueAPIGetFieldLong(this.data, i))
            }
        }
        return reflect.Value.of(ValueAPIGetFieldObject(this.data, i))
    }

    public getFieldByName(name: string): reflect.Value {
        let f = this.typ.getFieldByName(name)
        if (f.isStatic()) {
            throw new Error("Field must be not static")
        }
        const ft = f.getType()
        if (!ft.isReference()) {
            if (ft instanceof BooleanType) {
                return new BooleanValue(ft as BooleanType, ValueAPIGetFieldByNameBoolean(this.data, name))
            } else if (ft instanceof ByteType) {
                return new ByteValue(ft as ByteType, ValueAPIGetFieldByNameByte(this.data, name))
            } else if (ft instanceof ShortType) {
                return new ShortValue(ft as ShortType, ValueAPIGetFieldByNameShort(this.data, name))
            } else if (ft instanceof CharType) {
                return new CharValue(ft as CharType, ValueAPIGetFieldByNameChar(this.data, name))
            } else if (ft instanceof IntType) {
                return new IntValue(ft as IntType, ValueAPIGetFieldByNameInt(this.data, name))
            } else if (ft instanceof FloatType) {
                return new FloatValue(ft as FloatType, ValueAPIGetFieldByNameFloat(this.data, name))
            } else if (ft instanceof DoubleType) {
                return new DoubleValue(ft as DoubleType, ValueAPIGetFieldByNameDouble(this.data, name))
            } else if (ft instanceof LongType) {
                return new LongValue(ft as LongType, ValueAPIGetFieldByNameLong(this.data, name))
            }
            throw new AssertionError("Type " + f.getType() + " is not a (un)boxed primitive")
        }
        return reflect.Value.of(ValueAPIGetFieldByNameObject(this.data, name))
    }

    /**
     * Sets field value by it's name
     *
     * @param name of field
     *
     * @param val @{link reflect.Value}
     *
     * @throws error if this value doesn't have field with {@link name}
     * or type of {@link val} doesn't assignable to type of this value
     */
    public setFieldByName(name: string, val: reflect.Value) {
        let f = this.typ.getFieldByName(name)
        if (f.isStatic()) {
            throw new Error("Field must be not static")
        }
        let ft:Type
        let fvalue = this.getFieldByName(name)
        if (fvalue.getType() instanceof UndefinedType) {
            ft = f.getType()
        } else {
            ft = fvalue.getType()
        }
        let vt = val.getType()
        if (!ft.assignableFrom(vt)) {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }

        if (ft instanceof BooleanType && val instanceof BooleanValue) {
            ValueAPISetFieldByNameBoolean(this.data, name, (val as BooleanValue).getValueData())
        } else if (ft instanceof ByteType && val instanceof ByteValue) {
            ValueAPISetFieldByNameByte(this.data, name, (val as ByteValue).getValueData())
        } else if (ft instanceof ShortType && val instanceof ShortValue) {
            ValueAPISetFieldByNameShort(this.data, name, (val as ShortValue).getValueData())
        } else if (ft instanceof CharType && val instanceof CharValue) {
            ValueAPISetFieldByNameChar(this.data, name, (val as CharValue).getValueData())
        } else if (ft instanceof IntType && val instanceof IntValue) {
            ValueAPISetFieldByNameInt(this.data, name, (val as IntValue).getValueData())
        } else if (ft instanceof FloatType && val instanceof FloatValue) {
            ValueAPISetFieldByNameFloat(this.data, name, (val as FloatValue).getValueData())
        } else if (ft instanceof DoubleType && val instanceof DoubleValue) {
            ValueAPISetFieldByNameDouble(this.data, name, (val as DoubleValue).getValueData())
        } else if (ft instanceof LongType && val instanceof LongValue) {
            ValueAPISetFieldByNameLong(this.data, name, (val as LongValue).getValueData())
        } else if (ft.assignableFrom(StringType.REF) && val instanceof StringValue) {
            ValueAPISetFieldByNameObject(this.data, name, (val as StringValue).getData()! as string)
        } else if (ft instanceof ArrayType && val instanceof ArrayValue) {
            ValueAPISetFieldByNameObject(this.data, name, (val as ArrayValue).getData() as object)
        } else if (ft instanceof ClassType && this.isAssignableToObject(val)) {
            ValueAPISetFieldByNameObject(this.data, name, val.getData() as object)
        } else if (ft instanceof LambdaType && val instanceof LambdaValue) {
            ValueAPISetFieldByNameObject(this.data, name, (val as LambdaValue).getData() as object)
        } else if (ft instanceof DoubleType && val instanceof ByteValue) {
            ValueAPISetFieldByNameDouble(this.data, name, val.getValueData())
        } else if (ft instanceof DoubleType && val instanceof ShortValue) {
            ValueAPISetFieldByNameDouble(this.data, name, val.getValueData())
        } else if (ft instanceof DoubleType && val instanceof IntValue) {
            ValueAPISetFieldByNameDouble(this.data, name, val.getValueData())
        } else if (ft instanceof DoubleType && val instanceof LongValue) {
            ValueAPISetFieldByNameDouble(this.data, name, val.getValueData())
        } else if (ft instanceof DoubleType && val instanceof FloatValue) {
            ValueAPISetFieldByNameDouble(this.data, name, val.getValueData())
        } else if (ft instanceof FloatType && val instanceof ByteValue) {
            ValueAPISetFieldByNameFloat(this.data, name, val.getValueData())
        } else if (ft instanceof FloatType && val instanceof ShortValue) {
            ValueAPISetFieldByNameFloat(this.data, name, val.getValueData())
        } else if (ft instanceof FloatType && val instanceof IntValue) {
            ValueAPISetFieldByNameFloat(this.data, name, val.getValueData())
        } else if (ft instanceof FloatType && val instanceof LongValue) {
            ValueAPISetFieldByNameFloat(this.data, name, val.getValueData())
        } else if (ft instanceof LongType && val instanceof ByteValue) {
            ValueAPISetFieldByNameLong(this.data, name, val.getValueData())
        } else if (ft instanceof LongType && val instanceof ShortValue) {
            ValueAPISetFieldByNameLong(this.data, name, val.getValueData())
        } else if (ft instanceof LongType && val instanceof IntValue) {
            ValueAPISetFieldByNameLong(this.data, name, val.getValueData())
        } else if (ft instanceof LongType && val instanceof ByteValue) {
            ValueAPISetFieldByNameLong(this.data, name, val.getValueData())
        } else if (ft instanceof LongType && val instanceof ShortValue) {
            ValueAPISetFieldByNameLong(this.data, name, val.getValueData())
        } else if (ft instanceof IntType && val instanceof ByteValue) {
            ValueAPISetFieldByNameInt(this.data, name, val.getValueData())
        } else if (ft instanceof IntType && val instanceof ShortValue) {
            ValueAPISetFieldByNameInt(this.data, name, val.getValueData())
        } else if (ft instanceof ShortType && val instanceof ByteValue) {
            ValueAPISetFieldByNameShort(this.data, name, val.getValueData())
        } else {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
    }

    private isAssignableToObject(v: reflect.Value): boolean {
        return v instanceof ClassValue
            || v instanceof LambdaValue
            || v instanceof StringValue
            || v instanceof ArrayValue
    }

    /**
     * Sets field value by it's index
     *
     * @param i of field
     *
     * @param val @{link reflect.Value}
     *
     * @throws error if this value doesn't have field with {@link index}'th index
     * or type of {@link val} doesn't assignable to type of this value
     */
    public setField(i: long, val: reflect.Value) {
        let f = this.typ.getField(i)
        if (f.isStatic()) {
            throw new Error("Field must be not static")
        }
        let ft = f.getType()
        let vt = val.getType()
        if (!ft.assignableFrom(vt)) {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
        if (ft instanceof BooleanType && val instanceof BooleanValue) {
            ValueAPISetFieldBoolean(this.data, i, (val as BooleanValue).getValueData())
        } else if (ft instanceof ByteType && val instanceof ByteValue) {
            ValueAPISetFieldByte(this.data, i, (val as ByteValue).getValueData())
        } else if (ft instanceof ShortType && val instanceof ShortValue) {
            ValueAPISetFieldShort(this.data, i, (val as ShortValue).getValueData())
        } else if (ft instanceof CharType && val instanceof CharValue) {
            ValueAPISetFieldChar(this.data, i, (val as CharValue).getValueData())
        } else if (ft instanceof IntType && val instanceof IntValue) {
            ValueAPISetFieldInt(this.data, i, (val as IntValue).getValueData())
        } else if (ft instanceof FloatType && val instanceof FloatValue) {
            ValueAPISetFieldFloat(this.data, i, (val as FloatValue).getValueData())
        } else if (ft instanceof DoubleType && val instanceof DoubleValue) {
            ValueAPISetFieldDouble(this.data, i, (val as DoubleValue).getValueData())
        } else if (ft instanceof LongType && val instanceof LongValue) {
            ValueAPISetFieldLong(this.data, i, (val as LongValue).getValueData())
        } else if (ft instanceof StringType && val instanceof StringValue) {
            ValueAPISetFieldObject(this.data, i, (val as StringValue).getData()! as string)
        } else if (ft instanceof ArrayType && val instanceof ArrayValue) {
            ValueAPISetFieldObject(this.data, i, (val as ArrayValue).getData() as object)
        } else if (ft instanceof ClassType && this.isAssignableToObject(val)) {
            ValueAPISetFieldObject(this.data, i, (val as ClassValue).getData() as object)
        } else if (ft instanceof LambdaType && val instanceof LambdaValue) {
            ValueAPISetFieldObject(this.data, i, val.getData() as object)
        } else {
            throw new Error("Cannot assign field of type " + ft + " with value of type " + vt)
        }
    }

    public override toPrint(depth: int): string {

        if (this.typ.getName() == "std.core.ArrayBuffer") {
            if (depth == 0) {
                return "std.core.ArrayBuffer {...}";
            }
            const result = new StringBuilder("std.core.ArrayBuffer {");
            const dataField = this.getFieldByName('data');
            result.append("data: ").append(dataField.toPrint(depth - 1));
            const byteLengthField = this.getFieldByName('_byteLength');
            result.append(", byteLength: ").append(byteLengthField.toPrint(depth - 1));
            result.append("}");
            return result.toString();
        }

        if (this.typ.getName() == "std.core.Date") {
            return new StringBuilder("Date: ").append("\"").append((this.data as Date).toISOString()).append("\"").toString()
        }
        if (depth == 0) {
            return this.typ.getName() + " {...}"
        }

        const thisStr = this.data.toString()
        const objStr = new Object().toString()
        if (thisStr != objStr) {
            return thisStr
        }
        const fnum = this.getFieldsNum()
        if (fnum == 0) {
            return this.typ.getName() + " {}"
        }
        let res = new StringBuilder(this.typ.getName())
        res.append(" {")
        for (let i = 0; i < fnum; i++) {
            const f = this.typ.getField(i)
            if (f.isStatic()) {
                continue
            }
            const fv = this.getField(i)
            res.append(f.getName())
            res.append(": ")
            res.append(fv.toPrint(depth - 1))
            if (i != fnum - 1) {
                res.append(", ")
            }
        }
        res.append("}")
        return res.toString()
    }

    public override toString(): string {
        return "[object Object]"
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents array value
 */
export final class ArrayValue extends reflect.Value {
    private typ: ArrayType
    private data: Object

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return this.data
    }

    public constructor(typ: ArrayType, data: Object) {
        this.typ = typ
        this.data = data
    }

    public getLength(): long {
        return ValueAPIGetArrayLength(this.data)
    }

    public getElement(i: long): reflect.Value {
        const et = this.typ.getElementType()
        if (!et.isReference()) {
            if (et instanceof BooleanType) {
                return new BooleanValue(et as BooleanType, ValueAPIGetElementBoolean(this.data, i))
            } else if (et instanceof ByteType) {
                return new ByteValue(et as ByteType, ValueAPIGetElementByte(this.data, i))
            } else if (et instanceof ShortType) {
                return new ShortValue(et as ShortType, ValueAPIGetElementShort(this.data, i))
            } else if (et instanceof CharType) {
                return new CharValue(et as CharType, ValueAPIGetElementChar(this.data, i))
            } else if (et instanceof IntType) {
                return new IntValue(et as IntType, ValueAPIGetElementInt(this.data, i))
            } else if (et instanceof FloatType) {
                return new FloatValue(et as FloatType, ValueAPIGetElementFloat(this.data, i))
            } else if (et instanceof DoubleType) {
                return new DoubleValue(et as DoubleType, ValueAPIGetElementDouble(this.data, i))
            } else if (et instanceof LongType) {
                return new LongValue(et as LongType, ValueAPIGetElementLong(this.data, i))
            }
            throw new AssertionError("Type " + et + " is not a (un)boxed primitive")
        }
        return reflect.Value.of(ValueAPIGetElementObject(this.data, i))
    }

    /**
     * Sets i'th element of array
     *
     * @param i index
     *
     * @param val {@link reflect.Value} to be set
     *
     * @throws error if of {@link val} cannot be assign to i'th element of array
     */
    public setElement(i: long, val: reflect.Value) {
        let et = this.typ.getElementType()
        let vt = val.getType()
        if (!et.assignableFrom(vt)) {
            throw new Error("Cannot assign array of type " + et + " with value of type " + vt)
        }
        if (et instanceof BooleanType && val instanceof BooleanValue) {
            ValueAPISetElementBoolean(this.data, i, (val as BooleanValue).getValueData())
        } else if (et instanceof ByteType && val instanceof ByteValue) {
            ValueAPISetElementByte(this.data, i, (val as ByteValue).getValueData())
        } else if (et instanceof ShortType && val instanceof ShortValue) {
            ValueAPISetElementShort(this.data, i, (val as ShortValue).getValueData())
        } else if (et instanceof CharType && val instanceof CharValue) {
            ValueAPISetElementChar(this.data, i, (val as CharValue).getValueData())
        } else if (et instanceof IntType && val instanceof IntValue) {
            ValueAPISetElementInt(this.data, i, (val as IntValue).getValueData())
        } else if (et instanceof FloatType && val instanceof FloatValue) {
            ValueAPISetElementFloat(this.data, i, (val as FloatValue).getValueData())
        } else if (et instanceof DoubleType && val instanceof DoubleValue) {
            ValueAPISetElementDouble(this.data, i, (val as DoubleValue).getValueData())
        } else if (et instanceof LongType && val instanceof LongValue) {
            ValueAPISetElementLong(this.data, i, (val as LongValue).getValueData())
        } else if (et instanceof StringType && val instanceof StringValue) {
            ValueAPISetElementObject(this.data, i, (val as StringValue).getData()! as string)
        } else if (et instanceof ArrayType && val instanceof ArrayValue) {
            ValueAPISetElementObject(this.data, i, (val as ArrayValue).getData() as object)
        } else if (et instanceof ClassType && val instanceof ClassValue) {
            ValueAPISetElementObject(this.data, i, (val as ClassValue).getData() as object)
        } else {
            throw new Error("Cannot assign array of type " + et + " with value of type " + vt)
        }
    }

    public override toPrint(depth: int): string {
        if (depth == 0) {
            return "[...]"
        }
        const len = this.getLength()
        if (len == 0) {
            return "[]"
        }
        let res = new StringBuilder("[")
        for (let i = 0; i < len; i++) {
            res.append(this.getElement(i).toPrint(depth - 1))
            if (i != len - 1) {
                res.append(", ")
            }
        }
        res.append("]")
        return res.toString()
    }

    public override toString(): string {
        const len = this.getLength()
        if (len == 0) {
            return ""
        }
        let res = new StringBuilder()
        for (let i = 0; i < len; i++) {
            res.append(this.getElement(i).toString())
            if (i != len - 1) {
                res.append(",")
            }
        }
        return res.toString()
    }

    public override toLocaleString(): string {
        const len = this.getLength()
        if (len == 0) {
            return ""
        }
        let res = new StringBuilder()
        for (let i = 0; i < len; i++) {
            res.append(this.getElement(i).toLocaleString())
            if (i != len - 1) {
                res.append(",")
            }
        }
        return res.toString()
    }
}

export final class LambdaValue extends reflect.Value {
    private typ: LambdaType
    private data: Object

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return this.data
    }

    public constructor(typ: LambdaType, data: Object) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return "[object Function]"
    }

    public override toString(): string {
        return "Cannot get source code of function";
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents boolean value
 */
export final class BooleanValue extends reflect.Value {
    private typ: BooleanType
    private data: boolean

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Boolean(this.data)
    }

    public getValueData(): boolean {
        return this.data
    }

    public constructor(typ: BooleanType, data: boolean) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents byte value
 */
export final class ByteValue extends reflect.Value {
    private typ: ByteType
    private data: byte

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Byte(this.data)
    }

    public getValueData(): byte {
        return this.data
    }

    public constructor(typ: ByteType, data: byte) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents short value
 */
export final class ShortValue extends reflect.Value {
    private typ: ShortType
    private data: short

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Short(this.data)
    }

    public getValueData(): short {
        return this.data
    }

    public constructor(typ: ShortType, data: short) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents char value
 */
export final class CharValue extends reflect.Value {
    private typ: CharType
    private data: char

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Char(this.data)
    }

    public getValueData(): char {
        return this.data
    }

    public constructor(typ: CharType, data: char) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder("\"").append(this.data).append("\"").toString()
    }

    public override toString(): string {
        return new StringBuilder("\"").append(this.data).append("\"").toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents int value
 */
export final class IntValue extends reflect.Value {
    private typ: IntType
    private data: int

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Int(this.data)
    }

    public getValueData(): int {
        return this.data
    }

    public constructor(typ: IntType, data: int) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents float value
 */
export final class FloatValue extends reflect.Value {
    private typ: FloatType
    private data: float

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Float(this.data)
    }

    public getValueData(): float {
        return this.data
    }

    public constructor(typ: FloatType, data: float) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents double value
 */
export final class DoubleValue extends reflect.Value {
    private typ: DoubleType
    private data: double

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return this.data
    }

    public getValueData(): double {
        return this.data
    }

    public constructor(typ: DoubleType, data: double) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents long value
 */
export final class LongValue extends reflect.Value {
    private typ: LongType
    private data: long

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return new Long(this.data)
    }

    public getValueData(): long {
        return this.data
    }

    public constructor(typ: LongType, data: long) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents string value
 */
export final class StringValue extends reflect.Value {
    private typ: StringType
    private data: string

    public override getType(): Type {
        return this.typ as Type
    }

    public override getData(): Any {
        return this.data
    }

    public constructor(typ: StringType, data: String) {
        this.typ = typ
        this.data = data
    }

    public override toPrint(depth: int): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toString(): string {
        return new StringBuilder().append(this.data).toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents null value
 */
export final class NullValue extends reflect.Value {
    public static readonly INSTANCE = new NullValue()

    public override getType(): Type {
        return NullType.REF as Type
    }

    public override getData(): Any {
        return null
    }

    public constructor() {}

    public override toPrint(depth: int): string {
        return NullType.REF.toString()
    }

    public override toString(): string {
        return NullType.REF.toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents undefined value
 */
export final class UndefinedValue extends reflect.Value {
    public static readonly INSTANCE = new UndefinedValue()

    public override getType(): Type {
        return UndefinedType.REF as Type
    }

    public override getData(): Any {
        return undefined
    }

    public constructor() {}

    public override toPrint(depth: int): string {
        return UndefinedType.REF.toString()
    }

    public override toString(): string {
        return UndefinedType.REF.toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}

/**
 * Represents void value
 */
export final class VoidValue extends reflect.Value {
    public static readonly INSTANCE = new VoidValue()

    public override getType(): Type {
        return VoidType.REF as Type
    }

    public override getData(): Any {
        return Void.void_instance
    }

    public constructor() {}

    public override toPrint(depth: int): string {
        return VoidType.REF.toString()
    }

    public override toString(): string {
        return VoidType.REF.toString()
    }

    public override toLocaleString(): string {
        return this.toString()
    }
}
