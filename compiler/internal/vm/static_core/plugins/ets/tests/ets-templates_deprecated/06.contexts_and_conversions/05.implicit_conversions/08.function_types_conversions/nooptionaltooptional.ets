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
desc: Casting conversions non-optional to optional.
tags: [compile-only, negative]
---*/

function main(): void {
  type RequiredParamFunc = (x: number) => void;
  type OptionalParamFunc = (x?: number) => void;

  const requiredFunc: RequiredParamFunc = (x: number) => {
    console.log(`Value: ${x.toFixed(2)}`);
  };

  const optionalFunc: OptionalParamFunc = requiredFunc;

  let output: string = "";

  const originalConsoleLog = console.log;

  console.log = (message: string) => { output = message; };

  optionalFunc(42);

  if (output === "Value: 42.00") {
    console.log("Test passed! Function with required parameter works when argument is provided.");
  } else {
    console.error(`Test failed! Expected "Value: 42.00", but got "${output}"`);
  }

  try {
    optionalFunc();
  } catch (error) {
    console.error("Runtime error:", error.message);
  }
  console.log = originalConsoleLog;
}
