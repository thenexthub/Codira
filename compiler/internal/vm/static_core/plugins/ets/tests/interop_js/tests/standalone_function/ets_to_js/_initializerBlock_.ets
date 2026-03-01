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

package interopStandaloneFnTest

static {
    defaultClass = new ClassWithMethodEts();
    functionRetTypeNumberEts = makeFunctionReturningValueType<number>(1);
    functionRetTypeByteEts = makeFunctionReturningValueType<byte>(1);
    functionRetTypeShortEts = makeFunctionReturningValueType<short>(1);
    functionRetTypeIntEts = makeFunctionReturningValueType<int>(1);
    functionRetTypeLongEts = makeFunctionReturningValueType<long>(1);
    functionRetTypeFloatEts = makeFunctionReturningValueType<float>(3.140000104904175);
    functionRetTypeCharEts = makeFunctionReturningValueType<char>(c'c');
    functionRetTypeBooleanEts = makeFunctionReturningValueType<boolean>(true);
    functionRetTypeStringEts = makeFunctionReturningValueType<string>('test');
}
