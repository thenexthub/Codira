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

native function TypeAPIMethodInvoke(methodType: MethodType, recv: Any, args: FixedArray<Any>): Any
native function TypeAPIMethodInvokeConstructor(methodType: MethodType, args: FixedArray<Any>): Object

/**
 * Represents method of class or interface
 */
export class Method extends Object {
    private methodType: MethodType | undefined // not undefined if already created
    private ownerType: Type | undefined // not undefined if already created
    private name: string = ""
    private attributes: int
    private accessMod: byte

    private constructor () {}

    /**
     * Returns function type of this method
     *
     * @returns method's {@link MethodType}
     */
    public getType(): MethodType {
        return this.methodType!
    }

    /**
     * Returns the type in which the method is declared
     *
     * @returns {@link Type} that is method's owner type
     */
    public getOwnerType(): Type {
        return this.ownerType!
    }

    public getName(): string {
        return this.name
    }

    public isStatic(): boolean {
        return (this.attributes & Attributes.STATIC) != 0
    }

    public isFinal(): boolean {
        return (this.attributes & Attributes.FINAL) != 0
    }

    public isAbstract(): boolean {
        return (this.attributes & Attributes.ABSTRACT) != 0
    }

    public isConstructor(): boolean {
        return (this.attributes & Attributes.CONSTRUCTOR) != 0
    }

    public isGetter(): boolean {
        return (this.attributes & Attributes.GETTER) != 0
    }

    public isSetter(): boolean {
        return (this.attributes & Attributes.SETTER) != 0
    }

    public isNative(): boolean {
        return (this.attributes & Attributes.NATIVE) != 0
    }

    public isAsync(): boolean {
        return (this.attributes & Attributes.ASYNC) != 0
    }


    public override toString(): string {
        return this.getName() + ": " + this.getType().toString()
    }

    public invoke(recv: Any, args: FixedArray<Any>): Any {
        const isCtor = this.isConstructor()
        const isStatic = this.isStatic()
        if ((isStatic || isCtor) != (recv == null)) {
            throw new Error("reciever may be null only for static/ctor methods")
        }
        const thisType = this.getType()
        if (thisType.getParametersNum() != args.length) {
            throw new Error("arguments length mismatch " + thisType.getParametersNum() + " != " + args.length)
        }
        let convertedArgs: FixedArray<Any> = new FixedArray<Any>(args.length)
        for (let i = 0; i < convertedArgs.length; i++) {
            if (args[i] === null && thisType.getParameter(i).getType().equals(ObjectType)) {
                convertedArgs[i] = args[i]
            } else if (args[i] === undefined) {
                convertedArgs[i] = args[i]
            } else {
                convertedArgs[i] = thisType.getParameter(i).getType().convertObject(args[i])
            }
        }
        if (isCtor) {
            return TypeAPIMethodInvokeConstructor(this.methodType!, convertedArgs)
        } else {
            if (!isStatic) {
                recv = thisType.getReceiverType().convertObject(recv)
            }
            return TypeAPIMethodInvoke(this.methodType!, recv, convertedArgs)
        }
    }

    public getAttributes(): int {
        return this.attributes
    }

    public getAccessModifier(): int {
        return this.accessMod
    }

    public equals(oth: Any): boolean {
        return oth instanceof Method &&
                this.methodType!.equals((oth as Method).methodType!) &&
                this.name == (oth as Method).name &&
                this.accessMod == (oth as Method).accessMod &&
                this.attributes == (oth as Method).attributes
    }
}
