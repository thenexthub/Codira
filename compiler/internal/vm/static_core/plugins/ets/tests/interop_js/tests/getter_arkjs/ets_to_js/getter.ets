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

package getter.test;

class ModuleLoader {
    static _module: ESValue = ESValue.load('constant_js');

    public static getProp(propertyName: string): ESValue {
        return ModuleLoader._module.getProperty(propertyName);
    }
}

class PublicGetterClass {
    PUBLIC_GETTER_CLASS_VALUE = ModuleLoader.getProp("PUBLIC_GETTER_CLASS_VALUE");    
    _value = this.PUBLIC_GETTER_CLASS_VALUE.toString();

    get value(): string {
        return this._value;
    }
}

function create_public_getter_class_from_ets(): PublicGetterClass {
    return new PublicGetterClass();
}

class PublicGetterOverrideClass extends PublicGetterClass {
    PUBLIC_GETTER_OVERRIDE_CLASS_VALUE = ModuleLoader.getProp('PUBLIC_GETTER_OVERRIDE_CLASS_VALUE');
    public get value(): string {
        return this.PUBLIC_GETTER_OVERRIDE_CLASS_VALUE.toString();
    }
}

function create_public_getter_override_class_from_ets(): PublicGetterOverrideClass {
    return new PublicGetterOverrideClass();
}

class ProtectedGetterOrigenClass {
    PROTECTED_GETTER_CLASS_VALUE = ModuleLoader.getProp('PROTECTED_GETTER_CLASS_VALUE');
    protected _value = this.PROTECTED_GETTER_CLASS_VALUE.toString();

    public get value(): string {
        return this._value;
    }
}

function create_protected_getter_origen_class_from_ets(): ProtectedGetterOrigenClass {
    return new ProtectedGetterOrigenClass();
}

class ProtectedGetterInheritanceClass extends ProtectedGetterOrigenClass { }

function create_protected_getter_inheritance_class_from_ets(): ProtectedGetterInheritanceClass {
    return new ProtectedGetterInheritanceClass();
}

class ProtectedGetterOverrideClass extends ProtectedGetterOrigenClass {
    PROTECTED_GETTER_OVERRIDE_CLASS_VALUE = ModuleLoader.getProp('PROTECTED_GETTER_OVERRIDE_CLASS_VALUE');
    public get value(): string {
        return this.PROTECTED_GETTER_OVERRIDE_CLASS_VALUE.toString();
    }
}

function create_protected_getter_override_class_from_ets(): ProtectedGetterOverrideClass {
    return new ProtectedGetterOverrideClass();
}

class PrivateGetterClass {
    PRIVATE_GETTER_CLASS_VALUE = ModuleLoader.getProp('PRIVATE_GETTER_CLASS_VALUE');
    private _value = this.PRIVATE_GETTER_CLASS_VALUE.toString();

    public get value(): string {
        return this._value;
    }
}

function create_private_getter_class_from_ets(): PrivateGetterClass {
    return new PrivateGetterClass();
}

class PrivateGetterInheritanceClass extends PrivateGetterClass { }

function create_private_getter_inheritance_class_from_ets(): PrivateGetterInheritanceClass {
    return new PrivateGetterInheritanceClass();
}

class PrivateGetterOverrideClass extends PrivateGetterClass {
    PRIVATE_GETTER_OVERRIDE_CLASS_VALUE = ModuleLoader.getProp('PRIVATE_GETTER_OVERRIDE_CLASS_VALUE');
    public get value(): string {
        return this.PRIVATE_GETTER_OVERRIDE_CLASS_VALUE.toString();
    }
}

function create_private_getter_override_class_from_ets(): PrivateGetterOverrideClass {
    return new PrivateGetterOverrideClass();
}

type Union = number | string;

class UnionTypeClass {
    public _value: Union;

    constructor(value: Union) {
        this._value = value;
    }

    public get value(): Union {
        return this._value;
    }
}

function create_union_type_getter_class_from_ets(arg: Union): UnionTypeClass {
    return new UnionTypeClass(arg);
}

type TupleT = [number, string];

class TupleTypeClass {
    private _value: TupleT;

    constructor(value: TupleT) {
        this._value = value;
    }

    public get value(): TupleT {
        return this._value;
    }
}

function create_tuple_type_getter_class_from_ets(arg: TupleT): TupleTypeClass {
    return new TupleTypeClass(arg);
}

class AnyTypeClass<T> {
    public _value: T;

    constructor(value: T) {
        this._value = value;
    }

    public get value(): T {
        return this._value;
    }

    public set value(val: T) {
        this._value = val;
    }

}

class StaticClass<T> {
    public _value: T;
    public static _static_value: string = '';

    constructor(value: T) {
        this.value = value;
    }

    public get value(): T {
        return this._value;
    }

    public set value(val: T) {
        this._value = val;
    }

    public static get value(): string {
        return StaticClass._static_value;
    }

    public static set value(val: string) {
        StaticClass._static_value = val;
    }

}

class StaticPublicFieldClass {
    public value: string;
    public static _static_value: string = '';

    constructor(value: string) {
        this.value = value;
    }

    public static get value(): string {
        return StaticPublicFieldClass._static_value;
    }

    public static set value(val: string) {
        StaticPublicFieldClass._static_value = val;
    }

}

function create_any_type_getter_class_from_ets_string(arg: string): AnyTypeClass<string> {
    return new AnyTypeClass<string>(arg);
}

function create_any_type_getter_class_from_ets_int(arg: int): AnyTypeClass<int> {
    return new AnyTypeClass<int>(arg);
}

function create_any_type_getter_class_from_ets_bool(arg: boolean): AnyTypeClass<boolean> {
    return new AnyTypeClass<boolean>(arg);
}

function create_any_type_getter_class_from_ets_arr(arg: FixedArray<int>): AnyTypeClass<FixedArray<int>> {
    return new AnyTypeClass<FixedArray<int>>(arg);
}

function create_any_type_getter_class_from_ets_obj(arg: object): AnyTypeClass<object> {
    return new AnyTypeClass<object>(arg);
}

function create_any_type_getter_class_from_ets_tuple(arg: TupleT): AnyTypeClass<TupleT> {
    return new AnyTypeClass<TupleT>(arg);
}

function create_any_type_getter_class_from_ets_union(arg: Union): AnyTypeClass<Union> {
    return new AnyTypeClass<Union>(arg);
}

class SubsetByRef {
    private _refClass: PublicGetterClass;

    constructor() {
        this._refClass = new PublicGetterClass();
    }

    public get value(): string {
        return this._refClass.value;
    }
}

function create_subset_by_ref_getter_class_from_ets(): SubsetByRef {
    return new SubsetByRef();
}

class SubsetByValueClass {
    public _value: string;

    constructor(value: string) {
        this._value = value;
    }

    get value(): string {
        return this._value;
    }
}

function create_subset_by_value_getter_class_from_ets(): SubsetByValueClass {
    const GClass = new PublicGetterClass();
    return new SubsetByValueClass(GClass.value);
}