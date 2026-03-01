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
    CTE Type of lhsExpression is readonly array (see Readonly Parameters), while the converted type of rhsExpression is a non-readonly array;
    TypeError Cannot modify an array or tuple content that has the readonly parameter
 tags: 'negative, compile-only'
 ---*/

 let tuple: readonly [number, string] = [1, "hello"]

 function main() {
    tuple[0] = 0.987
 }