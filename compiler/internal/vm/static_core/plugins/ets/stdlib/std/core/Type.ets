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

export type TypeDescriptor = string

// Internal platform-dependent descriptor, part of implementation
type RuntimeTypeDescriptor = string

class TypeAPI {
    /*
        Type intrinsics
    */
    native static getTypeKind(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): byte

    native static isValueType(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): boolean

    native static getNullTypeDescriptor(): RuntimeTypeDescriptor

    native static getUndefinedTypeDescriptor(): RuntimeTypeDescriptor

    /*
        Class type intrinsics
    */
    native static getClass(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): Class | undefined

    native static getClassAttributes(cls: Class): int

    native static getFieldsNum(cls: Class): long

    native static getOwnFieldsNum(cls: Class): long

    native static getFieldPtr(cls: Class, i: long): Long | undefined

    native static getOwnFieldPtr(cls: Class, i: long): Long | undefined

    native static getFieldPtrByName(cls: Class, name: string): Long | undefined

    native static getFieldDescriptor(fieldPtr: long): RuntimeTypeDescriptor

    native static getFieldOwner(fieldPtr: long): Class

    native static getField(fieldPtr: long, fieldType: Type, ownerType: Type): Field

    native static getMethodsNum(cls: Class): long

    native static getMethodDescriptor(cls: Class, i: long): RuntimeTypeDescriptor | undefined

    native static getMethod(methodType: MethodType, ownerType: Type): Method

    native static getConstructorsNum(cls: Class): long

    native static getConstructorDescriptor(cls: Class, i: long): RuntimeTypeDescriptor | undefined

    native static getInterfacesNum(cls: Class): long

    native static getInterface(cls: Class, i: long): Class | undefined

    native static isInheritedFrom(lhs: Class, rhs: Class): boolean

    /*
        Function (Lambda or Method) type intrinsics
    */
    native static getFunctionAttributes(td: RuntimeTypeDescriptor): int

    /*
        Array type intrinsics
    */
    native static getArrayElementType(cls: Class): RuntimeTypeDescriptor

    native static makeArrayInstance(elemTD: RuntimeTypeDescriptor, contextLinker: RuntimeLinker, len: long): Object

    /*
        General helpers
    */
    static getClassDescriptor(cls: Class): RuntimeTypeDescriptor {
        return cls.getDescriptor()
    }

    private static NullTypeDescriptor: RuntimeTypeDescriptor = TypeAPI.getNullTypeDescriptor()
    private static UndefinedTypeDescriptor: RuntimeTypeDescriptor = TypeAPI.getUndefinedTypeDescriptor()

    static getTypeDescriptor(o: Any): RuntimeTypeDescriptor {
        if (o === null) {
            return TypeAPI.NullTypeDescriptor
        }
        return (o === undefined) ? TypeAPI.UndefinedTypeDescriptor : TypeAPI.getClassDescriptor(Class.of(o as object))
    }

    static getTypeName(cls: Class): string {
        return cls.getName()
    }

    static convertTypeDescriptor(td: TypeDescriptor): RuntimeTypeDescriptor {
        let sb = new StringBuilder("L")
        sb.append(td.replaceChar(c'.', c'/'))
        sb.append(";")
        return sb.toString()
    }

    native static getDeclaringClassImpl(type: LambdaType | MethodType): Class | undefined
}

// NOTE(shumilov-petr): replace to enum, enum not available now
export class TypeKind {
    public static readonly NONE: byte       = 0x0
    public static readonly VOID: byte       = 0x1

    public static readonly CHAR: byte       = 0x2
    public static readonly BOOLEAN: byte    = 0x3
    public static readonly BYTE: byte       = 0x4
    public static readonly SHORT: byte      = 0x5
    public static readonly INT: byte        = 0x6
    public static readonly LONG: byte       = 0x7
    public static readonly FLOAT: byte      = 0x8
    public static readonly DOUBLE: byte     = 0x9

    public static readonly CLASS: byte      = 0xA
    public static readonly STRING: byte     = 0xB
    public static readonly INTERFACE: byte  = 0xC
    public static readonly ARRAY: byte      = 0xD
    public static readonly TUPLE: byte      = 0xE
    public static readonly LAMBDA: byte     = 0xF
    public static readonly METHOD: byte     = 0x10

    public static readonly UNION: byte      = 0x11
    public static readonly UNDEFINED: byte  = 0x12
    public static readonly NULL: byte       = 0x13

    public static readonly ENUM: byte       = 0x14

    private constructor() {}
}

// 5 lower bits stores kind's id
const TypeKindMask: byte;

// NOTE(shumilov-petr): replace to enum, enum not available now
class ValueTypeDesc {
    public static readonly BOOLEAN: RuntimeTypeDescriptor   = "Z"
    public static readonly BYTE: RuntimeTypeDescriptor      = "B"
    public static readonly SHORT: RuntimeTypeDescriptor     = "S"
    public static readonly CHAR: RuntimeTypeDescriptor      = "C"
    public static readonly INT: RuntimeTypeDescriptor       = "I"
    public static readonly LONG: RuntimeTypeDescriptor      = "J"
    public static readonly FLOAT: RuntimeTypeDescriptor     = "F"
    public static readonly DOUBLE: RuntimeTypeDescriptor    = "D"
    public static readonly VOID: RuntimeTypeDescriptor      = "V"

    private constructor() {}
}

// initialized in _initializerBlock_.ets
export const ObjectType: ClassType;

export class Attributes {
    public static readonly STATIC: int      = 1 << 0  // Field, Method
    public static readonly READONLY: int    = 1 << 1  // Field
    public static readonly FINAL: int       = 1 << 2  // Method, Class
    public static readonly ABSTRACT: int    = 1 << 3  // Method
    public static readonly CONSTRUCTOR: int = 1 << 4  // Method
    public static readonly REST: int        = 1 << 5  // TypeAPIParameter
    public static readonly OPTIONAL: int    = 1 << 6  // TypeAPIParameter
    public static readonly NATIVE: int      = 1 << 7  // Method, Lambda
    public static readonly ASYNC: int       = 1 << 8  // Method, Lambda
    public static readonly GETTER: int      = 1 << 9  // Method
    public static readonly SETTER: int      = 1 << 10 // Method

