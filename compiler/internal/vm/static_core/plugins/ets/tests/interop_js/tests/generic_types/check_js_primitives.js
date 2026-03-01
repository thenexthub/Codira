/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

const { checkGenericValue } = require('generic_types.test.abc');

const FIX_BIGINT = true;

// Null
checkGenericValue(null);

// Undefined
checkGenericValue(undefined);

// Boolean
checkGenericValue(false);
checkGenericValue(true);
checkGenericValue(Boolean(false));
checkGenericValue(Boolean(true));

// Number
checkGenericValue(234);
checkGenericValue(4.234);
checkGenericValue(Number(34));
checkGenericValue(Number(-643.23566));

// BigInt
FIX_BIGINT && checkGenericValue(9007199254740991n);
FIX_BIGINT && checkGenericValue(BigInt('0b11010101'));

// String
checkGenericValue('abcdefg');
checkGenericValue(String('ABCDEFG'));
