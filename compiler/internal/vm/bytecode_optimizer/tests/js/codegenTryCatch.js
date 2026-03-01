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


function func1() {
  let a = 1.5;
  try {
    a += 1;
  } catch (e) {
    a++;
  }
  a++;
  return a;
}

function func2(a, b, c) {
  var a = 1;
  if (a) {
    return a;
  }
  else {
    a += 1;
  }
}

function func3(a, b, c, d, e) {
  return a + b + c + d + e;
}

function func4(x, y) {
  var a = x + y;
  var b = x - y;
  var c = x * y;
  var d = x / y;
  var e = x % y;
  return func3(e, d, c, b, a);
}

function func5(i, j, l) {
  for (var i = 0; i <= 4; i++) {
    for (var j = 0; j <= 4; j++) {
      for (var k = 1; k <= 2; k++) {
        System.out.printf(Loadstring);
      }
    }
    func4(x, y);
  }
}

function func6()
{
  return 'hello world';
}
