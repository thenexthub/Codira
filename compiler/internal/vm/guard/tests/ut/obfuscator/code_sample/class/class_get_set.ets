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

interface Style1 {
    get color(): string
    set color(s: string)
}
class StyleClass1 implements Style1 {
    public color: string = ''
}
class StyleClass2 implements Style1 {
    private color_: string = ''
    get color(): string {
        return this.color_
    }
    set color(s: string) {
        this.color_ = s
    }
}
class StyleClass3 implements Style1 {
    private colorClass3: string = ''
    get color(): string { return this.colorClass3 }
    set color(s: string) { this.colorClass3 = s }
}

let red = new StyleClass1();
red.color = 'red';
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1 ', red.color === 'red');
let blue = new StyleClass2();
blue.color = 'blue';
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1 ', blue.color === 'blue');
let green = new StyleClass3();
green.color = 'green';
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1 ', green.color === 'green');

interface Style2 {
    get color(): string
    readonly readable: number
}
class Style2Class implements Style2 {
    get color(): string {
        return 'Black';
    }
    set color(s: string) { }
    public readable: number = 0;
}
function howToWrite(s: Style2) {
    if (s instanceof Style2Class) {
        let s1 = s as Style2Class
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('test2 ', s1.color === 'Black')
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('test2 ', s1.readable === 0)

        s1.color = 'Blue'
        s1.readable = 42
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('test2 ', s1.color === 'Black');
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('test2 ', s1.readable === 42);
    }
}
howToWrite(new Style2Class)
