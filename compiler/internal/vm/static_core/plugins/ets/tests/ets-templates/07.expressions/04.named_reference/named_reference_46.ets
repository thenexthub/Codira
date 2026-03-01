/*
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

/*---
desc: Type of a named reference expression is the type of the entity a named reference refers to.
files:
- ./named_reference_export4.ets
---*/

import {Shapes} from "./named_reference_export4.ets"

namespace AnotherShapes {
    export class Circle {}
    export class Square {}
}

class Clazz {
    circle: Shapes.Circle = new Shapes.Circle;
    anotherCircle: AnotherShapes.Circle = new AnotherShapes.Circle;
}

function main() {

    let q = new Shapes.Square;
    let a = new AnotherShapes.Square;
    let c = new Clazz;
    
}
