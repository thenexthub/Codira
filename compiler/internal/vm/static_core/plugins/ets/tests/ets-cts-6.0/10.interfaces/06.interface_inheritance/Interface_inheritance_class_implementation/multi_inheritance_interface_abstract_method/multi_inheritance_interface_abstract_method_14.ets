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
desc: More interfaces and complex methods(number type).
---*/

interface Reader {
  read(file: number): number;
}

interface Writer {
  write(content: number, file: number): number;
}

interface Deleter {
  delete(file: number): number;
}

class FileManager implements Reader, Writer, Deleter {
  read(file: number): number {
    return 1;
  }

  write(content: number, file: number): number {
    return content + file;
  }

  delete(file: number): number {
    return 3;
  }
}

function main(): void {
  const fm = new FileManager();
  const result = fm.read(1)
  const result1 = fm.write(2, 2);
  const result2 = fm.delete(3);
  arktest.assertEQ(result, 1);
  arktest.assertEQ(result1, 4);
  arktest.assertEQ(result2, 3);
}