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
desc:  {function: Engine_array}
---*/


interface Iface<T> {
    returnArray() : (T | undefined)[];
}

class Provider<T> implements Iface<T> {
    override returnArray(): (T | undefined)[] {
        return new (T | undefined)[5];
    }
}

function main() : int {
    let provider = new Provider<Engine<Propan>>();
    let result = provider.returnArray();
    if (result.length != 5) return 1;

    for (let i = 0; i< result.length; i++) {
        result[i] = new Engine<Propan>();
    }
    for (let i = 0; i< result.length; i++) {
        console.println(result[i]);
        if (!result[i]!.name.equals("Propan")) return 1;
    }
    return 0;
}

class Engine<T> {
    public name : T;
}


class Propan {
    constructor() {
	  this.name = "Propan"
    }
    public name: String
}
