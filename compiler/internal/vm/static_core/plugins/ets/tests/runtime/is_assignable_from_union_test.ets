/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class A {};

class B extends A {};

class C {};

class D {};

class E {};

type AD = A | D;

type BC = B | C;

type ABC = A | B | C;

type ACD = A | C | D;

type DE = D | E;

type ABC_DE = ABC | DE;

type ANull = A | null;

type BNull = B | null;

type ADNull = A | D | null;

interface InterfaceI {
    method_AD(): AD;
    method_BC(): BC;
    method_DE(): DE;
    method_ABC(): ABC;
    method_ACD(): ACD;
    method_ABC_DE(): ABC_DE;
    method_ADNull(): ADNull;
    method_ANull(): ANull;
    method_BNull(): BNull;
}

function isInstanceofUndefined(val: A | null | undefined): boolean {
    return val instanceof undefined;
}

function isInstanceofNotUndefined(val: A | null | undefined): boolean {
    return val instanceof (A | null);
}

function isInstanceofNull(val: A | null | undefined): boolean {
    return val instanceof null;
}

function isInstanceofNotNull(val: A | null | undefined): boolean {
    return val instanceof (A | undefined);
}

function isInstanceofObject(val: A | null | undefined): boolean {
    return val instanceof Object;
}
