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

class AccSumCount {
  sum: number;
  count: number;
  constructor(sum: number, count: number) {
    this.sum = sum;
    this.count = count;
  }
}

class AccProdSteps {
  prod: number;
  steps: number;
  constructor(prod: number, steps: number) {
    this.prod = prod;
    this.steps = steps;
  }
}

function main(): int {
  const suite = new arktest.ArkTestsuite("Uint8ClampedArrayReduce");

  // reduce – number, no initialValue, varying callback arity
  suite.addTest("reduce number/no init – full args", test_reduce_number_noInit_fullArgs);
  suite.addTest("reduce number/no init – 3 args", test_reduce_number_noInit_threeArgs);
  suite.addTest("reduce number/no init – 2 args", test_reduce_number_noInit_twoArgs);
  suite.addTest("reduce number/no init – 1 arg", test_reduce_number_noInit_oneArg);
  suite.addTest("reduce number/no init – 0 args", test_reduce_number_noInit_zeroArgs);

  // reduce – number with initialValue
  suite.addTest("reduce number/with init – full args", test_reduce_number_withInit_fullArgs);

  // reduce – generic<U=...> with initialValue, varying arity
  suite.addTest("reduce generic<U=string>/with init – full args", test_reduce_generic_string_fullArgs);
  suite.addTest("reduce generic<U=object>/with init – 3 args", test_reduce_generic_object_threeArgs);
  suite.addTest("reduce generic<U=boolean>/with init – 2 args", test_reduce_generic_boolean_twoArgs);
  suite.addTest("reduce generic<U=number>/with init – 1 arg", test_reduce_generic_number_oneArg);
  suite.addTest("reduce generic<U=number>/with init – 0 args", test_reduce_generic_number_zeroArgs);

  // reduce – index/array params + exception cases
  suite.addTest("reduce – index and array params", test_reduce_index_and_array_params);
  suite.addTest("reduce – throws on empty without init", test_reduce_throws_on_empty);

  // reduceRight – number, no initialValue, varying callback arity
  suite.addTest("reduceRight number/no init – full args", test_reduceRight_number_noInit_fullArgs);
  suite.addTest("reduceRight number/no init – 3 args", test_reduceRight_number_noInit_threeArgs);
  suite.addTest("reduceRight number/no init – 2 args", test_reduceRight_number_noInit_twoArgs);
  suite.addTest("reduceRight number/no init – 1 arg", test_reduceRight_number_noInit_oneArg);
  suite.addTest("reduceRight number/no init – 0 args", test_reduceRight_number_noInit_zeroArgs);

  // reduceRight – number with initialValue
  suite.addTest("reduceRight number/with init – full args", test_reduceRight_number_withInit_fullArgs);

  // reduceRight – generic<U=...> with initialValue, varying arity
  suite.addTest("reduceRight generic<U=string>/with init – full args", test_reduceRight_generic_string_fullArgs);
  suite.addTest("reduceRight generic<U=object>/with init – 3 args", test_reduceRight_generic_object_threeArgs);
  suite.addTest("reduceRight generic<U=boolean>/with init – 2 args", test_reduceRight_generic_boolean_twoArgs);
  suite.addTest("reduceRight generic<U=number>/with init – 1 arg", test_reduceRight_generic_number_oneArg);
  suite.addTest("reduceRight generic<U=number>/with init – 0 args", test_reduceRight_generic_number_zeroArgs);

  // reduceRight – order + exception
  suite.addTest("reduceRight – order verification", test_reduceRight_order_verification);
  suite.addTest("reduceRight – throws on empty without init", test_reduceRight_throws_on_empty);

  return suite.run();
}

/* reduce – number, no initialValue, varying callback arity */

function test_reduce_number_noInit_fullArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const sum = arr.reduce((prev: number, cur: number, idx: int, a: Uint8ClampedArray): number => prev + cur);
  arktest.assertEQ(sum, 10);
}

function test_reduce_number_noInit_threeArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const sum = arr.reduce((prev: number, cur: number, idx: int): number => prev + cur);
  arktest.assertEQ(sum, 6);
}

function test_reduce_number_noInit_twoArgs(): void {
  const arr = new Uint8ClampedArray([5, 2, 7]);
  const sum = arr.reduce((prev: number, cur: number): number => prev + cur);
  arktest.assertEQ(sum, 14);
}

function test_reduce_number_noInit_oneArg(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const res = arr.reduce((prev: number): number => prev + 1);
  // No initialValue => 3 calls for length=4, starting with first element as prev
  arktest.assertEQ(res, arr[0] + 3);
}

function test_reduce_number_noInit_zeroArgs(): void {
  const arr = new Uint8ClampedArray([9, 8, 7]);
  const res = arr.reduce((): number => 123);
  arktest.assertEQ(res, 123);
}

/* reduce – number with initialValue */

function test_reduce_number_withInit_fullArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const sum = arr.reduce((prev: number, cur: number, idx: int, a: Uint8ClampedArray): number => prev + cur, 10);
  arktest.assertEQ(sum, 20);
}

/* reduce – generic<U=...> with initialValue, varying arity */

function test_reduce_generic_string_fullArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const joined = arr.reduce<string>(
    (prev: string, cur: number, idx: int, a: Uint8ClampedArray): string => prev + `-${cur}@${idx}`,
    "start"
  );
  arktest.assertEQ(joined, "start-1@0-2@1-3@2");
}

function test_reduce_generic_object_threeArgs(): void {
  const arr = new Uint8ClampedArray([2, 4, 6]);
  const acc = arr.reduce<AccSumCount>((prev: AccSumCount, cur: number, idx: int): AccSumCount => {
    prev.sum += cur;
    prev.count += 1;
    return prev;
  }, new AccSumCount(0, 0));
  arktest.assertEQ(acc.sum, 12);
  arktest.assertEQ(acc.count, 3);
}

