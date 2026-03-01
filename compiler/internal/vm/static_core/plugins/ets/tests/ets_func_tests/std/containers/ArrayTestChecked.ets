/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function main(): void {
  const result0 = Array.of<number>(5, 4, 3);
  arktest.assertEQ( result0.length, 3)
  arktest.assertEQ(result0.$_get(0), 5)
  arktest.assertEQ(result0.$_get(1), 4)
  arktest.assertEQ(result0.$_get(2), 3)

  const result1 = new Array<number>();
  arktest.assertEQ( result1.length, 0)

  testCopyWithin();
  testFill();
  testRevSort();
  testSliceSplice();
}

function testCopyWithin(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5, 6)
  const result4 = init.copyWithin(0, 2, -2);
  arktest.assertEQ( result4.length, 6)
  arktest.assertEQ(result4.$_get(0), 3)
  arktest.assertEQ(result4.$_get(1), 4)
  arktest.assertEQ(result4.$_get(2), 3)
  arktest.assertEQ(result4.$_get(3), 4)
  arktest.assertEQ(result4.$_get(4), 5)
  arktest.assertEQ(result4.$_get(5), 6)
  arktest.assertEQ( init.length, 6)
  arktest.assertEQ(init.$_get(0), 3)
  arktest.assertEQ(init.$_get(1), 4)
  arktest.assertEQ(init.$_get(2), 3)
  arktest.assertEQ(init.$_get(3), 4)
  arktest.assertEQ(init.$_get(4), 5)
  arktest.assertEQ(init.$_get(5), 6)
}

function testFill(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5)

  arktest.assertEQ( init.length, 5)
  arktest.assertEQ(init.$_get(0), 1)
  arktest.assertEQ(init.$_get(1), 2)
  arktest.assertEQ(init.$_get(2), 3)
  arktest.assertEQ(init.$_get(3), 4)
  arktest.assertEQ(init.$_get(4), 5)

  const result7 = init.fill(2, 2.0, 4.0);
  arktest.assertEQ( result7.length, 5)
  arktest.assertEQ(result7.$_get(0), 1)
  arktest.assertEQ(result7.$_get(1), 2)
  arktest.assertEQ(result7.$_get(2), 2)
  arktest.assertEQ(result7.$_get(3), 2)
  arktest.assertEQ(result7.$_get(4), 5)

  const result8 = init.fill(1, 3.0, Infinity);
  arktest.assertEQ( result8.length, 5)
  arktest.assertEQ(result8.$_get(0), 1)
  arktest.assertEQ(result8.$_get(1), 2)
  arktest.assertEQ(result8.$_get(2), 2)
  arktest.assertEQ(result8.$_get(3), 1)
  arktest.assertEQ(result8.$_get(4), 1)

  const result9 = init.fill(0, -Infinity, Infinity);
  arktest.assertEQ( result9.length, 5)
  arktest.assertEQ(result9.$_get(0), 0)
  arktest.assertEQ(result9.$_get(1), 0)
  arktest.assertEQ(result9.$_get(2), 0)
  arktest.assertEQ(result9.$_get(3), 0)
  arktest.assertEQ(result9.$_get(4), 0)

  const result10 = init.fill(30, -2.0, -1.0);
  arktest.assertEQ( result10.length, 5)
  arktest.assertEQ(result10.$_get(0), 0)
  arktest.assertEQ(result10.$_get(1), 0)
  arktest.assertEQ(result10.$_get(2), 0)
  arktest.assertEQ(result10.$_get(3), 30)
  arktest.assertEQ(result10.$_get(4), 0)

  arktest.assertEQ( init.length, 5)
  arktest.assertEQ(init.$_get(0), 0)
  arktest.assertEQ(init.$_get(1), 0)
  arktest.assertEQ(init.$_get(2), 0)
  arktest.assertEQ(init.$_get(3), 30)
  arktest.assertEQ(init.$_get(4), 0)
}

