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
    An explicit overload defined in a superinterface can be overridden in a subinterface.
    An explicit interface overload can be overridden in a class.
    Check that explicit overload can be inherited from a superinterface.
    Some interface methods overridden.
    No implicit overloading.
    Overload 'foo' is one of existing names.
---*/
class C1 {}
class C2 extends C1 {}
class C3 extends C2 {}

{{c.decl}}

function main() {
    {{c.main|indent}}
}
{%- endfor %}
