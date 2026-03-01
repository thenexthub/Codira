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

function main(): int {
  let failures: int = 0;

  failures += test(createEmptyArrayBuffer(), "Create empty ArrayBuffer");
  failures += test(createArrayBufferOfVariousLengths(), "Create ArrayBuffer of Various lengths");
  failures += test(sliceArrayBufferInt(), "Slice ArrayBuffer with various int parameters");

  return failures == 0 ? 0 : 1;
}
/**
 *  Check that created empty ArrayBuffer has zero byteLength
 */
function createEmptyArrayBuffer(): int {
  let a = new ArrayBuffer(0);
  if (a.byteLength != 0 || a.detached)
    return 1;
  return 0;
}

/**
 * Check created ArrayBuffer with different lengths
 */
function createArrayBufferOfVariousLengths(): int {
  let a = new ArrayBuffer(10);

  if (a.byteLength != 10)
    return 1;

  try {
    a = new ArrayBuffer(-1);
    return 1;
  }
  catch(e) {
    if (!(e instanceof RangeError))
      return 1;
  }

  return 0;
}

/**
* Check slice() method of ArrayBuffer with different int parameters
*/
function sliceArrayBufferInt(): int {
  let a = new ArrayBuffer(8);
  
  let start: number = 5;
  let end: number = 4;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  start = 10;
  end = 8;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  start = 1;
  end = 12;
  if (a.slice(start, end).byteLength != 7)
    return 1;
  
  start = 2;
  end = -4;
  if (a.slice(start, end).byteLength != 2)
    return 1;
  
  start = 2;
  end = -10;
  if (a.slice(start, end).byteLength != 0)
    return 1;
  
  start = 0;
  end = 4.5;
  if (a.slice(start, end).byteLength != 4)
    return 1;
  
  start = 4.5;
  end = 8;
  if (a.slice(start, end).byteLength != 4)
    return 1;
  
  return 0;
}

function test(result: int, message: String): int {
  if(result == 0) {
    console.println("PASSED: " + message);
    return 0;
  }
  console.println("FAILED: " + message);
  return 1;
}
