'use static'
/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
export declare let ub64: BigUint64Array;
export declare let b64: BigInt64Array;
export declare let ab: ArrayBuffer;
export declare let arr: number[];
export declare function fooArr(a: number[]): number[];
export declare let res: number[];
export declare function fooArr2(a: Array<string>): void;
export declare class C {
    static foo(a: Array<string>): void;
    bar(a: Array<string>): void;
}
export declare interface Iface {
    foo(a: Array<string>): void;
}
