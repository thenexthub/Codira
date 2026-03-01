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
desc: More interfaces and complex methods.
---*/

interface Reader {
  read(file: string): string;
}

interface Writer {
  write(content: string, file: string): string;
}

interface Deleter {
  delete(file: string): string;
}

class FileManager implements Reader, Writer, Deleter {
  read(file: string) {
    return `Reading content from ${file}`;
  }

  write(content: string, file: string) {
    return `Writing ${content} to ${file}`;
  }

  delete(file: string) {
    return `Deleting ${file}`;
  }
}

function main(): void {
  const fm = new FileManager();
  const result = fm.read('test.txt')
  const result1 = fm.write('Hello, world!', 'test.txt');
  const result2 = fm.delete('Our memories');
  arktest.assertEQ(result, "Reading content from test.txt");
  arktest.assertEQ(result1, "Writing Hello, world! to test.txt");
  arktest.assertEQ(result2, "Deleting Our memories");
}