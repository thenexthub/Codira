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
'use static'
export declare let trueLiteral: boolean;
export declare let falseLiteral: boolean;
export declare function getTrue(): boolean;
export declare function getFalse(): boolean;
export declare function bar(): boolean;
export declare function foo(): boolean;
export declare const aaa: (b: Any) => boolean;
export declare class Test {
    a: Any;
    constructor();
}
export declare interface TestA {
    aaa(): boolean;
}
