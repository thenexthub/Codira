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

// Before AOP:
class C2{
    public a:string;
    constructor(str:string){
        this.a = str;
    }
}

class C1{
    public field1:int =1;
    public field2: number = 2.4;
    public field3: boolean =true;
    public field4:C2 = new C2('c2 created by c1');
}

function loadfield1() : int{
    let klass = new C1();
   return 0;
}
function loadfield2() : number{
    let klass = new C1();
   return 0.0;
}
function loadfield3() : boolean{
    let klass = new C1();
   return false;
}
function loadfield4(): C2 {
    let klass = new C1();
    return new C2('test');
}

function main() {
    console.log(loadfield1());
    console.log(loadfield2());
    console.log(loadfield3());
    console.log(loadfield4());
}

