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

let arr : int[] = [0];
function get_element() {
    return 0;
}

let str_arr : string[] = [""];
function get_string() {
    return "TEST";
}

class MyClass {}
let class_arr : MyClass[] = [new MyClass()];
function get_class_array_size() {
    return 0;
}

function main() {
    let elem : int = get_element();
    console.log(elem);
    let str : string = get_string();
    console.log(str);
    let size : int = get_class_array_size();
    console.log(size);
}

// After AOP:

// let arr : int[] = [0];
// function get_element() {
//     let a : int[] = [1, 2, 3];
//     return a[0];
// }

// let str_arr : string[] = [""];
// function get_string() {
//     let s : string[] = ["TEST1", "TEST2"];
//     return s[1];
// }

// class MyClass {}
// let class_arr : MyClass[] = [new MyClass()];
// function get_class_array_size() {
//     let c : MyClass[] = new MyClass[3];
//     c[0] = new MyClass();
//     c[1] = new MyClass();
//     c[2] = new MyClass();
//     return c.length;
// }

// function main() {
//     let elem : int = get_element();
//     console.log(elem);
//     let str : string = get_string();
//     console.log(str);
//     let size : int = get_class_array_size();
//     console.log(size);
// }
