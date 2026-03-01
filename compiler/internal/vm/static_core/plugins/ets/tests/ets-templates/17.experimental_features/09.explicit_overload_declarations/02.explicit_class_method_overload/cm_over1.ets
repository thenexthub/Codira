/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

{%- for c in cases %}
/*---
desc: >-
    Method overload declaration allows declaring an overload alias as a class member for a set of static or instance methods.
---*/

enum Color { Red, Green = 82, Blue }
enum Size { S = "small", M = "medium", L = "large" }

type T = [number, string];
type R = [number, boolean];
type FF = (p: number) => string;
type EE = (p: string) => int;

class X {}
class Y extends X {}
class Z extends Y {}

class A {
    m01(p: byte): string { return "byte" }
    m02(p: short): string { return "short" }
    m03(p: int): string { return "int" }
    m04(p: long): string { return "long" }
    m05(p: boolean): string { return "boolean" }
    m06(p: float): string { return "float" }
    m07(p: double): string { return "double" }
    m08(p: string): string { return "string" }
    m09(p: Color): string { return "Color" }
    m10(p: Size): string { return "Size" }
    m11(p: T): string { return "T" }
    m12(p: R): string { return "R" }
    m13(p: () => void): string { return "void-void" }
    m14(p: (p: number) => void): string { return "number-void" }
    m15(p: FF): string { return "FF" }
    m16(p: EE): string { return "EE" }
    m17(p: FixedArray<Color>): string { return "FA:Color" }
    m18(p: FixedArray<A>): string { return "FA:A" }
    m19(p: FixedArray<byte>): string { return "FA:byte" }
    m20(p: FixedArray<short>): string { return "FA:short" }
    m21(p: FixedArray<int>): string { return "FA:int" }
    m22(p: FixedArray<long>): string { return "FA:long" }
    m23(p: FixedArray<boolean>): string { return "FA:boolean" }
    m24(p: FixedArray<float>): string { return "FA:float" }
    m25(p: FixedArray<double>): string { return "FA:double" }
    m26(p: FixedArray<number>): string { return "FA:number" }
    m27(p: FixedArray<string>): string { return "FA:string" }
    m28(p: FixedArray<Size>): string { return "FA:Size" }
    m29(p: FixedArray<object>): string { return "FA:object" }
    m30(p: int, q: int): string { return "i+i" }
    m31(p: string, q: double): string { return "s+d" }
    m32(p: boolean, q?: object): string { return "b+o?" }
    m33(p: int, q: object[]): string { return "i+o[]" }
    m34(p: string, ...q: object[]): string { return "s+...o" }
    m35(p: Y, q: Z): string { return "Y+Z" }
    m36(p: X, q: Y): string { return "X+Y" }
    m37(p: Z): string { return "Z" }
    m38(p: X): string { return "X" }
    m39(p: object): string { return "object" }
    m40(...p: object[]): string { return "objects" }

    overload meth {
        m01, m02, m03, m04, m05, m06, m07, m08, m09, m10,
        m11, m12, m13, m14, m15, m16, m17, m18, m19, m20,
        m21, m22, m23, m24, m25, m26, m27, m28, m29, m30,
        m31, m32, m33, m34, m35, m36, m37, m38, m39, m40
    }
}

function main() {
    let a: A = new A();
    arktest.assertEQ(a.meth({{c.value}}), '{{c.result}}')
}
{%- endfor %}