function test_reduce_generic_boolean_twoArgs(): void {
  const arr = new Uint8ClampedArray([1, 3, 5, 7]);
  const allOdd = arr.reduce<boolean>((prev: boolean, cur: number): boolean => prev && (cur % 2 !== 0), true);
  arktest.assertTrue(allOdd);
}

function test_reduce_generic_number_oneArg(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const calls = arr.reduce<number>((prev: number): number => prev + 1, 0);
  arktest.assertEQ(calls, 4);
}

function test_reduce_generic_number_zeroArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const res = arr.reduce<number>(() => 7, 100);
  arktest.assertEQ(res, 7);
}

/* reduce – index and array param verification */

function test_reduce_index_and_array_params(): void {
  const arr = new Uint8ClampedArray([10, 20, 30]);
  let seenSameArray = true;
  const visited: number[] = [];
  const sum = arr.reduce((prev: number, cur: number, idx: int, a: Uint8ClampedArray): number => {
    if (a !== arr) seenSameArray = false;
    visited.push(idx);
    return prev + cur;
  });
  arktest.assertTrue(seenSameArray);
  arktest.assertEQ(visited.length, 2); // no init => indexes 1,2
  arktest.assertEQ(visited[0], 1);
  arktest.assertEQ(visited[1], 2);
  arktest.assertEQ(sum, 60);
}

/* reduce – exceptions */

function test_reduce_throws_on_empty(): void {
  const empty = new Uint8ClampedArray(0);
  let threw = false;
  try {
    empty.reduce((p: number, c: number) => p + c);
  } catch (e) {
    threw = true;
  }
  arktest.assertTrue(threw);
}

/* reduceRight – number, no initialValue, varying callback arity */

function test_reduceRight_number_noInit_fullArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const diff = arr.reduceRight((prev: number, cur: number, idx: int, a: Uint8ClampedArray): number => prev - cur);
  // No initialValue: start prev=4; then 4-3=1; 1-2=-1; -1-1=-2
  arktest.assertEQ(diff, -2);
}

function test_reduceRight_number_noInit_threeArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const sum = arr.reduceRight((prev: number, cur: number, idx: int): number => prev + cur);
  arktest.assertEQ(sum, 6);
}

function test_reduceRight_number_noInit_twoArgs(): void {
  const arr = new Uint8ClampedArray([5, 2, 7]);
  const sum = arr.reduceRight((prev: number, cur: number): number => prev + cur);
  arktest.assertEQ(sum, 14);
}

function test_reduceRight_number_noInit_oneArg(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const res = arr.reduceRight((prev: number): number => prev + 1);
  arktest.assertEQ(res, arr[3] + 3);
}

function test_reduceRight_number_noInit_zeroArgs(): void {
  const arr = new Uint8ClampedArray([9, 8, 7]);
  const res = arr.reduceRight((): number => 50);
  arktest.assertEQ(res, 50);
}

/* reduceRight – number with initialValue */

function test_reduceRight_number_withInit_fullArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4]);
  const res = arr.reduceRight((prev: number, cur: number, idx: int, a: Uint8ClampedArray): number => prev + cur, 10);
  arktest.assertEQ(res, 20);
}

/* reduceRight – generic<U=...> with initialValue, varying arity */

function test_reduceRight_generic_string_fullArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const joined = arr.reduceRight<string>(
    (prev: string, cur: number, idx: int, a: Uint8ClampedArray): string => prev + `(${cur}<${idx})`,
    "R"
  );
  arktest.assertEQ(joined, "R(3<2)(2<1)(1<0)");
}

function test_reduceRight_generic_object_threeArgs(): void {
  const arr = new Uint8ClampedArray([2, 4, 6]);
  const acc = arr.reduceRight<AccProdSteps>((prev: AccProdSteps, cur: number, idx: int): AccProdSteps => {
    prev.prod *= cur;
    prev.steps += 1;
    return prev;
  }, new AccProdSteps(1, 0));
  arktest.assertEQ(acc.prod, 48);
  arktest.assertEQ(acc.steps, 3);
}

function test_reduceRight_generic_boolean_twoArgs(): void {
  const arr = new Uint8ClampedArray([2, 4, 6]);
  const anyEvenFromRight = arr.reduceRight<boolean>((prev: boolean, cur: number): boolean => prev || (cur % 2 === 0), false);
  arktest.assertTrue(anyEvenFromRight);
}

function test_reduceRight_generic_number_oneArg(): void {
  const arr = new Uint8ClampedArray([1, 2, 3, 4, 5]);
  const calls = arr.reduceRight<number>((prev: number): number => prev + 1, 0);
  arktest.assertEQ(calls, 5);
}

function test_reduceRight_generic_number_zeroArgs(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const res = arr.reduceRight<number>(() => -1, 999);
  arktest.assertEQ(res, -1);
}

/* reduceRight – order verification */

function test_reduceRight_order_verification(): void {
  const arr = new Uint8ClampedArray([1, 2, 3]);
  const left = arr.reduce((p: number, c: number) => p * 10 + c, 0);
  const right = arr.reduceRight((p: number, c: number) => p * 10 + c, 0);
  arktest.assertEQ(left, 123);
  arktest.assertEQ(right, 321);
}

/* reduceRight – exceptions */

function test_reduceRight_throws_on_empty(): void {
  const empty = new Uint8ClampedArray(0);
  let threw = false;
  try {
    empty.reduceRight((p: number, c: number) => p + c);
  } catch (e) {
    threw = true;
  }
  arktest.assertTrue(threw);
}
