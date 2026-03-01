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
desc: Casting conversions enum to float.
---*/

enum Status {
  Active = 3_000_001,
  Inactive = 3_000_002,
  Pending = 3_000_003
}

let activeFloatValue: number = Status.Active;
let inactiveFloatValue: number = Status.Inactive;
let pendingFloatValue: number = Status.Pending;

let expectedActiveValue: number = 3000001.0f;
let expectedInactiveValue: number = 3000002.0f;
let expectedPendingValue: number = 3000003.0f;

function main(): void {
  arktest.assertEQ(activeFloatValue, expectedActiveValue)
  arktest.assertEQ(inactiveFloatValue, expectedInactiveValue)
  arktest.assertEQ(pendingFloatValue, expectedPendingValue)
}