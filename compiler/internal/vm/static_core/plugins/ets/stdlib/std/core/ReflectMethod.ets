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

export namespace reflect {

/**
 * Represents method of class or interface
 */
class Method {
    protected name: string | undefined
    protected ownerType: Class
    protected methodPtr: long
    protected attributes: int
    protected accessMod: byte

    protected native constructor()

    public isNative(): boolean {
        return (this.attributes & Attributes.NATIVE) != 0
    }

    public isPublic(): boolean {
        return (this.accessMod == AccessModifier.PUBLIC)
    }

    public isPrivate(): boolean {
        return (this.accessMod == AccessModifier.PRIVATE)
    }

    public isProtected(): boolean {
        return (this.accessMod == AccessModifier.PROTECTED)
    }

    public getName(): string {
        return this.name ?? Method.getNameInternal(this.methodPtr)
    }

    public getOwner(): Class {
        return this.ownerType
    }

    public getReturnType(): Class {
        return Method.getReturnTypeImpl(this.methodPtr)!
    }

    public getParametersTypes(): FixedArray<Class> {
        return Method.getParametersTypesImpl(this.methodPtr)
    }

    public getParameterType(idx: int): Class | undefined {
        return Method.getParameterTypeByIdxImpl(this.methodPtr, idx)
    }

    public getParametersNum(): int {
        return Method.getParametersNumImpl(this.methodPtr)
    }

    protected invoke(thisObj: Object | undefined, args?: FixedArray<Any>): Any {
        this.ownerType.initialize()
        return this.invokeInternal(thisObj, args)
    }

    private static native getNameInternal(methodPtr: long): string
    private static native getReturnTypeImpl(methodPtr: long): Class | undefined
    private static native getParametersTypesImpl(methodPtr: long): FixedArray<Class>
    private static native getParameterTypeByIdxImpl(methodPtr: long, idx: int): Class | undefined
    private static native getParametersNumImpl(methodPtr: long): int
    private native invokeInternal(thisObj: Object | undefined, args?: FixedArray<Any>): Any
}

} // namespace reflect
