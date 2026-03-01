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

//简单元组类型
export type BasicTuple = [number, string, boolean];

//元组类型的可选类型
export type OptionalElementTuple = [number, string?, boolean?];

//异构元组类型
export type HeterogeneousTuple = [string, number, boolean, Date];

//元组的剩余元素
export type RestTuple = [string, ...number[]];

//包含固定长度的元组
export type FixedLengthTuple = [number, string, boolean];

//元组与数组的结合
export type TupleArray = [number, string][];

//多维元组
export type TwoDimensionalTuple = [[number, string], [boolean, Date]];

//元组与联合类型结合
export type UnionTuple = [string | number, boolean, Date];