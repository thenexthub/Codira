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

export @interface FieldAnno {
    names: String[] = ['name01', 'name02']
	addr: string = 'shanghai'
	d_int: int = 11
	d_long: long = 22
	d_float: float = 33.3f
	d_double: double = 55.55
}

export @interface MethodAnno {
    names: String[] = ['name01', 'name02']
	addr: string = 'shanghai'
	d_int: int = 11
	d_long: long = 22
	d_float: float = 33.3f
	d_double: double = 55.55
}

export @interface ClassAnno {
    names: String[] = ['name01', 'name02']
	addr: string = 'shanghai'
	d_int: int = 11
	d_long: long = 22
	d_float: float = 33.3f
	d_double: double = 55.55
}

export abstract class Base {
    abstract getName(): String;
}

const const_name: string = 'billy';
const const_int: int = 11;
const const_long: long = 22;
const const_float: float = 33.3f;
const const_double: double = 55.55;

@ClassAnno({names: ['test'], addr: 'taiwan', d_int: 11, d_long: 22, d_float: 33.3f, d_double: 55.55})
export class Cls extends Base {
    @FieldAnno({names: ['test'], addr: 'taiwan', d_int: 11, d_long: 22, d_float: 33.3f, d_double: 55.55})
    name: string = 'alice'
    name2: string = const_name
	d_int: int = 11
	d_int2: int = const_int
	d_long: long = 22
	d_long2: long = const_long
	d_float: float = 33.3f
	d_float2: float = const_float
	d_double: double = 55.55
	d_double2: double = const_double

    setName(name: string) {
        this.name = name
    }
    getName(): string {
        return this.name
    }

    calculate(a: int, b: int): int {
        if(a > b) {
            try {
                return a - b;
            } catch (e) {
                // CC-OFFNXT(G.AOD.03) for test purposes
                console.log('err')
                return a + b;
            }
        } else {
            return a * b;
        }
    }

    @MethodAnno({names: ['test'], addr: 'taiwan', d_int: 11, d_long: 22, d_float: 33.3f, d_double: 55.55})
    add(a: int, b: int): int {
        return a + b;
    }
}
