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

function foo() {
    console.log("foo")
}

function bar() {
    console.log("bar")
}

class c1{}
function test(x:c1){
    if(x){
        console.log(1);
    }
}

function test2(a:double){
    console.log("test");
}


function main() {
    foo()
    test2(200000);
}

function test3(a:c1){
    if(typeof a === 'c1'){
        console.log("123");
    }
}

class ClassFloat{
    protected static float1:float =234.5f
}

class A{n:int=42}
class B{n:int=12}

function test4(x:A|B){
  x.n
}