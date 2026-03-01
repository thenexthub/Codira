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

export enum Letter {
    A,
    B,
    C
};

export enum Color {
    Red,
    Green = -100,
    Blue,
    White = 0x00
};

export enum Color1 {
  Red,
  Green,
  Blue
}

export enum UserRole {
  Admin = 'admin',
  Editor = 'editor',
  Viewer = 'viewer',
  Guest = 'guest'
}

export type MyTuple = [number, string, boolean]
let tuple: MyTuple = [6, "abc", true]

// NOTE(ivagin): enable when supported by declgen #12808
// export enum Name {
//        Ivan = "Ivan",
//        Li = "Li"
// };
