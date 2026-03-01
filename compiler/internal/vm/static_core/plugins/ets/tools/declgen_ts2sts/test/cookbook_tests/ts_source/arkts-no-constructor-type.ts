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

export class UserIns {
    name: string;
    age: number;
    constructor(name: string, age: number) {
        this.name = name
        this.age = age
    };
}
export function foo<T extends new (...args: any[]) => any>(Class: T): void { }
export let v1: new (...args: any[]) => any = UserIns;
export let v2: new (name: string, age: number) => { name: string; age: number };
