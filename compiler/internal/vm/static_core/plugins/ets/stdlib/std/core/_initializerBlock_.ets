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

package std.core;
// Note: This file initialize all the `const` property that non-immediately initialized(spec-issue 217).
static {
    FROM_BUFFER = new FromBuffer()
    // 5 lower bits stores kind's id
    TypeKindMask = ((1 << 6) - 1).toByte();
    BigIntType = Class.from<BigInt>();
    ObjectType = Type.of(new Object()) as ClassType;
    console = Console.getInstance();
    coroutine = new Coroutine();
    mainEAWorker = new MainEAWorker();

    MAX_REGULAR_INTEGER = new BigInt(Number.MAX_SAFE_INTEGER)
    FAST_TO_STRING_HANDLER_MAP = () => {
        let result = new ToStringHandlerMap
        result[2] = (instance: BigInt):string => { return instance.toStringAsBinary() }
        result[4] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(4) }
        result[8] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(8) }
        result[10] = (instance: BigInt):string => { return instance.toStringAsDecimal() }
        result[16] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(16) }
        result[32] = (instance: BigInt):string => { return instance.toStringAsPowerOfTwoRadix(32) }
        return result
    }()

    dayNames = [
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    ];

    monthNames = [
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    ];

    firstDayInMonthNormal = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334];
    firstDayInMonthLeap = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335];

    maxDateOffset = 8.64e15.toLong();
    INVALID_DATE = maxDateOffset + 1;

    maxDateYears = 275760;
}
