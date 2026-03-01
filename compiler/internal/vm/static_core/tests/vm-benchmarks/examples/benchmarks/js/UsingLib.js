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

/**
 * @Import { randomNumber } from './libOne.mjs'
 */

/**
 * @State
 */
function usingLib() {

  /**
   * @Param 128
   */
    this.size;
  /**
   * @Param "random"
   */
    this.variant;
    let nums;
    let numsInitial;

  /**
   * @Setup
   */
    this.setup = function() {
        numsInitial = new Array(this.size);
        switch (this.variant) {
            case 'random':
            for (let i = 0; i < this.size; i++) {
                numsInitial[i] = randomNumber(this.size);
            }
            break;
            default:
            throw new Error(
                'Invalid test variant: ' + this.variant);
        }
    };

  /**
   * @Benchmark
   * @returns {Obj}
   */
    this.testSort = function() {
        nums = Array.from(numsInitial);
        nums.sort();
        return nums;
    };

}
