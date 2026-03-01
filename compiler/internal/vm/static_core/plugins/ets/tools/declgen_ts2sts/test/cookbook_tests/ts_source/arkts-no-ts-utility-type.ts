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

export type LowercaseGreeting = "hello, world";
export type Greeting = Capitalize<LowercaseGreeting>;
export type QuietGreeting = Lowercase<Greeting>;
export type ShoutyGreeting = Uppercase<QuietGreeting>;
export type UncomfortableGreeting = Uncapitalize<ShoutyGreeting>;

export class UserIns {
    name: string;
    age: number;
    constructor(name: string, age: number) {
        this.name = name
        this.age = age
    };
}
export function createInstance<T extends new (...args: any[]) => any>(Class: T, ...args: ConstructorParameters<T>): InstanceType<T> {
    return new Class(...args);
}

export type T0 = Exclude<"a" | "b" | "c", "a">;
export type T1 = Extract<"a" | "b" | "c", "a" | "f">;
export type T3 = NonNullable<string | number | undefined>;
export type T4 = InstanceType<typeof UserIns>;
export type T5 = Parameters<(s: string) => void>;
export type T6 = ReturnType<() => string>;

export type NoInfer<T> = T & { [N in keyof T]?: never };
export function createStreetLight<C extends string>(colors: C[], defaultColor?: NoInfer<C>) { }

export type UserPreview = Omit<UserIns, "age">;

export function toHex(this: Number) {
    return this.toString(16);
}
export const fiveToHex: OmitThisParameter<typeof toHex> = toHex.bind(5);

export function numberToString(n: ThisParameterType<typeof toHex>) {
    return toHex.apply(n);
}
export type TodoPreview = Pick<UserIns, "name">;

export function makeObject<T>(obj: T & ThisType<T>): T {
    return obj;
}
