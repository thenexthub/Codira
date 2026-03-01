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

export declare function getFullName({ firstName, lastName }: {
    firstName: any;
    lastName: any;
}, { fff, asdsadsda }: {
    fff: any;
    asdsadsda: any;
}): void;

export function* foo() {
    yield 1;
}

export function* bar(arg: any) {
    yield arg;
}

export function UserInfo(name: string, age: number): void {
    name = 'xiao';
    age = 18;
}

export function gwtUserName(code: number): string {
    return code === 1 ? 'xiao' : 'zhu'
}