    private constructor() {}
}

export class AccessModifier {
    public static readonly PUBLIC: byte       = 0
    public static readonly PRIVATE: byte      = 1
    public static readonly PROTECTED: byte    = 2

    private constructor() {}
}

/**
 * Runtime ArkTS type representation
 */
export abstract class Type extends Object {
    private td: RuntimeTypeDescriptor
    private contextLinker: RuntimeLinker

    /**
     * Resolves type by descriptor
     *
     * @param td type descriptor
     *
     * @returns instance of appropriate type or undefined if resolving is failed
     */
    public static resolve(td: TypeDescriptor): Type | undefined {
        const callerClass = Class.ofCaller()
        const contextLinker = callerClass == undefined ? getBootRuntimeLinker() : callerClass.getLinker()
        const internalTd: RuntimeTypeDescriptor = TypeAPI.convertTypeDescriptor(td)
        return Type.resolve(internalTd, contextLinker)
    }

    /**
     * Resolves type by internal descriptor, part of implementation
     *
     * @param td internal runtime type descriptor
     *
     * @param contextLinker in which classes must be searched
     *
     * @returns instance of appropriate type or undefined if resolving is failed
     */
    public static resolve(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): Type | undefined {
        let kind = (TypeAPI.getTypeKind(td, contextLinker) & TypeKindMask).toByte()
        switch (kind) {
            case TypeKind.NONE:
                return undefined
            case TypeKind.VOID:
                return td == ValueTypeDesc.VOID ? VoidType.VAL : VoidType.REF
            case TypeKind.CHAR:
                return TypeAPI.isValueType(td, contextLinker) ? CharType.VAL : CharType.REF
            case TypeKind.BOOLEAN:
                return TypeAPI.isValueType(td, contextLinker) ? BooleanType.VAL : BooleanType.REF
            case TypeKind.BYTE:
                return TypeAPI.isValueType(td, contextLinker) ? ByteType.VAL : ByteType.REF
            case TypeKind.SHORT:
                return TypeAPI.isValueType(td, contextLinker) ? ShortType.VAL : ShortType.REF
            case TypeKind.INT:
                return TypeAPI.isValueType(td, contextLinker) ? IntType.VAL : IntType.REF
            case TypeKind.LONG:
                return TypeAPI.isValueType(td, contextLinker) ? LongType.VAL : LongType.REF
            case TypeKind.FLOAT:
                return TypeAPI.isValueType(td, contextLinker) ? FloatType.VAL : FloatType.REF
            case TypeKind.DOUBLE:
                return TypeAPI.isValueType(td, contextLinker) ? DoubleType.VAL : DoubleType.REF

            case TypeKind.CLASS:
                return new ClassType(td, contextLinker)
            case TypeKind.STRING:
                return StringType.REF
            case TypeKind.INTERFACE:
                return new InterfaceType(td, contextLinker)
            case TypeKind.ARRAY:
                return ArrayType.getInstance(td, contextLinker)
            case TypeKind.TUPLE:
                return new TupleType(td, contextLinker)
            case TypeKind.LAMBDA:
                return new LambdaType(td, contextLinker)
            case TypeKind.METHOD:
                return new MethodType(td, contextLinker)
            case TypeKind.UNION:
                return new UnionType(td, contextLinker)
            case TypeKind.UNDEFINED:
                return UndefinedType.REF
            case TypeKind.NULL:
                return NullType.REF

            case TypeKind.ENUM:
                return new EnumType(td, contextLinker)
            default:
                throw new AssertionError("Unexpected TypeKind " + kind)
        }
    }

    public static from<T>(): Type {
        //should never be executed since FE lowering this
        throw new AssertionError("Type.from call will be casted by frontend")
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: boolean): Type {
        return BooleanType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: byte): Type {
        return ByteType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: char): Type {
        return CharType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: short): Type {
        return ShortType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: int): Type {
        return IntType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: long): Type {
        return LongType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: float): Type {
        return FloatType.VAL
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: double): Type {
        return DoubleType.VAL
    }

    // -----

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: string): Type {
        return StringType.REF
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<boolean>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.BOOLEAN)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<byte>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.BYTE)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<char>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.CHAR)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<short>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.SHORT)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<int>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.INT)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<long>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.LONG)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<float>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.FLOAT)
    }

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<double>): Type {
        return ArrayType.getInstance(Class.of(v), ValueTypeDesc.DOUBLE)
    }

    // -----

    /**
     * Returns Type of value
     *
     * @param v value
     *
     * @returns Type instance of this value
     */
    public static of(v: FixedArray<Object>): Type {
        return ArrayType.getInstance(Class.of(v))
    }

    // -----

    public static of(o: Any): Type {
        const td = TypeAPI.getTypeDescriptor(o)
        const contextClass = o == undefined ? Class.ofCaller() : Class.of(o as object)
        const contextLinker = contextClass == undefined ? getBootRuntimeLinker() : contextClass.getLinker()
        return Type.resolve(td, contextLinker)!
    }

    // -----

    public abstract isPrimitive(): boolean // Or Composite

    public abstract isReference(): boolean // Or Value

    public abstract hasName(): boolean

    protected constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        this.td = td
        this.contextLinker = contextLinker
    }

    public getTypeDesc(): RuntimeTypeDescriptor {
        return this.td
    }

    protected getContextLinker(): RuntimeLinker {
        return this.contextLinker
    }

    public getClass(): Class | undefined {
        return TypeAPI.getClass(this.td, this.contextLinker)
    }

    // T.subTypeOf(U) means that `T extends U`
    protected subTypeOf(other: Type): boolean {
        if (this.equals(other)) {
            return true
        } else if (other.equals(ObjectType)) {
            let isNullish = (this) instanceof UndefinedType || (this) instanceof NullType
            return this.isReference() && !isNullish
        }
        return false
    }

    // T.assignableFrom(U) means that `T <- U`
    public assignableFrom(other: Type): boolean {
        if (other.subTypeOf(this)) {
            return true
        }
        if (this.isNumericType() && other.isNumericType()) {
            return true
        }
        if (this instanceof BooleanType && other instanceof BooleanType) {
            return true
        }
        if (this instanceof CharType && other instanceof CharType) {
            return true
        }
        return false
    }

    public abstract convertObject(obj: Any): Any;

    private isNumericType(): boolean {
        return (this) instanceof ByteType
            || (this) instanceof ShortType
            || (this) instanceof IntType
            || (this) instanceof LongType
            || (this) instanceof FloatType
            || (this) instanceof DoubleType
    }

    public abstract getName(): string

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public abstract getLiteral(): string

    public override toString(): string {
        if (this.hasName()) {
            return this.getName()
        }
        return this.getLiteral()
    }

    abstract equals(other: Type): boolean

    public static toRefType(primitiveType: Type): Type {
        if (!primitiveType.isPrimitive()) {
            throw new Error(`type ${primitiveType.getName()} is not primitive type`)
        }

        if (primitiveType.isReference()) {
            // StringType is primitive and reference at the same time
            return primitiveType
        }

        if (primitiveType instanceof BooleanType) {
            return BooleanType.REF
        } else if (primitiveType instanceof ByteType) {
            return ByteType.REF
        } else if (primitiveType instanceof CharType) {
            return CharType.REF
        } else if (primitiveType instanceof ShortType) {
            return ShortType.REF
        } else if (primitiveType instanceof IntType) {
            return IntType.REF
        } else if (primitiveType instanceof LongType) {
            return LongType.REF
        } else if (primitiveType instanceof FloatType) {
            return FloatType.REF
        } else if (primitiveType instanceof DoubleType) {
            return DoubleType.REF
        } else if (primitiveType instanceof VoidType) {
            return VoidType.REF
        } else {
            throw new Error(`failed to convert primitive type ${primitiveType.getName()} to relevant reference type`)
        }
    }
}

