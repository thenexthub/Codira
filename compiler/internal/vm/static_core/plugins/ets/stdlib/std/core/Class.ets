/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

/**
 * Unstable API, represents internal runtime type descriptor
 */
export final class Class {
    private name: string | undefined
    private superClass: Class | undefined
    private linker: RuntimeLinker | undefined
    private flags: int
    private typeMetaData: long
    private methodsNum: int

    /**
     * Stores Class of boolean primitive type
     */
    public static readonly PRIMITIVE_BOOLEAN: Class = Class.of(null)

    /**
     * Stores Class of byte primitive type
     */
    public static readonly PRIMITIVE_BYTE: Class = Class.of(null)

    /**
     * Stores Class of char primitive type
     */
    public static readonly PRIMITIVE_CHAR: Class = Class.of(null)

    /**
     * Stores Class of short primitive type
     */
    public static readonly PRIMITIVE_SHORT: Class = Class.of(null)

    /**
     * Stores Class of int primitive type
     */
    public static readonly PRIMITIVE_INT: Class = Class.of(null)

    /**
     * Stores Class of long primitive type
     */
    public static readonly PRIMITIVE_LONG: Class = Class.of(null)

    /**
     * Stores Class of float primitive type
     */
    public static readonly PRIMITIVE_FLOAT: Class = Class.of(null)

    /**
     * Stores Class of double primitive type
     */
    public static readonly PRIMITIVE_DOUBLE: Class = Class.of(null)

    /**
     * Stores Class of number primitive type (same as double)
     */
    public static readonly PRIMITIVE_NUMBER: Class = Class.of(null)

    /**
     * Stores Class of void type
     */
    public static readonly PRIMITIVE_VOID: Class = Class.of(null)

    static {
        initializePrimitivesInClass()

        Class.PRIMITIVE_BOOLEAN.name = 'u1'
        Class.PRIMITIVE_BYTE.name = 'i8'
        Class.PRIMITIVE_SHORT.name = 'i16'
        Class.PRIMITIVE_CHAR.name = 'u16'
        Class.PRIMITIVE_INT.name = 'i32'
        Class.PRIMITIVE_LONG.name = 'i64'
        Class.PRIMITIVE_FLOAT.name = 'f32'
        Class.PRIMITIVE_DOUBLE.name = 'f64'
        Class.PRIMITIVE_NUMBER.name = 'f64'
        Class.PRIMITIVE_VOID.name = 'void'
    }

    private constructor() { throw new Error("Class constructor called") }
    private native getNameInternal(): string
    private native getLinkerInternal(): RuntimeLinker | undefined

    /**
     * Gets the name of a class in assembly
     *
     * @returns The name of a Class in assembly
     */
    public getName(): string {
        if (this.name === undefined) {
            this.name = this.getNameInternal()
        }
        return this.name!
    }

    /**
     * Gets the super class of a class
     *
     * @returns The superclass of a class, or `undefined` if this is either Object or interface class
     */
    public getSuper(): Class | undefined {
        return this.superClass
    }

    public native final isSubtypeOf(other: Class): boolean

    public getLinker(): RuntimeLinker {
        if (this.linker == undefined) {
            const linker = this.getLinkerInternal()
            this.linker = linker ?? getBootRuntimeLinker()
        }
        return this.linker!
    }

    public native isNamespace(): boolean;

    public getNamespace(): Class | undefined {
        throw new Error("not implemented")
    }

    public native getFixedArrayComponentType(): Class | undefined

    public native getInterfaces(): FixedArray<Class>

    /**
     * Invoke class initializer once if class is not initialized
     */
    public native initialize(): void

    private static native ofObject(obj: Object): Class

    public static ofAny(obj: Any): Class | undefined {
        if (obj instanceof Object) {
            return Class.of(obj as Object)
        }
        if (obj === null) {
            return Class.from<null>()
        }
        return undefined
    }

    public static of(obj: Object | null): Class {
        if (obj instanceof Object) {
            return Class.ofObject(obj)
        }
        return Class.from<null>()
    }

    public static from<T>(): Class {
        //should never be executed
        throw new AssertionError("Class.from call will be casted by frontend")
    }

