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

// Basic type
export function processInput(input: string): string;
export function processInput(input: number): number;
export function processInput(input: boolean): boolean;
export function processInput(input: string | number | boolean): string | number | boolean {
  if (typeof input === 'string') {
    return input.toUpperCase();
  } else if (typeof input === 'number') {
    return input * 2;
  } else {
    return !input;
  }
}

// Generic
export function createArray<T>(value: T, length: number): T[];
export function createArray<T, U>(value1: T, value2: U, length: number): (T | U)[];
export function createArray<T, U>(
  value: T,
  value2OrLength: number | U,
  length?: number
): T[] | (T | U)[] {
  if (typeof length === 'number') {
    const value2 = value2OrLength as U;
    return Array.from({ length }, (_, i) => i % 2 === 0 ? value : value2);
  } else {
    const len = value2OrLength as number;
    return Array(len).fill(value);
  }
}

// Object
export interface User {
  id: number;
  name: string;
}

export interface Admin {
  id: number;
  role: string;
}

export function getUser(id: number): User;
export function getUser(role: string): Admin;
export function getUser(idOrRole: number | string): User | Admin;
export function getUser(idOrRole: number | string): User | Admin {
  if (typeof idOrRole === 'number') {
    return { id: idOrRole, name: 'John Doe' };
  } else {
    return { id: 1, role: idOrRole };
  }
}
