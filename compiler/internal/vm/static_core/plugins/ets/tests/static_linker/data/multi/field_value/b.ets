/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { STR } from './a'

function check(a: string, expected: string): void {
    const a1 = a as Object | null | undefined;
    arktest.assertTrue(a1 !== null);
    arktest.assertTrue(a1 !== undefined);
    arktest.assertEQ(a1, expected);
}

function deeper(a: string): void {
    check(a, 'abc');
}

function main(): void {
    check(STR, 'abc');
    check('a' + STR, 'aabc');
    deeper(STR);
}
