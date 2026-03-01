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
desc: Casting conversions enum to double.
---*/

enum Status {
  Active = 10,
  Inactive = 20,
  Pending = 30
}

let activeDoubleValue: number = Status.Active;
let inactiveDoubleValue: number = Status.Inactive;
let pendingDoubleValue: number = Status.Pending;

let expectedActiveValue: number = 10.0;
let expectedInactiveValue: number = 20.0;
let expectedPendingValue: number = 30.0;

function main(): void {
  arktest.assertEQ(activeDoubleValue, expectedActiveValue)
  arktest.assertEQ(inactiveDoubleValue, expectedInactiveValue)
  arktest.assertEQ(pendingDoubleValue, expectedPendingValue)
}