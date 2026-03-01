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
desc: A default method can be explicitly declared private in an interface body.
tags: [compile-only, negative]
---*/
interface Vehicle {
    private getHorsePower3(rpm: int, torque: int): int {
        return (rpm * torque) / 5252;
    }

    getPwrIndex(torque: int): int {
        return this.getHorsePower3(5252, torque);
    }
}

class Car implements Vehicle {
    private getHorsePower3(rpm: int, torque: int): int {
        return (rpm * torque) / 2626;
    }
}
