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
// 字符串枚举
declare enum Colors {
  Red = 'RED',
  Green = 'GREEN',
  Blue = 'BLUE',
  Yellow = "'YELLOW'"
}

// 数字枚举
declare enum Status {
  Pending,    // 0
  Approved,   // 1
  Rejected,   // 2
}

// 常量枚举
// constants.d.ts
declare const enum Status1 {
  Pending = 3,
  Approved = 4,
  Rejected = 5
}

// 异构枚举
// response.d.ts
declare enum Response1 {
  No = 0,
  Yes = 'YES',
}

// @systemapi
declare enum SystemErrorCode{
  success = "0",
  invalidInput = "1",
  networkError = "2",
  internalError = "3"
}

// @deprecated
declare enum LegacyStatus{
  active = 0,
  inactive = 1,
  pending = 2
}