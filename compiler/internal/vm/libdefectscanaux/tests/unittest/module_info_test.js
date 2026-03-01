/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


// default import
import var1 from './mod1';
let a = var1;

// namespace import
import * as ns from './mod2';
ns.add(2);

// regular import
import { var3 } from '../mod3';
a = var3;

// rename import
import { var4 as localVar4 } from '../../mod4';
a = localVar4;

// export variable
export let var5;
var5++;

let var6 = 100;
var6 = var6 + 1;
export { var6 };

// export class
class InnerUserInput {
  getTextBase() { }
}
class UserInput extends InnerUserInput {
  getText() { }
}
export { UserInput as UInput };

// export function
function func3(num1, num2) {
  return num1 + num2;
}

v6 = func3(v5 + v6);

export { func3 as exFunc3 };

export default function func1() { }
export function func2() { }

// indirect export
export { v5 as v } from './mod5';
export * from './mod6';
export { foo } from '../../mod7';