    /**
     * Get current class
     */
    public static native current(): Class

    /**
     * Get class of caller
     */
    public static native ofCaller(): Class | undefined

    public native createInstance(): Object

    /**
     * Get internal string descriptor
     */
    public native getDescriptor(): string

    public native isEnum(): boolean

    public native isInterface(): boolean

    public native isFixedArray(): boolean

    /**
     * Check if class is union
     *
     * @returns true if class is union
     */
    public native isUnion(): boolean

    /**
     * Gets public instance methods (including default methods from implemented interfaces) declared in the class
     *
     * @returns Immutable array of instance {@link reflect.InstanceMethod} objects
     */
    public getInstanceMethods(): FixedArray<reflect.InstanceMethod> {
        return this.getInstanceMethodsInternal(true)
    }

    /**
     * Looks up a public instance method (including default methods from implemented interfaces)
     * among the methods declared in the class
     *
     * @param name - The method name to search for
     *
     * @param signature - Array of parameter classes defining the method signature.
     *
     * @throws {Error} When signature is omitted and multiple overloads exist
     *
     * @returns The {@link reflect.StaticMethod} if found, `undefined` otherwise
     */
    public getInstanceMethod(name: string, signature?: FixedArray<Class>): reflect.InstanceMethod | undefined {
        let methods = this.getInstanceMethods()
        if (methods.length == 0) {
            return undefined
        }
        if (signature == undefined) {
            return Class.findInstanceMethodByNameUnchecked(methods, name)
        }
        return Class.findInstanceMethodBySignatureUnchecked(methods, name, signature)
    }

    /**
     * Gets public static methods declared in the class
     *
     * @returns Immutable array of static {@link reflect.StaticMethod} objects
     */
    public getStaticMethods(): FixedArray<reflect.StaticMethod> {
        if (this.isInterface()) {
            return []
        }
        return this.getStaticMethodsInternal(true)
    }

    /**
     * Looks up a public static method among the methods declared in the class
     *
     * @param name - The method name to search for
     *
     * @param signature - Array of parameter classes defining the method signature.
     *
     * @throws {Error} When signature is omitted and multiple overloads exist
     *
     * @returns The {@link reflect.StaticMethod} if found, `undefined` otherwise
     */
    public getStaticMethod(name: string, signature?: FixedArray<Class>): reflect.StaticMethod | undefined {
        let methods = this.getStaticMethods()
        if (methods.length == 0) {
            return undefined
        }
        if (signature == undefined) {
            return Class.findStaticMethodByNameUnchecked(methods, name)
        }
        return Class.findStaticMethodBySignatureUnchecked(methods, name, signature)
    }

    /**
     * Gets public instance fields declared in the class
     *
     * @returns Immutable array of instance {@link reflect.InstanceField} objects
     */
    public getInstanceFields(): FixedArray<reflect.InstanceField> {
        if (this.isInterface()) {
            return []
        }
        return this.getInstanceFieldsInternal(true)
    }

    /**
     * Looks up a public instance field by name among the fields declared in this class
     *
     * @param name - The field name to search for
     *
     * @returns The {@link reflect.InstanceField} if found, `undefined` otherwise
     */
    public getInstanceField(name: string): reflect.InstanceField | undefined {
        return this.getInstanceFieldByNameInternal(name, true)
    }

    /**
     * Gets public static fields declared in this class
     *
     * @returns Immutable array of static {@link reflect.StaticField} objects
     */
    public getStaticFields(): FixedArray<reflect.StaticField> {
        if (this.isInterface()) {
            return []
        }
        return this.getStaticFieldsInternal(true)
    }

    /**
     * Looks up a public static field by name among the fields declared in this class
     *
     * @param name - The field name to search for
     *
     * @returns The {@link reflect.StaticField} if found, `undefined` otherwise
     */
    public getStaticField(name: string): reflect.StaticField | undefined {
        return this.getStaticFieldByNameInternal(name, true)
    }

    /**
     * Gets all public constructors of a class
     *
     * @returns Immutable array of constructor {@link reflect.Constructor} objects
     */
    public getConstructors(): FixedArray<reflect.Constructor> {
        return this.getConstructorsInternal(true)
    }

