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

function loop1(a) {
  while (a < 0) {
    a -= 1;
  }
}

function loop2() {
  while (true) {
    print('infinite loop');
  }
}

function loop3() {
  let arr = [1, 2, 3];
  for (let n of arr) {
    for (let i = 0; i < n; i++) {
      print(i);
    }
  }
}

function loop4() {
  let a = 1;
  for (;;) {
    a += 1;
    if (a > 5) {
      break;
    }
  }
}

function if1(a) {
  if (a) {
    print('a');
  } else {
    print('b');
  }
}

function if2(a) {
  if (a > 0) {
    if (a > 1) {
      return true;
    }
  }
  return false;
}

function try1() {
  try {
    a = 1;
  } catch (e) {
    print(e);
  }
}

