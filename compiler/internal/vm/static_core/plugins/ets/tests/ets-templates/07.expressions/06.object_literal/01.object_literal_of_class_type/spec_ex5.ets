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

/*---
desc: >-
    If a class has accessors (see Accessor Declarations) for a property,
    and its setter is provided, then this property can be used as a part of an object literal.
---*/

class OK {
    set attr (attr: number) {}
}

function main(): void {
    const a: OK = {attr: 42} // OK, as the setter be called
    arktest.assertTrue(a instanceof OK)
}