/**
 * Represents null type
 */
export final class NullType extends Type {
    public static readonly REF: NullType = new NullType()

    private constructor() {
        super(TypeAPI.getTypeDescriptor(null), getBootRuntimeLinker())
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns type name
     */
    public override getName(): string {
        return "null"
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "null"
    }

    /**
     * Checks for equality this instance with provided object, treated as a DoubleType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has NullType
     */
    public override equals(other: Type): boolean {
        return other instanceof NullType
    }

    public override convertObject(obj: Any): Any {
        if (obj != null) {
            throw new Error("invalid conversion")
        }
        return null
    }
}

export final class UndefinedType extends Type {
    public static readonly REF: UndefinedType = new UndefinedType()

    private constructor() {
        super(TypeAPI.getTypeDescriptor(undefined), getBootRuntimeLinker())
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @returns type name
     */
    public override getName(): string {
        return "undefined"
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "undefined"
    }

    /**
     * Checks for equality this instance with provided object, treated as a UndefinedType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has UndefinedType
     */
    public override equals(other: Type): boolean {
        return other instanceof UndefinedType
    }

    public override convertObject(obj: Any): Any {
        throw new Error("todo(kprokopenko): add when undefined becomes available")
    }
}

/**
 * Represents void type
 */
export final class VoidType extends Type {
    public static readonly VAL: VoidType = new VoidType(ValueTypeDesc.VOID, true)
    public static readonly REF: VoidType = new VoidType(TypeAPI.getTypeDescriptor(Void.void_instance), false)

    private readonly value: boolean

    private constructor(td: RuntimeTypeDescriptor, value: boolean) {
        super(td, getBootRuntimeLinker())
        this.value = value
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.value
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @returns type name
     */
    public override getName(): string {
        return "void"
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "void"
    }

    /**
     * Checks for equality this instance with provided object, treated as a VoidType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has VoidType
     */
    public override equals(other: Type): boolean {
        return other instanceof VoidType
    }

    public override convertObject(obj: Any): Any {
        if (!this.assignableFrom(Type.of(obj))) {
            throw new Error("invalid conversion")
        }
        return Void.void_instance
    }
}

/**
 * Represents char type
 *
 * @note Boxed Char and primitive char both have CharType
 */

export final class CharType extends Type {
    public static readonly VAL: CharType = new CharType(ValueTypeDesc.CHAR, true)
    public static readonly REF: CharType = new CharType(TypeAPI.getTypeDescriptor(new Char()), false)
    private static readonly CLASS = Class.of(new Char())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(CharType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "char"
        }
        return "Char"
    }

    public override equals(other: Type): boolean {
        return other instanceof CharType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid type for conversion")
        }
        return obj
    }
}

/**
 * Represents boolean type
 *
 * @note Boxed Boolean and primitive boolean both have BooleanType
 */
export final class BooleanType extends Type {
    public static readonly VAL: BooleanType = new BooleanType(ValueTypeDesc.BOOLEAN, true)
    public static readonly REF: BooleanType = new BooleanType(TypeAPI.getTypeDescriptor(new Boolean()), false)
    private static readonly CLASS = Class.of(new Boolean())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(BooleanType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "boolean"
        }
        return "Boolean"
    }

    public override equals(other: Type): boolean {
        return other instanceof BooleanType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid type for conversion")
        }
        return obj
    }
}

/**
 * Represents byte type
 *
 * @note Boxed Byte and primitive byte both have ByteType
 */
export final class ByteType extends Type {
    public static readonly VAL: ByteType = new ByteType(ValueTypeDesc.BYTE, true)
    public static readonly REF: ByteType = new ByteType(TypeAPI.getTypeDescriptor(new Byte()), false)
    private static readonly CLASS = Class.of(new Byte())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(ByteType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "byte"
        }
        return "Byte"
    }

    public override equals(other: Type): boolean {
        return other instanceof ByteType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toByte()
    }
}

/**
 * Represents short type
 *
 * @note Boxed Short and primitive short both have ShortType
 */
export final class ShortType extends Type {
    public static readonly VAL: ShortType = new ShortType(ValueTypeDesc.SHORT, true)
    public static readonly REF: ShortType = new ShortType(TypeAPI.getTypeDescriptor(new Short()), false)
    private static readonly CLASS = Class.of(new Short())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(ShortType.CLASS)
    }

    /**
     * Returns literal of type
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "short"
        }
        return "Short"
    }

    /**
     * Checks for equality this instance with provided object, treated as a ShortType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has ShortType and
     * this type and other type both are reference or value
     */
    public override equals(other: Type): boolean {
        return other instanceof ShortType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toShort()
    }
}

