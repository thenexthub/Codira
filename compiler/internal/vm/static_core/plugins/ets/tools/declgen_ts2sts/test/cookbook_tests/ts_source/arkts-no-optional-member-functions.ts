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
export class A {
    fun?(): void
    bar(): void { }
    var1: number
    var2?: number
}
export class B extends A {
    fun?(): void { }
}
export abstract class C {
    abstract fun?(): string
    abstract foo(): void
    protected abstract bar?(): string
}
export class D {
    fun?(): string
    var1?: number
}
export class E {
    bar?(): B
    protected foo(arg: A): A { return arg }
    private fun1?(): string
    var1?: A
    var2: B
}
export class F extends E {
    bar(): B { return new B() }
    foo(arg: A): A { return new A() }
    fun2?(): string { return "123" }
}
export class G extends C {
    fun(): string { return "123" }
    foo(): void { }
    bar?(): string { return "123" }
}
