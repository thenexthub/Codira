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

 function main() {
  console.log("%i", 10);
  console.log("%i", Int.toByte(128));
  console.log("%i", 3.14);
  console.log("%i", "1234.567");
  console.log("%i", undefined);
  console.log("%i", true);
  console.log("%i", [[123], [456]]);
  console.log("%i", [] as Any[]);
  console.log("%i", 15n);
  console.log("%i", "");
  console.log("%i", "meow");

  console.log("---")
  console.log("%d", 10);
  console.log("%d", Int.toByte(128));
  console.log("%d", 3.14);
  console.log("%d", "1234.567");
  console.log("%d", undefined);
  console.log("%d", true);
  console.log("%d", [[123], [456]]);
  console.log("%d", [] as Any[]);
  console.log("%d", "");
  console.log("%d", "meow");

  console.log("---")
  console.log("%f", 10);
  console.log("%f", Int.toByte(128));
  console.log("%f", 3.14);
  console.log("%f", "1234.567");
  console.log("%f", undefined);
  console.log("%f", true);
  console.log("%f", [[123], [456]]);
  console.log("%f", [] as Any[]);
  console.log("%f", "");
  console.log("%f", "meow");

  console.log("---")
  console.log("%o", [[1, 2]]);
  console.log("%O", [[1, 2]]);

  console.log("---")
  console.log("%s", "hello, world");
  console.log("%s", [1, 2, 3]);
  console.log("%s", Int.toByte(128));
  console.log("%s", undefined);

  console.log("---")
  console.log("%j", "hello, world");
  console.log("%j", 123);
  console.log("%j", [1, 2, 3]);
  console.log("%j", true);
  console.log("%j", undefined);
  console.log("%j", null);
}
