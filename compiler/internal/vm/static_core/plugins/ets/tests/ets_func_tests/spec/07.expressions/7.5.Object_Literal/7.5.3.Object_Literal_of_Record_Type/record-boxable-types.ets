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
desc: 7.5.3 Object Literal of Record Type
tags: []
---*/

let string_number: Record<String, Number> = {
  "hello": 10,
  "world": 20,
};

arktest.assertEQ( string_number.size, 2)
arktest.assertEQ( string_number.get("hello"), 10)
arktest.assertEQ( string_number.get("world"), 20)

let string_double: Record<String, Double> = {
  "hello": 10,
  "world": 20,
};

arktest.assertEQ( string_double.size, 2)
arktest.assertEQ( string_double.get("hello"), 10)
arktest.assertEQ( string_double.get("world"), 20)

let string_float: Record<String, Float> = {
  "hello": 10,
  "world": 20,
};

arktest.assertEQ( string_float.size, 2)
arktest.assertEQ( string_float.get("hello"), 10)
arktest.assertEQ( string_float.get("world"), 20)

let string_long: Record<String, Long> = {
  "hello": 10,
  "world": 20,
};

arktest.assertEQ( string_long.size, 2)
arktest.assertEQ( string_long.get("hello"), 10)
arktest.assertEQ( string_long.get("world"), 20)

let string_int: Record<String, Int> = {
  "hello": 10,
  "world": 20,
};

arktest.assertEQ( string_int.size, 2)
arktest.assertEQ( string_int.get("hello"), 10)
arktest.assertEQ( string_int.get("world"), 20)

function main() {
}
