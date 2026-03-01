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

import { CustomInt } from './CustomInt';

export class CustomLong {
    public static calculateNumberOfLeadingZeros(value: long): int {
        const asInt: int = (value >>> 32).toInt();
        return asInt == 0 ? 32 + CustomInt.calculateNumberOfLeadingZeros(value.toInt())
            : CustomInt.calculateNumberOfLeadingZeros(asInt);
    }

    public static calculateNumberOfTrailingZeros(value: long): int {
        const asInt: int = value.toInt();
        return asInt == 0 ? 32 + CustomInt.calculateNumberOfTrailingZeros((value >>> 32).toInt())
            : CustomInt.calculateNumberOfTrailingZeros(asInt);
    }

    public static main() {
        let l: long = 128;
        arktest.assertEQ(CustomLong.calculateNumberOfTrailingZeros(l), 6);
        arktest.assertEQ(CustomLong.calculateNumberOfLeadingZeros(l), 56);
    }
}
