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

native function callBacktrace(): int;

function TestBacktrace_entry_simple(): int {
    return TestBacktrace_func1();
}

function TestBacktrace_func1(): int {
    return TestBacktrace_func2();
}

function TestBacktrace_func2(): int {
    return callBacktrace();
}

function TestBacktrace_entry_complex(): int {
    return TestBacktrace_entry_test_1_frame();
}

function TestBacktrace_entry_test_1_frame(): int {
    return getOperationFactory()
}

interface FileDataValue {
  fileName: string;
  filePath: string;
  data: DataValue;
}

interface DataValue {
  dataLength: number;
  dataOffset: number;
  dataStr: string;
}

class OperationFactory {
  private currentType: string = 'add';

  createOperation(type: 'add' | 'subtract' | 'multiply' | 'divide', a: number, b: number): number {
    this.currentType = type;
    switch (type) {
      case 'add':
        return a + b;
      case 'subtract':
        return a - b;
      case 'multiply':
        return a * b;
      case 'divide':
        if (b === 0) {
          throw new Error("Division by zero is not allowed.");
        }
        return a / b;
      default:
        throw new Error("Unknown operation type");
    }
  }
}

function getFileDate(): int {
  let FileDataValues: FixedArray<FileDataValue> = [{
    fileName: 'OperationFactoryFile',
    filePath: 'source/url/OperationFactoryFile',
    data: {
      dataLength: 10,
      dataOffset: 2,
      dataStr: 'This is a test data!'
    }
  }];
  return callBacktrace();
}

function getOperationFactory(): int {
  const factory = new OperationFactory();
  factory.createOperation('add', 2, 3);
  factory.createOperation('subtract', 5, 5);
  factory.createOperation('multiply', 3, 6);
  factory.createOperation('divide', 2, 2);
  return getFileDate();
}

function main(): void {
  loadLibrary("call_backtrace");
  let res = TestBacktrace_entry_simple();
  arktest.assertEQ(res, 1);
  let res2 = TestBacktrace_entry_complex();
  arktest.assertEQ(res, 1);
}