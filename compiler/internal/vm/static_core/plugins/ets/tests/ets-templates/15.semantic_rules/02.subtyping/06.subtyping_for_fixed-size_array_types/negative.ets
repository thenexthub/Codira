/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{%- for c in cases %}

/*---
desc: >-
    Fixed-size array types are covariant and are formally defined as follows:
    FixedSize<B> <: FixedSize<A> if B <: A.
tags: [negative, compile-only]
---*/

interface I {}
interface J extends I { j: int }
class Base {}
class Derived extends Base {}
type CustomArr<T extends Base> = FixedArray<T>

let x: {{'CustomArr' if c.customtype else 'FixedArray'}}<{{c.dtype}}> = {{c.value}}
let y: FixedArray<{{c.stype}}> = x
x = y

{%- endfor %}
