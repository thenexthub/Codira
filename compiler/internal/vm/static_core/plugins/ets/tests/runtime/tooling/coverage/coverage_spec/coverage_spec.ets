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
import { func1 } from './package_a/module1.ets';
import { func2 } from './package_a/module2.ets';


function func() {
  console.println('func enter');
  return;
}

function riskyOperation(): number {
    // Simulate operations that may fail
    throw new Error('Stochastic Operation Failure');
}

function main() {
  let choice: int = 2;  // Type must be declared
  let a: number = 10;
  let b: number = 5;
  let flag: number = 1;
  let value: number = 7;
  func1();
  func2();

  // 1. SwitchStatement - Note: switch only supports int-like, string, or enum types
  switch (choice) {
    case 1:
      console.info('Case 1');
      break;
    case 2:
      console.info('Case 2');
      func();
      break;
    case 3:
      console.info('Case 3');
      break;
    default:
      console.info('Default case');
      break;
  }

  // 2. ConditionalExpression (ternary operator)
  let max: number = (a > b) ? a : b;
  console.info(`Max is ${max}`);

  // 3. ForUpdateStatement: update clause in for loop (i++)
  for (let i: number = 0; i < 3 && i > 0; i++) {  // Must be initialized or have type declared
    console.info(`For loop iteration ${i}`);
  }

  // 4. WhileStatement
  let j: number = 0;
  while (j < 3) {
    console.info(`While loop iteration ${j}`);
    j++;
  }

  // 5. DoWhileStatement
  let x: number = 0;
  do {
    console.info(`Do-while loop iteration ${x}`);
    x++;
  } while (x < 3);

  // 6. ForInStatement: ETS does not support for...in for arrays (only for object properties)
  //  Use regular for loop as alternative
  let arr: number[] = [1, 2, 3, 4, 5];
  for (let i: int = 0; i < arr.length; i++) {
    console.info(`ForIn simulation: index=${i}, value=${arr[i]}`);
  }

  // 7. If condition checks (containing at least two conditions)

  // Example 1: &&
  if (a > 0 && b < 10 && a < 20) {
    console.info('Both conditions in if are true.');
  } else {
    console.info('At least one condition in if is false.');
  }

  // Example 2: ||
  if (flag === 1 || value > 100) {
    console.info('At least one condition in second if is true.');
  } else {
    console.info('Both conditions in second if are false.');
  }

  // Example 3: Nested if
  if (value > 5) {
    if (value < 8) {
      console.info('Value is 6 or 7.');
    } else {
      console.info('Value is 8 or more.');
    }
  } else {
    console.info('Value is 5 or less.');
  }

  try {
    console.info('Inner try block started');

    // Inner operation that may throw exception
    let result = riskyOperation();
    console.info(`Inner operation result: ${result}`);
  } catch (innerError) {
    if (innerError instanceof Error) {
      // Inner catch: handles inner exception
      console.error('Inner exception caught:', innerError.message);
    }
  } finally {
    console.info('Finally block: always executed.');
  }

  if (true) {
    let a = 4;
    let b = a/2;
  }
}