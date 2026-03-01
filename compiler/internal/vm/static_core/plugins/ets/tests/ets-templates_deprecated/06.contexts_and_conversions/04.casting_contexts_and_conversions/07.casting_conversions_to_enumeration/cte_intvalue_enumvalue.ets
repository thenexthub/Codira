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
desc: Casting conversions to intvalue to enumvalue.
tags: [compile-only, negative]
---*/

enum Status {
    Pending = 1,
    Active,
    Inactive
}

function convertIntToEnum(statusValue: number): string | undefined {
    let status: Status = statusValue as unknown as Status;
    return Status[status] !== undefined ? Status[status] : undefined;
}

function main(): void {
    console.log(convertIntToEnum(1)); //  "Pending"
    console.log(convertIntToEnum(2)); //  "Active"
    console.log(convertIntToEnum(3)); //  "Inactive"
    console.log(convertIntToEnum(0)); //  undefined
    console.log(convertIntToEnum(4)); //  undefined
}