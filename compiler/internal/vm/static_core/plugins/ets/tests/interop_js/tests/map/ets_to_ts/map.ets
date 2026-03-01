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

package map.test;

export let myMap: Map<string, number>;
static {
  myMap = new Map<string, number>();
  myMap.set('apple', 1);
}

export function GetMapElement(key: string): number|undefined {
  return myMap.get(key);
}

export function HasMapElement(key: string): boolean {
  return myMap.has(key);
}

export function GetMapSize(): number {
  return myMap.size;
}

export let WeekDays: Map<string, number>;
static {
  WeekDays = new Map<string, number>();
  WeekDays.set('Monday', 1);
  WeekDays.set('Tuesday', 2);
  WeekDays.set('Wednesday', 3);
  WeekDays.set('Thursday', 4);
  WeekDays.set('Friday', 5);
  WeekDays.set('Saturday', 6);
  WeekDays.set('Sunday', 7);
}