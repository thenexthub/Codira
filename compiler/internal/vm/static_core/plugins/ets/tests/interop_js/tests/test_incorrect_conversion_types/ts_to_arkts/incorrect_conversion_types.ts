/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

interface TestInterface {
    testNumber: number;
    testString: string;
}

export const testInterface: TestInterface = {
        testNumber: 100,
        testString: 'Test',
};

export const tsString: string = 'Test';
export const tsNumberToByteLR: number = -129;
export const tsNumberToByteRR: number = 128;
export const tsNumberToIntLR: number = -2147483649;
export const tsNumberToIntRR: number = 2147483648;
export const tsNumberToShortLR: number = -32769;
export const tsNumberToShortRR: number = 32768;
export const tsNumberToLongLR: number = -9223372036854775809;
export const tsNumberToLongRR: number = 9223372036854775808;
export const tsNumberToCharLR: number = -1;
export const tsNumberToCharRR: number = 256;
export const tsBoolean: boolean = true;
export const tsNull: null = null;
export const tsUndefined: undefined = undefined;
export const tsBigInt: bigint = 0n;
export const tsArrayNum: number[] = [10, 20, 30];
export const tsArrayStr: string[] = ['red', 'green', 'blue'];
