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

/*---
desc: A default method in an interface body may explicitly be declared private.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0139: Signature getHorsePower(rpm: Int, torque: Int): Int is not visible here."
---*/

interface Vehicle {
    private getHorsePower(rpm: int, torque: int): int {
        return (rpm * torque) / 5;
    }
}

class A implements Vehicle {
    public bar(): int {
        return this.getHorsePower(100, 2);  // CTE: not visible
    }
}
