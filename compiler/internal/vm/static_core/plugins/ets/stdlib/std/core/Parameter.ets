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

/**
 * Represents parameter of function type
 */
export final class TypeAPIParameter extends Object {
    private paramType: Type | undefined // not undefined if already created
    private name: string = ""
    private attributes: int

    private constructor () {}

    public getType(): Type {
        return this.paramType!
    }

    public getName(): string {
        return this.name
    }

    public getAttributes(): int {
        return this.attributes
    }

    public isRest(): boolean {
        return (this.attributes & Attributes.REST) != 0
    }

    public isOptional(): boolean {
        return (this.attributes & Attributes.OPTIONAL) != 0
    }

    public override toString(): string {
        return this.getName() + ": " + this.getType().toString()
    }

    public equals(oth: Any): boolean {
        return oth instanceof TypeAPIParameter &&
                this.paramType!.equals((oth as TypeAPIParameter).paramType!) &&
                this.name == (oth as TypeAPIParameter).name &&
                this.attributes == (oth as TypeAPIParameter).attributes
    }
}