/**
 * Represents int type
 *
 * @note Boxed Int and primitive int both have IntType
 */
export final class IntType extends Type {
    public static readonly VAL: IntType = new IntType(ValueTypeDesc.INT, true)
    public static readonly REF: IntType = new IntType(TypeAPI.getTypeDescriptor(new Int()), false)
    private static readonly CLASS = Class.of(new Int())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @throws error in case of absence of name
     *
     * @returns type name
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(IntType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "int"
        }
        return "Int"
    }

    /**
     * Checks for equality this instance with provided object, treated as a IntType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has IntType and
     * this type and other type both are reference or value
     */
    public override equals(other: Type): boolean {
        return other instanceof IntType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toInt()
    }
}

/**
 * Represents long type
 *
 * @note Boxed Long and primitive long both have LongType
 */
export final class LongType extends Type {
    public static readonly VAL = new LongType(ValueTypeDesc.LONG, true)
    public static readonly REF = new LongType(TypeAPI.getTypeDescriptor(new Long()), false)
    private static readonly CLASS = Class.of(new Long())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type if exists and empty string otherwise
     *
     * @throws error in case of absence of name
     *
     * @returns type name
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(LongType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "long"
        }
        return "Long"
    }

    /**
     * Checks for equality this instance with provided object, treated as a LongType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has LongType and
     * this type and other type both are reference or value
     */
    public override equals(other: Type): boolean {
        return other instanceof LongType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toLong()
    }
}

/**
 * Represents float type
 *
 * @note Boxed Float and primitive float both have FloatType
 */
export final class FloatType extends Type {
    public static readonly VAL: FloatType = new FloatType(ValueTypeDesc.FLOAT, true)
    public static readonly REF: FloatType = new FloatType(TypeAPI.getTypeDescriptor(new Float()), false)
    private static readonly CLASS = Class.of(new Float())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(FloatType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "float"
        }
        return "Float"
    }

    /**
     * Checks for equality this instance with provided object, treated as a FloatType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has FloatType and
     * this type and other type both are reference or value
     */
    public override equals(other: Type): boolean {
        return other instanceof FloatType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toFloat()
    }
}

/**
 * Represents double type
 *
 * @note Boxed Double and primitive double both have DoubleType
 */
export final class DoubleType extends Type {
    public static readonly VAL: DoubleType = new DoubleType(ValueTypeDesc.DOUBLE, true)
    public static readonly REF: DoubleType = new DoubleType(TypeAPI.getTypeDescriptor(new Double()), false)
    private static readonly CLASS = Class.of(new Double())

    private isValue: boolean

    private constructor(td: RuntimeTypeDescriptor, isValue: boolean) {
        super(td, getBootRuntimeLinker())
        this.isValue = isValue
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return !this.isValue
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return !this.isValue
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    //NOTE(kirill-mitkin): add error
    public override getName(): string {
        if (this.isValue) {
            return ""
        }
        return TypeAPI.getTypeName(DoubleType.CLASS)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        if (this.isValue) {
            return "double"
        }
        return "Double"
    }

    /**
     * Checks for equality this instance with provided object, treated as a DoubleType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has DoubleType and
     * this type and other type both are reference or value
     */
    public override equals(other: Type): boolean {
        return other instanceof DoubleType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return (obj as Numeric).toDouble()
    }
}

/**
 * Represents type of classes
 */
export final class ClassType extends Type {
    private readonly cls: Class
    private readonly attrs: int

    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
        this.cls = TypeAPI.getClass(td, contextLinker)!
        this.attrs = TypeAPI.getClassAttributes(this.cls)
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @returns type name
     */
    public override getName(): string {
        return TypeAPI.getTypeName(this.cls)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "class{...}"
    }

    /**
     * Checks for equality this instance with provided object, treated as a ClassType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has ClassType and their names are the same
     */
    public override equals(other: Type): boolean {
        // Must compare classes, as descriptors might be equal between classes from different contexts
        return (other instanceof ClassType) && (other as ClassType).cls == this.cls
    }

    /**
     * Returns base type of this class
     * If this type is the type of Object class then returns this
     *
     * @returns base type of class
     */
    public getBaseType(): ClassType {
        const base = this.cls.getSuper()
        return (base == null) ? this : Type.resolve(base.getDescriptor(), base.getLinker())! as ClassType
    }

    /**
     * Returns number of direct superinterfaces of this class
     *
     * @returns number of interfaces that was implemented by this class directly
     */
    public getInterfacesNum(): long {
        return TypeAPI.getInterfacesNum(this.cls)
    }

    /**
     * Returns ith direct superinterface of this class
     *
     * @param i index
     *
     * @throws TypeError when i greater then num of interfaces
     *
     * @returns type of ith superinterface
     */
    public getInterface(i: long): InterfaceType {
        const iface = TypeAPI.getInterface(this.cls, i)
        if (iface) {
            return Type.resolve(iface.getDescriptor(), iface.getLinker()) as InterfaceType
        }
        throw new TypeError("no interface at " + i)
    }

    /**
     * Returns number of all fields
     * including static, instance and also fields of all its superclasses
     *
     * @returns number of fields
     *
     * @example
     *
     * ```
     * class A {
     *     a : int
     * }
     *
     * class B extends A {
     *     b : int
     * }
     * ```
     * let bType class type of B, then `bType.getFieldsNum()` returns 2
     * Note that Object class also is super class of B
     */
    public getFieldsNum(): long {
        return TypeAPI.getFieldsNum(this.cls)
    }

    public getOwnFieldsNum(): long {
        return TypeAPI.getOwnFieldsNum(this.cls)
    }

    /**
     * Returns ith Field of this class
     *
     * @param i index (using flat semantic)
     *
     * @throws TypeError when i greater then number of fields
     *
     * @returns ith Field
     */
    public getField(i: long): Field {
        const fieldPtr = TypeAPI.getFieldPtr(this.cls, i)
        if (fieldPtr) {
            return this.getFieldImpl(fieldPtr)
        }
        throw new TypeError("no field at " + i)
    }

