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
desc: QualifiedName (see Names) is an expression that consists of dot-separated names.
files:
- ./named_reference_export4.ets
- ./named_reference_export6.ets
---*/

import {Shapes} from "./named_reference_export4.ets"
import {ShapeInterfaces} from "./named_reference_export6.ets"

class Clazz {
  static circle: Shapes.Circle = new Shapes.Circle;
  square: Shapes.Square = new Shapes.Square;
  maybeSquare?: Shapes.Square = undefined;
  circles: Shapes.Circle[] = [];
}

class Quadrupole extends Shapes.Square {}

class SomeShape implements ShapeInterfaces.Calculable {
  calculateArea(): number {
    return 13;
  }
}

function main() {

  let circle = new Shapes.Circle;
  let clazz = new Clazz;
  let more = new Quadrupole;
  let shape = new SomeShape;
  
}
