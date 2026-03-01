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


try {
  try {
    a = 1;
  } catch (e) {
    a;
  }
  if (a == "") {
    throw "null";
  }
  if (x > 100) {
    throw "max";
  } else {
    throw "min";
  }
}
catch (err) {
  func0(a, 10);
}
finally {
  console.log("error");
}

var car = ["B", "V", "p", "F", "A"];
var text = "";
var i;
for (i = 0; i < 5; i++) {
  text += car[i] + x + y;
}

function func0(x, y) {
  var a = x + y;
  var text = "";
  if (isNaN(x)) {
    text = "number is invalid";
  } else {
    text = (x < 18) ? "young" : "old";
  }
  return a;
}

function func1(x, y) {
  var arr = ["B", "V", "p", "F", "A"];
  let text = "";
  for (let x of arr) {
    text += x;
  }
  while (i < 5) {
    text += "The number is " + i;
    i++;
  }
  do {
    text += "The number is " + i;
    i++;
  }
  while (i < 5);
}

function func2(x, y) {
  var arr = ["B", "V", "p", "F", "A"];
  for (let x of arr) {
    y += x;
  }
  var arr = ["B", "V", "p", "F", "A"];

  for (let x of arr) {
    y += x;
  }
  return y;
}