    /**
     * Returns ith Field owned by this class
     *
     * @param i index (using flat semantic)
     *
     * @throws TypeError when i greater then number of fields
     *
     * @returns ith Field
     */
    public getOwnField(i: long): Field {
        const fieldPtr = TypeAPI.getOwnFieldPtr(this.cls, i)
        if (fieldPtr) {
            return this.getFieldImpl(fieldPtr)
        }
        throw new TypeError("no own field at " + i)
    }

    /**
     * Find Field by name
     *
     * @param name name of field
     *
     * @throws TypeError when class doesn't have field with this name
     *
     * @returns Field instance with this name
     */
    public getFieldByName(name: string): Field {
        const fieldPtr = TypeAPI.getFieldPtrByName(this.cls, name)
        if (fieldPtr) {
            return this.getFieldImpl(fieldPtr)
        }
        throw new TypeError("no field \"" + name + "\"")
    }

    public getMethodByName(name: string): Method{
        for (let methodIdx = 0; methodIdx < this.getMethodsNum(); ++methodIdx) {
            const method = this.getMethod(methodIdx)
            if (method.getName() == name && !method.isStatic()) {
                return method
            }
        }
        throw new TypeError("no instance method \"" + name + "\"")
    }

    private getFieldImpl(fieldPtr: long): Field {
        const fieldDesc = TypeAPI.getFieldDescriptor(fieldPtr)
        const fieldType = Type.resolve(fieldDesc, this.getContextLinker())!
        const ownerClass = TypeAPI.getFieldOwner(fieldPtr)
        const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
        return TypeAPI.getField(fieldPtr, fieldType, ownerType)
    }

    /**
     * Returns number of methods of this class
     * including static methods and methods of super classes
     * @example
     *
     * ```
     * class A {
     *     a(): void {}
     * }
     *
     * class B extends A {
     *     b(): void {}
     * }
     * ```
     * let bType class type of B, then `bType.getMethodsNum()` returns at least 2
     * Note that Object class also super class of B
     * @returns number of methods
     */
    public getMethodsNum(): long {
        return TypeAPI.getMethodsNum(this.cls)
    }

    /**
     * Returns ith Method of this class
     *
     * @param i index (using flat semantic)
     *
     * @throws TypeError when i greater then number of methods
     *
     * @returns ith method
     */
    public getMethod(i: long): Method {
        const td = TypeAPI.getMethodDescriptor(this.cls, i)
        if (td) {
            const methodType = Type.resolve(td, this.getContextLinker())! as MethodType
            const ownerClass = TypeAPI.getDeclaringClassImpl(methodType)!
            const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
            return TypeAPI.getMethod(methodType, ownerType)
        }
        throw new TypeError("no method at " + i)
    }

    /**
     * Returns number of constructors of this class
     * Note that constructors of super class isn't considered as constructors of this class
     *
     * @returns number of constructors
     */
    public getConstructorsNum(): long {
        return TypeAPI.getConstructorsNum(this.cls)
    }

    /**
     * Returns ith constructor of this class
     *
     * @param i index
     *
     * @throws TypeError then i greater then number of constructors
     *
     * @returns {@link Method} instance representing ith constructor
     */
    public getConstructor(i: long): Method {
        const td = TypeAPI.getConstructorDescriptor(this.cls, i)
        if (td) {
            const methodType = Type.resolve(td, this.getContextLinker())! as MethodType
            const ownerClass = TypeAPI.getDeclaringClassImpl(methodType)!
            const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
            return TypeAPI.getMethod(methodType, ownerType)
        }
        throw new TypeError("no constructor at " + i)
    }

    /**
     * Checks for existence of empty constructor of this class
     *
     * @returns true if there is empty constructor of this class
     */
    public hasEmptyConstructor(): boolean {
        let num = this.getConstructorsNum()
        for (let i = 0; i < num; i++) {
            if (this.getConstructor(i).getType().getParametersNum() == 0) {
                return true
            }
        }
        return false
    }

    public isFinal(): boolean {
        return (this.attrs & Attributes.FINAL) != 0
    }

    public hasField(name: string): boolean {
        const fieldPtr = TypeAPI.getFieldPtrByName(this.cls, name)
        return (fieldPtr !== undefined)
    }

    /**
     * Makes instance of this type by invoking
     * empty constructor of this class
     *
     * @throws error when class doesn't have an empty constructor
     */
    public make(): Object {
        return this.cls.createInstance()
    }

    // todo(kprokopenko): make varargs
    public make(args: FixedArray<Any>): Object {
        const argTypes = new Array<Type>(args.length)
        for (let i = 0; i < args.length; i++) {
            argTypes[i] = Type.of(args[i])
        }
        // collect all applicable constructors
        const ctors = new Array<Method>()
        const ctorNum = this.getConstructorsNum()
        for (let i = 0; i < ctorNum; i++) {
            const c = this.getConstructor(i)
            const ct = c.getType()
            if (ct.getParametersNum() == args.length) {
                let ok = true
                for (let arg = 0; arg < args.length; arg++) {
                    if (!ct.getParameter(arg).getType().assignableFrom(argTypes[arg])) {
                        ok = false
                        break
                    }
                }
                if (ok) {
                    ctors.push(c)
                }
            }
        }
        // inspect if constructor has a more specific one (O(n^2))
        for (let inspect = 0; inspect < ctors.length; inspect++) {
            const toRem = ctors.at(inspect)!.getType()
            let rem = false
            for (let c = 0; c < ctors.length; c++) {
                if (c == inspect) {
                    continue
                }
                rem = true
                const cur = ctors.at(c)!.getType()
                for (let a = 0; a < args.length; a++) {
                    if (!toRem.getParameter(a).getType().assignableFrom(cur.getParameter(a).getType())) {
                        rem = false;
                        break;
                    }
                }
                if (rem) {
                    break
                }
            }
            if (rem) {
                ctors.splice(inspect, 1)
                inspect--
            }
        }
        if (ctors.length != 1) {
            throw new Error("can't select consturctor: " + ctors.length + " left")
        }
        return ctors.at(0)!.invoke(undefined, args) as Object
    }

