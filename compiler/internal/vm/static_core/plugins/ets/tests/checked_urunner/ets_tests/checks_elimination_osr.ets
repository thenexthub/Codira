/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! CHECKER      Checks Elimination in OSR mode, when there is an OSR entry between 2 NullCheck-s
//! RUN          options: "--no-async-jit --compiler-hotness-threshold=1 --compiler-enable-osr=true --compiler-ignore-failures=true", entry: "checks_elimination_osr.ETSGLOBAL::main"
//! METHOD       "checks_elimination_osr.ETSGLOBAL::find"
//! EVENT        /Compilation,checks_elimination_osr.ETSGLOBAL::find,0.*,COMPILED/
//! EVENT_NOT    /OsrEntry,checks_elimination_osr.ETSGLOBAL::find,.*,AFTER_IFRAME,SUCCESS/
// now try-catch doesn't work in osr mode. when it will be supported, there should be 3 NullCheck instructions
// because NullCheck from try block do not dominate another one, which should not be deleted by ChecksElimination

function find(arr: FixedArray<byte>, indexes: FixedArray<byte>, elem: int): int {
  let x : int = 0;
  try {
    __noinline__foo(arr.length, elem); // can throw error
    x = indexes[0]; // NullCheck can be not executed
  } catch (e) {
    arktest.assertTrue(false, "error, " + e.toString());
  }

  for (let i: int = 0; i < arr.length; ++i) {
    // OSR entry here
    if (arr[i] == elem) {
      x = i;
    }
  }
  return indexes[x]; // should not delete NullCheck
}

function __noinline__foo(length: int, elem: int) : void {
  if (elem >= length) {
    throw new ArrayIndexOutOfBoundsError("find failed: out of range");
  }
}

function main(): int {
  const byteArr: FixedArray<byte> = [10, 14, 7, 4, 0, 9, 6, 15, 3, 12, 2, 8, 5, 11, 13, 1];
  const indArr: FixedArray<byte> =  [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15];
  return find(byteArr, indArr, 2) - 10;
}