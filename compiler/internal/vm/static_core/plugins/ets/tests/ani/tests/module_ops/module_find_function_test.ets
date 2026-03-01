/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

export function getInitialIntValue(): int {
    return 0;
}

export namespace ops {
    function getIntValueOps(): int {
        return 0;
    }
}

function getIntValue(i: int): int {
    return 0;
}
function getIntValue(i: String): int {
    return 1;
}
function getIntValue(): int {
    return 2;
}

export function getInitialStringValue(): String {
    return "a";
}

function overloaded(i: int): int {
    return 0
}
function overloaded(): int {
    return 1
}

function getIntValueA(i: int): void {}
function getIntValueB(i: int): void {}
function getIntValueC(i: int): void {}
function getIntValueD(i: int): void {}
function getIntValueE(i: int): void {}
function getIntValueF(i: int): void {}
function getIntValueG(i: int): void {}
function getIntValueH(i: int): void {}
function getIntValueI(i: int): void {}
function getIntValueJ(i: int): void {}

class Test
{
    public getIntValue(): int {
        return 0;
    }
    public getIntValueA(): int {
        return 0;
    }
};
class TestA extends Test {
    public static addIntValue(a: int, b: int): int {
        return a + b;
    }
    public getIntValueA(): int {
        return 0;
    }
};

async function async_f(a: int): Promise<int> {
    return a;
}

function SetIntValueJ(i: int): void {}

