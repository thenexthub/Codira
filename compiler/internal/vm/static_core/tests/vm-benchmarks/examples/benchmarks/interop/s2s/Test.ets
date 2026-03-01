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
 * @State
 * @Import { StringUtil } from '../../shared/ets/libStringUtil.ets'
 * @Tags interop, s2s
 */
class Test {

  /**
   * @Param 128
   */
    stringSize: int;
  /**
   * @Param 100
   */
    loopCount: int;
    startString: string = '';
    concatString: string = 'Ab';

  /**
   * @Setup
   */
    setup(): void {
        let util = new StringUtil();
        this.startString = util.randomString(this.stringSize);
    }

  /**
   * @Benchmark
   * @Tags strings, substring
   */
    public test(): string {
        let result: string = this.startString;
        for (let i = 0; i < this.loopCount; i++) {
            result = result.concat(this.concatString);
        }
        return result.substring(this.loopCount / 2, this.loopCount);
    }

}