    /**
     * Gets all constituent types of a union class
     *
     * @returns Immutable array of Class objects or undefined if class is not union
     */
    public getUnionConstituentTypes(): FixedArray<Class> | undefined {
        if (!this.isUnion()) {
            return undefined
        }
        return this.getUnionConstituentTypesInternal()
    }

    /**
     * Check the class is final
     */
    public native isFinal(): boolean

    /**
     * Check the class is abstract
     */
    public native isAbstract(): boolean

    /**
     * Check if class is primitive type
     *
     * @returns true if class is primitive
     */
    public native isPrimitive(): boolean

    // NOTE(kurnevichstanislav): squash two implementations into one after #31016 is fixed.
    private static findStaticMethodByNameUnchecked(
        methods: FixedArray<reflect.StaticMethod>,
        name: string
    ): reflect.StaticMethod | undefined {
        let candidate: reflect.StaticMethod | undefined = undefined

        for (let method of methods) {
            if (method.getName() == name) {
                if (candidate != undefined) {
                    throw new Error(`Found two methods with the same name '${name}'. Please specify signature.`)
                }
                candidate = method
            }
        }
        return candidate
    }

    private static findInstanceMethodByNameUnchecked(
        methods: FixedArray<reflect.InstanceMethod>,
        name: string
    ): reflect.InstanceMethod | undefined {
        let candidate: reflect.InstanceMethod | undefined = undefined

        for (let method of methods) {
            if (method.getName() == name) {
                if (candidate != undefined) {
                    throw new Error(`Found two methods with the same name '${name}'. Please specify signature.`)
                }
                candidate = method
            }
        }
        return candidate
    }

    // NOTE(kurnevichstanislav): squash two implementations into one after #31016 is fixed.
    private static findStaticMethodBySignatureUnchecked(
        methods: FixedArray<reflect.StaticMethod>,
        name: string,
        signature: FixedArray<Class>
    ): reflect.StaticMethod | undefined {
        for (let method of methods) {
            if (method.getName() == name && Class.compareSignatureStatic(method, signature)) {
                return method
            }
        }
        return undefined
    }

    private static findInstanceMethodBySignatureUnchecked(
        methods: FixedArray<reflect.InstanceMethod>,
        name: string,
        signature: FixedArray<Class>
    ): reflect.InstanceMethod | undefined {
        for (let method of methods) {
            if (method.getName() == name && Class.compareSignatureInstance(method, signature)) {
                return method
            }
        }
        return undefined
    }

    // NOTE(kurnevichstanislav): squash two implementations into one after #31016 is fixed.
    private static compareSignatureStatic(method: reflect.StaticMethod, signature: FixedArray<Class>): boolean {
        if (method.getParametersNum() != signature.length) {
            return false
        }
        for (let idx = 0; idx < signature.length; ++idx) {
            if (method.getParameterType(idx) != signature[idx]) {
                return false
            }
        }
        return true
    }

    private static compareSignatureInstance(method: reflect.InstanceMethod, signature: FixedArray<Class>): boolean {
        if (method.getParametersNum() != signature.length) {
            return false
        }
        for (let idx = 0; idx < signature.length; ++idx) {
            if (method.getParameterType(idx) != signature[idx]) {
                return false
            }
        }
        return true
    }

    private native getInstanceMethodsInternal(publicOnly: boolean): FixedArray<reflect.InstanceMethod>
    private native getInstanceFieldByNameInternal(name: string, publicOnly: boolean): reflect.InstanceField | undefined
    private native getInstanceFieldsInternal(publicOnly: boolean): FixedArray<reflect.InstanceField>
    private native getStaticMethodsInternal(publicOnly: boolean): FixedArray<reflect.StaticMethod>
    private native getStaticFieldsInternal(publicOnly: boolean): FixedArray<reflect.StaticField>
    private native getStaticFieldByNameInternal(name: string, publicOnly: boolean): reflect.StaticField | undefined
    private native getConstructorsInternal(publicOnly: boolean): FixedArray<reflect.Constructor>
    private native getUnionConstituentTypesInternal(): FixedArray<Class>
}

native function initializePrimitivesInClass(): void