    // class.subTypeOf(U) means that `class extends U` or `class implements U`
    protected override subTypeOf(other: Type): boolean {
        if (super.subTypeOf(other)) {
            return true
        }
        // `this` is derived of `other` class?
        if (other instanceof ClassType) {
            let bt = this;
            while (!bt.equals(bt.getBaseType())) {
                if (bt.equals(other)) {
                    return true
                }
                bt = bt.getBaseType()
            }
            return false
        }
        // `this` implements `other` interface?
        if (other instanceof InterfaceType) {
            let iface = other as InterfaceType
            let bt = this
            while (!bt.equals(bt.getBaseType())) {
                let ifaceNum = bt.getInterfacesNum()
                for (let i = 0; i < ifaceNum; i++) {
                    if (bt.getInterface(i).hasSuperInterface(iface)) {
                        return true
                    }
                }
                bt = bt.getBaseType()
            }
        }
        return false
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents interface type
 */
export final class InterfaceType extends Type {
    private cls: Class

    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
        this.cls = TypeAPI.getClass(td, contextLinker)!
    }

    /**
     * @returns Class of interface
     */
    public getClass(): Class {
        return this.cls
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @returns name of type
     */
    public override getName(): string {
        return TypeAPI.getTypeName(this.cls)
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "interface{...}"
    }

    /**
     * Checks for equality this instance with provided object, treated as a InterfaceType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has InterfaceType and
     * their names are the same
     */
    public override equals(other: Type): boolean {
        // Must compare classes, as descriptors might be equal between classes from different contexts
        return other instanceof InterfaceType && this.cls == (other as InterfaceType).cls
    }

    // `this` extends of `other` interface?
    protected override subTypeOf(other: Type): boolean {
        if (super.subTypeOf(other)) {
            return true
        }
        if (other instanceof InterfaceType) {
            return this.hasSuperInterface(other as InterfaceType)
        }
        return false
    }

    public getInterfacesNum(): long {
        return TypeAPI.getInterfacesNum(this.cls)
    }

    /**
     * Returns ith direct superinterface of this interface
     *
     * @param i index
     *
     * @throws TypeError when i greater then num of interfaces
     *
     * @returns type of ith superinterface
     */
    public getInterface(i: long): InterfaceType {
        const iface = TypeAPI.getInterface(this.cls, i)
        if (iface) {
            return Type.resolve(iface.getDescriptor(), iface.getLinker()) as InterfaceType
        }
        throw new TypeError("no interface at " + i)
    }

    /**
     * Returns number of methods of this interface
     * including static methods and methods of super interfaces
     * @example
     *
     * ```
     * class A {
     *     a(): void {}
     * }
     *
     * class B extends A {
     *     b(): void {}
     * }
     * ```
     * let bType interface type of B, then `bType.getMethodsNum()` returns at least 2
     * Note that Object class also super class of B
     * @returns number of methods
     */
    public getMethodsNum(): long {
        return TypeAPI.getMethodsNum(this.cls)
    }

    /**
     * Returns ith Method of this interface
     *
     * @param i index (using flat semantic)
     *
     * @throws error when i greater then number of methods
     *
     * @returns ith method
     */
    public getMethod(i: long): Method {
        const td = TypeAPI.getMethodDescriptor(this.cls, i)
        if (td) {
            const methodType = Type.resolve(td, this.getContextLinker())! as MethodType
            const ownerClass = TypeAPI.getDeclaringClassImpl(methodType)!
            const ownerType = Type.resolve(ownerClass.getDescriptor(), ownerClass.getLinker())!
            return TypeAPI.getMethod(methodType, ownerType)
        }
        throw new TypeError("no method at " + i)
    }

    public hasSuperInterface(expected: InterfaceType): boolean {
        if (this.equals(expected)) {
            return true
        }
        return TypeAPI.isInheritedFrom(this.cls, expected.cls)
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents array type
 */
export final class ArrayType extends Type {
    private elemTD: RuntimeTypeDescriptor

    public static readonly BOOLEAN_VAL: ArrayType   = new ArrayType(Class.of(new boolean[0]), ValueTypeDesc.BOOLEAN)
    public static readonly BOOLEAN_REF: ArrayType   = new ArrayType(Class.of(new Boolean[0]), TypeAPI.getTypeDescriptor(new Boolean()))
    public static readonly CHAR_VAL: ArrayType      = new ArrayType(Class.of(new char[0]), ValueTypeDesc.CHAR)
    public static readonly CHAR_REF: ArrayType      = new ArrayType(Class.of(new Char[0]), TypeAPI.getTypeDescriptor(new Char()))
    public static readonly BYTE_VAL: ArrayType      = new ArrayType(Class.of(new byte[0]), ValueTypeDesc.BYTE)
    public static readonly BYTE_REF: ArrayType      = new ArrayType(Class.of(new Byte[0]), TypeAPI.getTypeDescriptor(new Byte()))
    public static readonly SHORT_VAL: ArrayType     = new ArrayType(Class.of(new short[0]), ValueTypeDesc.SHORT)
    public static readonly SHORT_REF: ArrayType     = new ArrayType(Class.of(new Short[0]), TypeAPI.getTypeDescriptor(new Short()))
    public static readonly INT_VAL: ArrayType       = new ArrayType(Class.of(new int[0]), ValueTypeDesc.INT)
    public static readonly INT_REF: ArrayType       = new ArrayType(Class.of(new Int[0]), TypeAPI.getTypeDescriptor(new Int()))
    public static readonly LONG_VAL: ArrayType      = new ArrayType(Class.of(new long[0]), ValueTypeDesc.LONG)
    public static readonly LONG_REF: ArrayType      = new ArrayType(Class.of(new Long[0]), TypeAPI.getTypeDescriptor(new Long()))
    public static readonly FLOAT_VAL: ArrayType     = new ArrayType(Class.of(new float[0]), ValueTypeDesc.FLOAT)
    public static readonly FLOAT_REF: ArrayType     = new ArrayType(Class.of(new Float[0]), TypeAPI.getTypeDescriptor(new Float()))
    public static readonly DOUBLE_VAL: ArrayType    = new ArrayType(Class.of(new double[0]), ValueTypeDesc.DOUBLE)
    public static readonly DOUBLE_REF: ArrayType    = new ArrayType(Class.of(new Double[0]), TypeAPI.getTypeDescriptor(new Double()))

    private constructor(arrayClass: Class, elemTD: RuntimeTypeDescriptor) {
        super(TypeAPI.getClassDescriptor(arrayClass), arrayClass.getLinker())
        this.elemTD = elemTD
    }

    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        return other instanceof ArrayType && (other as ArrayType).getElementType().subTypeOf(this.getElementType())
    }

    public static getInstance(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker): ArrayType {
        const arrayClass = TypeAPI.getClass(td, contextLinker)!
        return ArrayType.getInstance(arrayClass)
    }

    public static getInstance(arrayClass: Class, elemTD?: RuntimeTypeDescriptor): ArrayType {
        elemTD = elemTD ?? TypeAPI.getArrayElementType(arrayClass)
        const contextLinker = arrayClass.getLinker()
        let ek = (TypeAPI.getTypeKind(elemTD, contextLinker) & TypeKindMask).toByte()
        switch (ek) {
            case TypeKind.BOOLEAN:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.BOOLEAN_VAL : ArrayType.BOOLEAN_REF
            case TypeKind.CHAR:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.CHAR_VAL : ArrayType.CHAR_REF
            case TypeKind.BYTE:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.BYTE_VAL : ArrayType.BYTE_REF
            case TypeKind.SHORT:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.SHORT_VAL : ArrayType.SHORT_REF
            case TypeKind.INT:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.INT_VAL : ArrayType.INT_REF
            case TypeKind.LONG:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.LONG_VAL : ArrayType.LONG_REF
            case TypeKind.FLOAT:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.FLOAT_VAL : ArrayType.FLOAT_REF
            case TypeKind.DOUBLE:
                return TypeAPI.isValueType(elemTD, contextLinker) ? ArrayType.DOUBLE_VAL : ArrayType.DOUBLE_REF
            case TypeKind.CLASS:
            case TypeKind.STRING:
            case TypeKind.INTERFACE:
            case TypeKind.ARRAY:
            case TypeKind.TUPLE:
            case TypeKind.LAMBDA:
            case TypeKind.UNION:
                return new ArrayType(arrayClass, elemTD)
            default:
                throw new AssertionError("Unexpected TypeKind " + ek)
        }
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @throws error in case of absence of name
     *
     * @returns name of type
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return this.getElementType().toString() + "[]"
    }

    /**
     * Checks for equality this instance with provided object, treated as a ArrayType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has ArrayType and
     * their element types are the same
     */
    public override equals(other: Type): boolean {
        return other instanceof ArrayType && (other as ArrayType).getElementType().equals(this.getElementType())
    }

    /**
     * Returns element type of this array
     *
     * @returns element type
     */
    public getElementType(): Type {
        return Type.resolve(this.elemTD, this.getContextLinker())!
    }

    /**
     * Makes instance of this array with provided length
     * Each element are instantiated using default value
     * If element type is class value then empty constructor are called
     *
     * @param length of array
     *
     * @throws error if element type doesn't have default value
     *
     * @returns new instance of array
     */
    public make(length: long): Object {
        return TypeAPI.makeArrayInstance(this.elemTD, this.getContextLinker(), length)
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

export final class TupleType extends Type {
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    public override isPrimitive(): boolean {
        return false
    }

    public override isReference(): boolean {
        return false
    }

    public override hasName(): boolean {
        return false
    }

    public override getName(): string {
        return ""
    }

    public override getLiteral(): string {
        return "[ ..., ...]"
    }

    public override equals(other: Type): boolean {
        return false
    }

    public override convertObject(obj: Any): Any {
        throw new Error("todo(kprokopenko): add when tuple becomes available")
    }
}

/**
 * Represents function type
 *
 * @note lambdas, functions and methods have function type
 */
export abstract class FunctionType extends Type {
    private readonly attrs: int

    protected constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
        this.attrs = TypeAPI.getFunctionAttributes(td)
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns name of type
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns return type of function type
     *
     * @returns result type
     */
    public getResultType(): Type {
        const resultTypeTd = this.getResultTypeDescriptor()
        return Type.resolve(resultTypeTd, this.getContextLinker())!
    }

    /**
     * Checks for equality this instance with provided object, treated as a FunctionType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has FunctionType and
     * they both has same signatures
     */
    public override equals(other: Type): boolean {
        return (other instanceof FunctionType) && (other as FunctionType).getTypeDesc() == this.getTypeDesc()
    }

    /**
     * Returns number of parameters
     * For static methods reciever type counted as parameter
     * For instance methods reciever type isn't counted as parameter
     *
     * @returns number of parameters
     */
    public native getParametersNum(): long

    /**
     * Returns ith parameter of function type
     *
     * @param i index
     *
     * @throws TypeError when i greater then number of parameters
     *
     * @return TypeAPIParameter, corresponding ith parameter in signature
     */
    public getParameter(i: long): TypeAPIParameter {
        const paramDescriptor = this.getParameterDescriptor(i)
        if (paramDescriptor) {
            const paramType = Type.resolve(paramDescriptor, this.getContextLinker())!
            return this.getParameter(i, paramType)
        }
        throw new TypeError("no parameter at " + i)
    }

    /**
     * Returns number of type parameters of this class
     *
     * @returns number of type parameters
     */
    public getTypeParametersNum(): long {
        return 0
    }

    private native getResultTypeDescriptor(): RuntimeTypeDescriptor

    private native getParameterDescriptor(i: long): RuntimeTypeDescriptor | undefined

    private native getParameter(i: long, paramType: Type): TypeAPIParameter
}


export final class LambdaType extends FunctionType {
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    public override getLiteral(): string {
        let sb = new StringBuilder("(")
        const paramsNum = this.getParametersNum()
        for (let i = 0; i < paramsNum; ++i) {
            sb.append(this.getParameter(i).toString())
            if (i != paramsNum - 1) {
                sb.append(", ")
            }
        }
        sb.append("): ")
        sb.append(this.getResultType().toString())
        return sb.toString()
    }

    /**
     * Make an instance of LambdaType
     *
     * @returns LambdaType instance
     */
    public make(): Object {
        return TypeAPI.getDeclaringClassImpl(this)!.createInstance()
    }

    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        if (!(other instanceof LambdaType)) {
            return false
        }
        let l = (this)
        let r = other as LambdaType
        if (l.getParametersNum() < r.getParametersNum()) {
            return false
        }

        // Parameter types are using contravariance
        for (let i = 0; i < r.getParametersNum(); i++) {
            let lt = l.getParameter(i).getType()
            let rt = r.getParameter(i).getType()

            if (!lt.subTypeOf(rt)) {
                if (rt.isPrimitive()) {
                    const rrt = Type.toRefType(rt)
                    if (!lt.subTypeOf(rrt)) {
                        return false
                    }
                } else {
                    return false
                }
            }
        }

        // Return types are using covariance
        if (r.getResultType().subTypeOf(l.getResultType())) {
            return true
        }

        const rvalResType = r.getResultType()
        if (rvalResType.isPrimitive()) {
            return Type.toRefType(rvalResType).subTypeOf(l.getResultType())
        } else {
            return false
        }
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (this.assignableFrom(objType)) {
            return obj
        }
        throw new Error("invalid conversion")
    }
}

export final class MethodType extends FunctionType {
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    public override getLiteral(): string {
        let sb = new StringBuilder("(")
        const paramsNum = this.getParametersNum()
        if (this.hasReceiver()) {
            sb.append("this: ")
            sb.append(this.getReceiverType().toString())
            if (paramsNum > 0) {
                sb.append(", ")
            }
        }
        for (let i = 0; i < paramsNum; ++i) {
            sb.append(this.getParameter(i).toString())
            if (i != paramsNum - 1) {
                sb.append(", ")
            }
        }
        sb.append("): ")
        sb.append(this.getResultType().toString())
        return sb.toString()
    }

    /**
     * Returns receiver type of this method
     *
     * @throws TypeError if method has no receiver (i.e. static)
     *
     * @return Type corresponding to method's receiver
     */
    public getReceiverType(): Type {
        const receiverTd = this.getReceiverTypeDescriptor()
        if (receiverTd) {
            return Type.resolve(receiverTd, this.getContextLinker())!
        }
        throw new TypeError("Method does not have receiver")
    }

    public override assignableFrom(other: Type): boolean {
        return false
    }

    public hasReceiver(): boolean {
        return this.getReceiverTypeDescriptor() !== undefined
    }

    public override convertObject(obj: Any): Any {
        throw new Error("Only LambdaType can be converted")
    }

    private native getReceiverTypeDescriptor(): RuntimeTypeDescriptor | undefined
}

/**
 * Represents string type
 */
export final class StringType extends Type {
    public static readonly REF: StringType = new StringType()

    public constructor() {
        super(TypeAPI.convertTypeDescriptor("std.core.String"), getBootRuntimeLinker())
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return true
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns type name
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "string"
    }

    /**
     * Checks for equality this instance with provided object, treated as a StringType
     *
     * @param other object to be checked against
     *
     * @returns true if object also has StringType
     */
    public override equals(other: Type): boolean {
        return other instanceof StringType
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (!this.equals(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}

/**
 * Represents enum type
 */
export final class EnumType extends Type {
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    public override assignableFrom(other: Type): boolean {
        if (super.assignableFrom(other)) {
            return true
        }
        if (!(other instanceof EnumType)) {
            return false
        }
        let rt = other as EnumType
        return this.getName() == rt.getName()
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return false
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return true
    }

    /**
     * Returns name of type
     *
     * @returns type name
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "enum {...}"
    }

    /**
     * Checks for equality this instance with provided object, treated as a EnumType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has EnumType and their names are the same
     */
    public override equals(other: Type): boolean {
        // NOTE(shumilov-petr): not implemented
        return false
    }

    public getConstantsNum(): long {
        // NOTE(shumilov-petr): not implemented
        return 0
    }

    public getConstant(i: long): EnumConstant {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    public getConstantByName(name: string): EnumConstant {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    public create(consts: EnumConstant[]): EnumType {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    public make(constantName: string): Object {
        // NOTE(shumilov-petr): not implemented
        throw new Error("not implemented")
    }

    public override convertObject(obj: Any): Any {
        throw new Error("todo(kprokopenko): enum conversion")
    }
}

/**
 * Represents union type
 */
export final class UnionType extends Type {
    public constructor(td: RuntimeTypeDescriptor, contextLinker: RuntimeLinker) {
        super(td, contextLinker)
    }

    public override assignableFrom(other: Type): boolean {
        if (other instanceof UnionType) {
            const otherUnion = other as UnionType
            // all cases of other are assignable into this
            const otherCasesN = this.getCasesNum()
            return true
        }
        return false
    }

    /**
     * Checks whether type is primitive or composite
     *
     * @returns true if type is primitive and false otherwise
     */
    public override isPrimitive(): boolean {
        return false
    }

    /**
     * Checks whether type is reference or composite
     *
     * @returns true if type is reference and false otherwise
     */
    public override isReference(): boolean {
        return true
    }

    /**
     * Checks whether type has name
     *
     * @returns true if type has name
     */
    public override hasName(): boolean {
        return false
    }

    /**
     * Returns name of type
     *
     * @returns type name
     */
    public override getName(): string {
        return ""
    }

    /**
     * Returns literal of type if exists
     *
     * @returns type literal
     */
    public override getLiteral(): string {
        return "(... | ...)"
    }

    public getCasesNum(): long {
        return 0
    }

    /**
     * Checks for equality this instance with provided object, treated as a UnionType
     *
     * @param other type to be checked against
     *
     * @returns true if object also has UnionType and their cases are the same
     */
    public override equals(other: Type): boolean {
        return false
    }

    public override convertObject(obj: Any): Any {
        const objType = Type.of(obj)
        if (this.equals(objType)) {
            return obj
        }
        if (!this.assignableFrom(objType)) {
            throw new Error("invalid conversion")
        }
        return obj
    }
}
