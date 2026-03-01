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
desc: "A string object is immutable, the value of a string object cannot be changed\
  \ after the object is created. The value of a string object can be shared. Type\
  \ string has dual semantics, i.e.: \u2022 Type string behaves like a reference type\
  \ (see Reference Types) if created, assigned, or passed as an argument; \u2022 Type\
  \ string is handled as a value (see Value Types) by all string operations (see String\
  \ Concatenation, Equality Expressions, and String Relational Operators)."
name: string_as_reference_0
---*/

function main(): void {
  let a: string = "abc";
    let b: string = a;

    arktest.assertEQ( a, b)
    arktest.assertEQ( GC.getObjectAddress(a), GC.getObjectAddress(b))
}
