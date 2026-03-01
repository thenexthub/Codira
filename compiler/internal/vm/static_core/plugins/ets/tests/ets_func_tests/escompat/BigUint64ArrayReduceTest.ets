/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

class AccSumCount {
  public sum: bigint;
  public count: number;
  public constructor(sum: bigint, count: number) {
    this.sum = sum;
    this.count = count;
  }
}

class AccProdSteps {
  public prod: bigint;
  public steps: number;
  public constructor(prod: bigint, steps: number) {
    this.prod = prod;
    this.steps = steps;
  }
}

function main(): int {
  const suite = new arktest.ArkTestsuite('BigUint64ArrayReduce');

  // reduce – bigint, no initialValue, varying callback arity
  suite.addTest('reduce bigint/no init – full args', testReduceBigintNoInitFullArgs);
  suite.addTest('reduce bigint/no init – 3 args', testReduceBigintNoInitThreeArgs);
  suite.addTest('reduce bigint/no init – 2 args', testReduceBigintNoInitTwoArgs);
  suite.addTest('reduce bigint/no init – 1 arg', testReduceBigintNoInitOneArg);
  suite.addTest('reduce bigint/no init – 0 args', testReduceBigintNoInitZeroArgs);

  // reduce – bigint with initialValue
  suite.addTest('reduce bigint/with init – full args', testReduceBigintWithInitFullArgs);

  // reduce – generic<U=...> with initialValue, varying arity
  suite.addTest('reduce generic<U=string>/with init – full args', testReduceGenericStringFullArgs);
  suite.addTest('reduce generic<U=object>/with init – 3 args', testReduceGenericObjectThreeArgs);
  suite.addTest('reduce generic<U=boolean>/with init – 2 args', testReduceGenericBooleanTwoArgs);
  suite.addTest('reduce generic<U=bigint>/with init – 1 arg', testReduceGenericBigintOneArg);
  suite.addTest('reduce generic<U=bigint>/with init – 0 args', testReduceGenericBigintZeroArgs);

  // reduce – index/array params + exception cases
  suite.addTest('reduce – index and array params', testReduceIndexAndArrayParams);
  suite.addTest('reduce – throws on empty without init', testReduceThrowsOnEmpty);

  // reduceRight – bigint, no initialValue, varying callback arity
  suite.addTest('reduceRight bigint/no init – full args', testReduceRightBigintNoInitFullArgs);
  suite.addTest('reduceRight bigint/no init – 3 args', testReduceRightBigintNoInitThreeArgs);
  suite.addTest('reduceRight bigint/no init – 2 args', testReduceRightBigintNoInitTwoArgs);
  suite.addTest('reduceRight bigint/no init – 1 arg', testReduceRightBigintNoInitOneArg);
  suite.addTest('reduceRight bigint/no init – 0 args', testReduceRightBigintNoInitZeroArgs);

  // reduceRight – bigint with initialValue
  suite.addTest('reduceRight bigint/with init – full args', testReduceRightBigintWithInitFullArgs);

  // reduceRight – generic<U=...> with initialValue, varying arity
  suite.addTest('reduceRight generic<U=string>/with init – full args', testReduceRightGenericStringFullArgs);
  suite.addTest('reduceRight generic<U=object>/with init – 3 args', testReduceRightGenericObjectThreeArgs);
  suite.addTest('reduceRight generic<U=boolean>/with init – 2 args', testReduceRightGenericBooleanTwoArgs);
  suite.addTest('reduceRight generic<U=bigint>/with init – 1 arg', testReduceRightGenericBigintOneArg);
  suite.addTest('reduceRight generic<U=bigint>/with init – 0 args', testReduceRightGenericBigintZeroArgs);

  // reduceRight – order + exception
  suite.addTest('reduceRight – order verification', testReduceRightOrderVerification);
  suite.addTest('reduceRight – throws on empty without init', testReduceRightThrowsOnEmpty);

  return suite.run();
}

/* reduce – bigint, no initialValue, varying callback arity */

function testReduceBigintNoInitFullArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const sum = arr.reduce((prev: bigint, cur: bigint, idx: int, a: BigUint64Array): bigint => prev + cur);
  arktest.assertEQ(sum, 10n);
}

function testReduceBigintNoInitThreeArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const sum = arr.reduce((prev: bigint, cur: bigint, idx: int): bigint => prev + cur);
  arktest.assertEQ(sum, 6n);
}

function testReduceBigintNoInitTwoArgs(): void {
  const arr = new BigUint64Array([5n, 2n, 7n]);
  const sum = arr.reduce((prev: bigint, cur: bigint): bigint => prev + cur);
  arktest.assertEQ(sum, 14n);
}

function testReduceBigintNoInitOneArg(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const res = arr.reduce((prev: bigint): bigint => prev + 1n);
  // No initialValue => 3 calls for length=4, starting with first element as prev
  arktest.assertEQ(res, arr[0] + 3n);
}

function testReduceBigintNoInitZeroArgs(): void {
  const arr = new BigUint64Array([9n, 8n, 7n]);
  const res = arr.reduce((): bigint => 123n);
  arktest.assertEQ(res, 123n);
}

/* reduce – bigint with initialValue */

function testReduceBigintWithInitFullArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const sum = arr.reduce((prev: bigint, cur: bigint, idx: int, a: BigUint64Array): bigint => prev + cur, 10n);
  arktest.assertEQ(sum, 20n);
}

/* reduce – generic<U=...> with initialValue, varying arity */

function testReduceGenericStringFullArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const joined = arr.reduce<string>(
    (prev: string, cur: bigint, idx: int, a: BigUint64Array): string => prev + `-${cur}@${idx}`,
    'start'
  );
  arktest.assertEQ(joined, 'start-1@0-2@1-3@2');
}

function testReduceGenericObjectThreeArgs(): void {
  const arr = new BigUint64Array([2n, 4n, 6n]);
  const acc = arr.reduce<AccSumCount>((prev: AccSumCount, cur: bigint, idx: int): AccSumCount => {
    prev.sum += cur;
    prev.count += 1;
    return prev;
  }, new AccSumCount(0n, 0));
  arktest.assertEQ(acc.sum, 12n);
  arktest.assertEQ(acc.count, 3);
}

function testReduceGenericBooleanTwoArgs(): void {
  const arr = new BigUint64Array([1n, 3n, 5n, 7n]);
  const allOdd = arr.reduce<boolean>((prev: boolean, cur: bigint): boolean => prev && (cur % 2n !== 0n), true);
  arktest.assertTrue(allOdd);
}

function testReduceGenericBigintOneArg(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const calls = arr.reduce<bigint>((prev: bigint): bigint => prev + 1n, 0n);
  arktest.assertEQ(calls, 4n);
}

function testReduceGenericBigintZeroArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const res = arr.reduce<bigint>(() => 7n, 100n);
  arktest.assertEQ(res, 7n);
}

/* reduce – index and array param verification */

function testReduceIndexAndArrayParams(): void {
  const arr = new BigUint64Array([10n, 20n, 30n]);
  let seenSameArray = true;
  const visited: number[] = [];
  const sum = arr.reduce((prev: bigint, cur: bigint, idx: int, a: BigUint64Array): bigint => {
    if (a !== arr) {
      seenSameArray = false;
    }
    visited.push(idx);
    return prev + cur;
  });
  arktest.assertTrue(seenSameArray);
  arktest.assertEQ(visited.length, 2); // no init => indexes 1,2
  arktest.assertEQ(visited[0], 1);
  arktest.assertEQ(visited[1], 2);
  arktest.assertEQ(sum, 60n);
}

/* reduce – exceptions */

function testReduceThrowsOnEmpty(): void {
  const empty = new BigUint64Array(0);
  arktest.expectThrow(() => {
    empty.reduce((p: bigint, c: bigint) => p + c);
  });
}

/* reduceRight – bigint, no initialValue, varying callback arity */

function testReduceRightBigintNoInitFullArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const diff = arr.reduceRight((prev: bigint, cur: bigint, idx: int, a: BigUint64Array): bigint => prev - cur);
  // No initialValue: start prev=4; then 4-3=1; 1-2=-1; -1-1=-2
  arktest.assertEQ(diff, -2n);
}

function testReduceRightBigintNoInitThreeArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const sum = arr.reduceRight((prev: bigint, cur: bigint, idx: int): bigint => prev + cur);
  arktest.assertEQ(sum, 6n);
}

function testReduceRightBigintNoInitTwoArgs(): void {
  const arr = new BigUint64Array([5n, 2n, 7n]);
  const sum = arr.reduceRight((prev: bigint, cur: bigint): bigint => prev + cur);
  arktest.assertEQ(sum, 14n);
}

function testReduceRightBigintNoInitOneArg(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const res = arr.reduceRight((prev: bigint): bigint => prev + 1n);
  arktest.assertEQ(res, arr[3] + 3n);
}

function testReduceRightBigintNoInitZeroArgs(): void {
  const arr = new BigUint64Array([9n, 8n, 7n]);
  const res = arr.reduceRight((): bigint => 50n);
  arktest.assertEQ(res, 50n);
}

/* reduceRight – bigint with initialValue */

function testReduceRightBigintWithInitFullArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n]);
  const res = arr.reduceRight((prev: bigint, cur: bigint, idx: int, a: BigUint64Array): bigint => prev + cur, 10n);
  arktest.assertEQ(res, 20n);
}

/* reduceRight – generic<U=...> with initialValue, varying arity */

function testReduceRightGenericStringFullArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const joined = arr.reduceRight<string>(
    (prev: string, cur: bigint, idx: int, a: BigUint64Array): string => prev + `(${cur}<${idx})`,
    'R'
  );
  arktest.assertEQ(joined, 'R(3<2)(2<1)(1<0)');
}

function testReduceRightGenericObjectThreeArgs(): void {
  const arr = new BigUint64Array([2n, 4n, 6n]);
  const acc = arr.reduceRight<AccProdSteps>((prev: AccProdSteps, cur: bigint, idx: int): AccProdSteps => {
    prev.prod *= cur;
    prev.steps += 1;
    return prev;
  }, new AccProdSteps(1n, 0));
  arktest.assertEQ(acc.prod, 48n);
  arktest.assertEQ(acc.steps, 3);
}

function testReduceRightGenericBooleanTwoArgs(): void {
  const arr = new BigUint64Array([2n, 4n, 6n]);
  const anyEvenFromRight = arr.reduceRight<boolean>(
    (prev: boolean, cur: bigint): boolean => prev || (cur % 2n === 0n), false
  );
  arktest.assertTrue(anyEvenFromRight);
}

function testReduceRightGenericBigintOneArg(): void {
  const arr = new BigUint64Array([1n, 2n, 3n, 4n, 5n]);
  const calls = arr.reduceRight<bigint>((prev: bigint): bigint => prev + 1n, 0n);
  arktest.assertEQ(calls, 5n);
}

function testReduceRightGenericBigintZeroArgs(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const res = arr.reduceRight<bigint>(() => -1n, 999n);
  arktest.assertEQ(res, -1n);
}

/* reduceRight – order verification */

function testReduceRightOrderVerification(): void {
  const arr = new BigUint64Array([1n, 2n, 3n]);
  const left = arr.reduce((p: bigint, c: bigint) => p * 10n + c, 0n);
  const right = arr.reduceRight((p: bigint, c: bigint) => p * 10n + c, 0n);
  arktest.assertEQ(left, 123n);
  arktest.assertEQ(right, 321n);
}

/* reduceRight – exceptions */

function testReduceRightThrowsOnEmpty(): void {
  const empty = new BigUint64Array(0);
  arktest.expectThrow(() => {
    empty.reduceRight((p: bigint, c: bigint) => p + c);
  });
}