function testRevSort(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5, 11, 21, 111)

  arktest.assertEQ( init.length, 8)
  arktest.assertEQ(init.$_get(0), 1)
  arktest.assertEQ(init.$_get(1), 2)
  arktest.assertEQ(init.$_get(2), 3)
  arktest.assertEQ(init.$_get(3), 4)
  arktest.assertEQ(init.$_get(4), 5)
  arktest.assertEQ(init.$_get(5), 11)
  arktest.assertEQ(init.$_get(6), 21)
  arktest.assertEQ(init.$_get(7), 111)

  const result13 = init.reverse();
  arktest.assertEQ( result13.length, 8)
  arktest.assertEQ(result13.$_get(0), 111)
  arktest.assertEQ(result13.$_get(1), 21)
  arktest.assertEQ(result13.$_get(2), 11)
  arktest.assertEQ(result13.$_get(3), 5)
  arktest.assertEQ(result13.$_get(4), 4)
  arktest.assertEQ(result13.$_get(5), 3)
  arktest.assertEQ(result13.$_get(6), 2)
  arktest.assertEQ(result13.$_get(7), 1)

  arktest.assertEQ( init.length, 8)
  arktest.assertEQ(init.$_get(0), 111)
  arktest.assertEQ(init.$_get(1), 21)
  arktest.assertEQ(init.$_get(2), 11)
  arktest.assertEQ(init.$_get(3), 5)
  arktest.assertEQ(init.$_get(4), 4)
  arktest.assertEQ(init.$_get(5), 3)
  arktest.assertEQ(init.$_get(6), 2)
  arktest.assertEQ(init.$_get(7), 1)

  const result15 = init.toReversed();
  arktest.assertEQ( result15.length, 8)
  arktest.assertEQ(result15.$_get(0), 1)
  arktest.assertEQ(result15.$_get(1), 2)
  arktest.assertEQ(result15.$_get(2), 3)
  arktest.assertEQ(result15.$_get(3), 4)
  arktest.assertEQ(result15.$_get(4), 5)
  arktest.assertEQ(result15.$_get(5), 11)
  arktest.assertEQ(result15.$_get(6), 21)
  arktest.assertEQ(result15.$_get(7), 111)

  arktest.assertEQ( init.length, 8)
  arktest.assertEQ(init.$_get(0), 111)
  arktest.assertEQ(init.$_get(1), 21)
  arktest.assertEQ(init.$_get(2), 11)
  arktest.assertEQ(init.$_get(3), 5)
  arktest.assertEQ(init.$_get(4), 4)
  arktest.assertEQ(init.$_get(5), 3)
  arktest.assertEQ(init.$_get(6), 2)
  arktest.assertEQ(init.$_get(7), 1)

  const result17 = init.toSorted();
  arktest.assertEQ( result17.length, 8)
  arktest.assertEQ(result17.$_get(0), 1)
  arktest.assertEQ(result17.$_get(1), 11)
  arktest.assertEQ(result17.$_get(2), 111)
  arktest.assertEQ(result17.$_get(3), 2)
  arktest.assertEQ(result17.$_get(4), 21)
  arktest.assertEQ(result17.$_get(5), 3)
  arktest.assertEQ(result17.$_get(6), 4)
  arktest.assertEQ(result17.$_get(7), 5)

  arktest.assertEQ( init.length, 8)
  arktest.assertEQ(init.$_get(0), 111)
  arktest.assertEQ(init.$_get(1), 21)
  arktest.assertEQ(init.$_get(2), 11)
  arktest.assertEQ(init.$_get(3), 5)
  arktest.assertEQ(init.$_get(4), 4)
  arktest.assertEQ(init.$_get(5), 3)
  arktest.assertEQ(init.$_get(6), 2)
  arktest.assertEQ(init.$_get(7), 1)

  const result19 = init.sort();
  arktest.assertEQ( result19.length, 8)
  arktest.assertEQ(result19.$_get(0), 1)
  arktest.assertEQ(result19.$_get(1), 11)
  arktest.assertEQ(result19.$_get(2), 111)
  arktest.assertEQ(result19.$_get(3), 2)
  arktest.assertEQ(result19.$_get(4), 21)
  arktest.assertEQ(result19.$_get(5), 3)
  arktest.assertEQ(result19.$_get(6), 4)
  arktest.assertEQ(result19.$_get(7), 5)

  arktest.assertEQ( init.length, 8)
  arktest.assertEQ(init.$_get(0), 1)
  arktest.assertEQ(init.$_get(1), 11)
  arktest.assertEQ(init.$_get(2), 111)
  arktest.assertEQ(init.$_get(3), 2)
  arktest.assertEQ(init.$_get(4), 21)
  arktest.assertEQ(init.$_get(5), 3)
  arktest.assertEQ(init.$_get(6), 4)
  arktest.assertEQ(init.$_get(7), 5)
}

function testSliceSplice(): void {
  const init = Array.of<number>(1, 2, 3, 4, 5, 11, 21, 111)

  const result21 = init.slice(1, 3);
  arktest.assertEQ( result21.length, 2)
  arktest.assertEQ(result21.$_get(0), 2)
  arktest.assertEQ(result21.$_get(1), 3)

  arktest.assertEQ( init.length, 8)
  arktest.assertEQ(init.$_get(0), 1)
  arktest.assertEQ(init.$_get(1), 2)
  arktest.assertEQ(init.$_get(2), 3)
  arktest.assertEQ(init.$_get(3), 4)
  arktest.assertEQ(init.$_get(4), 5)
  arktest.assertEQ(init.$_get(5), 11)
  arktest.assertEQ(init.$_get(6), 21)
  arktest.assertEQ(init.$_get(7), 111)

  const result23 = init.splice(2, 100);
  arktest.assertEQ( result23.length, 6)
  arktest.assertEQ(result23.$_get(0), 3)
  arktest.assertEQ(result23.$_get(1), 4)
  arktest.assertEQ(result23.$_get(2), 5)
  arktest.assertEQ(result23.$_get(3), 11)
  arktest.assertEQ(result23.$_get(4), 21)
  arktest.assertEQ(result23.$_get(5), 111)

  arktest.assertEQ( init.length, 2)
  arktest.assertEQ(init.$_get(0), 1)
  arktest.assertEQ(init.$_get(1), 2)
}
