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

import { testFunction } from 'test_import';
import { Chrono } from 'std/time';

const MS2NS = 1000000;

export class S2d {

    state: number = 0
    totalTime: number = 0;
    runsLeft: number = 0;

    public setup(): void {
        console.log('benchmark is set up')
    }

    bench(): int {
        const n = testFunction() as number;
        return 1;
    }

    test(): number {
        while (this.runsLeft > 0 ) {
            const start = Chrono.nanoNow();
            this.bench();
            const end = Chrono.nanoNow();
            this.runsLeft = this.runsLeft - 1;
            this.totalTime = this.totalTime + (end - start);
        }
        return this.totalTime;
    }

}

