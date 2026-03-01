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

export class CustomInt {
    public static calculateNumberOfLeadingZeros(value: int): int {
        if (value <= 0) {
            return value == 0 ? 32 : 0;
        }

        let res: int = 31;
        for (let sh = 16; sh > 1; sh /= 2) {
            if (value >= (1 << sh)) {
                res -= sh;
                value >>>= sh;
            }
        }
        return res - (value >>> 1);
    }

    public static calculateNumberOfTrailingZeros(value: int): int {
        value = ~value & (value - 1);
        if (value <= 0) {
            return value & 32;
        }

        let res: int = 0;
        for (let sh = 16; sh > 1; sh /= 2) {
            if (value > (1 << sh)) {
                res += sh;
                value >>>= sh;
            }
        }
        return res + (value >>> 1);
    }
}
