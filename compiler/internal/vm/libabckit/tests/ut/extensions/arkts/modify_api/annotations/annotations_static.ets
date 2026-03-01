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

@interface interfacetest{
    a1F1: number = 44;
}

@interface interfacetesttwo{
    a2F1: number = 55;
    a2F2: number = 66;
}

class c1 {
    foo (){}
    c1F1: string = "testString1";
    @interfacetest()
    c1F2: string = "testString2";
}

class c2 {
    c2F1: string = "myClass param";
}

interface I1 {
    @interfacetest()
    i1F1: string;
}

interface I2 {
    i2F1: string;
}