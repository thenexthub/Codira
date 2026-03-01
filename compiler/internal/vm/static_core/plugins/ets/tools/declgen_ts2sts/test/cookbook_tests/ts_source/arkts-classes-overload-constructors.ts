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
export class Dummy {
    constructor(param0: string);
    constructor(param0: number, param1?: boolean);
    constructor(param0: number | string, param1?: boolean) {}
}

export class Test {
    constructor();
    constructor(async?: boolean);
    constructor(syncFlags: number, waitTime: number);
    constructor(param0: string, param1: number);
    constructor(param0: string, param1: number, param2: string);
    constructor(syncFlags?: number, waitTime?: number);
    constructor(param0?: boolean | number | string, param1?: number, param2?: string) {
    }
}