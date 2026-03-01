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

type KeyType = 'day' | 'month' | 'year';

export let myRecord: Record<KeyType, number | string> = {
    day: 199999999,
    month: 'two',
    year: 3
};

export let originalRecord: Record<KeyType, number | string> = {
    day: 1,
    month: 2,
    year: 3
};

export function testNewRecordValueFromStatic(): boolean {
    let res = true;
    try {
        ASSERT_TRUE(originalRecord.day === 'one');
        ASSERT_TRUE(originalRecord.month === 'two');
        ASSERT_TRUE(originalRecord.year === 'three');
    } catch (e) {
        res = false;
    }
    return res;
}