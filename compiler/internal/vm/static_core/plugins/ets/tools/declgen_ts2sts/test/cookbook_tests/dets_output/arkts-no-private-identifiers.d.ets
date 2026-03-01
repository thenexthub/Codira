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
declare class C {
    a: number;
    b: number;
    static c: string;
    static d: string;
    protected e: Map<Any, Any>;
    protected static f: Map<string, string>;
    j(): number;
    k(): number;
    static l(): string;
    static m(): string;
    protected o(): Map<Any, Any>;
    protected static p(): Map<string, string>;
}
declare class Singleton {
    static getInstance(): Singleton;
    someMethod(): void;
}
