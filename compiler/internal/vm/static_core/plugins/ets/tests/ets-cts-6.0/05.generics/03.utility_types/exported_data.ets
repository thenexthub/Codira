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

/*---
desc: Exported class, interface and a class which implements an interface
tags:
- compile-only
- not-a-test
---*/

export let gs: int = 0 // getter-setter

export abstract class AC {
    i: number = 0;
    j: string = "";
}

export class C {
    i: number = 0;
    j: string = "";
}

export interface I {
    i: number;
    j: string;
}

export class CI implements I {
    _i: number = 0
    _j: string = ""
    set i(i: number) {gs+=1; this._i = i }
    get i(): number {gs+=2; return this._i }
    set j(j: string) {gs+=4; this._j = j }
    get j(): string {gs+=8; return this._j }
}

export class CA extends AC { }

// Same with optional params
export abstract class ACOPT {
    i?: number = 0;
    j?: string = "";
}

export class COPT {
    i?: number = 0;
    j?: string = "";
}

export interface IOPT {
    i?: number;
    j?: string;
}

export class CIOPT implements IOPT {
    _i?: number = 0
    _j?: string = ""
    set i(i: number|undefined) {gs+=1; this._i = i }
    get i(): number|undefined {gs+=2; return this._i }
    set j(j: string|undefined) {gs+=4; this._j = j }
    get j(): string|undefined {gs+=8; return this._j }
}

export class CAOPT extends ACOPT { }
