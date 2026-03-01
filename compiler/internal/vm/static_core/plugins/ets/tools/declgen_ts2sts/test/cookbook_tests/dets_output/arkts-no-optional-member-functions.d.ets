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
export declare class A {
    bar(): void;
    var1: number;
    var2?: number;
}
export declare class B extends A {
}
export declare abstract class C {
    abstract foo(): void;
}
export declare class D {
    var1?: number;
}
export declare class E {
    protected foo(arg: A): A;
    var1?: A;
    var2: B;
}
export declare class F extends E {
    bar(): B;
    foo(arg: A): A;
}
export declare class G extends C {
    fun(): string;
    foo(): void;
}
