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
import {p29} from "./superInterface"

export declare interface SuperInterface {
    p: number;
}

export declare class SubInterface implements SuperInterface {
    p: number;
    p1: number;
}

export declare abstract class SuperClass {
    p: number;
}

export declare class SubClass extends SuperClass {
    p: number;
}

export declare class SuperClass1 {
    p: number;
}

export declare class SubClass1 extends SuperClass1 {
    p: number;
}

interface A {
    p: number;
}

interface B extends A {
    p1: number
}

interface C {
    f(): void
}

interface D extends C {
}

interface E extends A {
}

interface F extends C {
    g(): void
}

export interface BuglyLogAdapter extends p29 {
}
