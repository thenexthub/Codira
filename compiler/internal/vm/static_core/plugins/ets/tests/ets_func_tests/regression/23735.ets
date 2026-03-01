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

/*---
desc: Issue 23735 Crash using array of interfaces in namespace
tags: []
---*/

export namespace arraystore {
  export interface MyInterface {
      value: number;
      prompt: String;
  }

}
type Alias = arraystore.MyInterface

function receiver(value: arraystore.MyInterface) {
}

function provider(): arraystore.MyInterface {
    return {value: 45, prompt: "Hello World"}
}

function spread(...values: arraystore.MyInterface[]): number {
    return values.length
}

const GLOB: arraystore.MyInterface = {value: 40, prompt: "Hello World"}

type TLOUT = () => arraystore.MyInterface
type TLINOUT = (value: arraystore.MyInterface) => arraystore.MyInterface

function main() {
  let item: arraystore.MyInterface =  {value: 43, prompt: "Hello World"}
  const alias: Alias = {value: 44, prompt: "Hello World"}

  const tlt: TLOUT = () => { return {value: 46, prompt: "Hello World"} }

  const tlo: TLINOUT = (value: arraystore.MyInterface) => { return value }

  let items: arraystore.MyInterface[] = [{value: 42, prompt: "Hello World"}, item, alias, provider(), tlt() , tlo(provider()), GLOB ]
  receiver(items[0])

  arktest.assertEQ(46, items[4].value)
  arktest.assertEQ(7, spread(...items))

}

