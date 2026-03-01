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

native function TypeAPIGetStaticFieldValue(ownerType: Type, name: string): Any
native function TypeAPISetStaticFieldValue(ownerType: Type, name: string, val: Any): void

/**
 * Represents field of class or interface
 *
 */
export class Field extends Object {
    private fieldType: Type | undefined // not undefined if already created
    private ownerType: Type | undefined // not undefined if already created
    private name: string = ""
    private attributes: int
    private accessMod: byte

    private constructor () {}

    /**
     * Returns type of this field
     *
     * @returns {@link Type} of this field
     */
    public getType(): Type {
        return this.fieldType!
    }

    public getOwnerType(): Type {
        return this.ownerType!
    }

    public getName(): String {
        return this.name
    }

    public getAttributes(): int {
        return this.attributes
    }

    public getAccessModifier(): int {
        return this.accessMod
    }

    public getStaticValue(): Any {
        if (!this.isStatic()) {
            throw new AssertionError("Field value is not static!")
        }
        return TypeAPIGetStaticFieldValue(this.ownerType!, this.name)
    }

    public setStaticValue(v: Any): void {
        if (!this.isStatic()) {
            throw new AssertionError("Field value is not static!")
        }
        if (!this.getType().assignableFrom(Type.of(v))) {
            throw new AssertionError("Cannot set value " + v + " to " + this.getType())
        }
        TypeAPISetStaticFieldValue(this.ownerType!, this.name, this.getType().convertObject(v))
    }

    public isStatic(): boolean {
        return (this.attributes & Attributes.STATIC) != 0
    }

    public isReadonly(): boolean {
        return (this.attributes & Attributes.READONLY) != 0
    }

    public override toString(): string {
        return this.getName() + ": " + this.getType().toString()
    }

    public equals(oth: Any): boolean {
        return oth instanceof Field &&
                this.fieldType!.equals((oth as Field).fieldType!) &&
                this.name == (oth as Field).name &&
                this.accessMod == (oth as Field).accessMod &&
                this.attributes == (oth as Field).attributes
    }
}
