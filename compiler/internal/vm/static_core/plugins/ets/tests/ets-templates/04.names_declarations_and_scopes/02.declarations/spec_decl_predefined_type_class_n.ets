/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Declarations must be distinguishable in a declaration space.
tags: [compile-only, negative]
expected_out: "Syntax error ESY0295: String is a predefined type, cannot be used as an identifier"
---*/

// CTE: Name of the declaration clashes with the predefined type or standard library entity name.

class String {}
