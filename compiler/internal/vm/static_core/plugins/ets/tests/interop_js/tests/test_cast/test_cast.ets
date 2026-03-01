/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

let module = ESValue.load('js_code')
let funcArgType = module.getProperty('funcArgType');

function TestFuncCallArg_Pos(): boolean {
  let arg12: "literal" = "literal";
  if (funcArgType.invoke(arg12).toString() == "literal") {
    return true;
  } else {
    return false;
  }
}

function TestFuncCallArg_Neg(): boolean {
  let arg12: "literal" = "literal";
  if (funcArgType.invoke(arg12).toString() == "literal2") {
    return true;
  } else {
    return false;
  }
}