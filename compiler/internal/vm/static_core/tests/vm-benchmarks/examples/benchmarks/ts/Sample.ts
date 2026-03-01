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
 * @Import { StringUtil } from '../shared/ts/libStringUtil.ts'
 * @Import { dummy } from '../shared/ts/libDummy.ts'
 * @Include "../shared/ts/includeMe.ts"
 */

/**
 * @State
 * @Tags shared
 */
class Sample {

    /**
     * @Param 128
     */
    stringSize: number;
    /**
     * @Param 100
     */
    loopCount: number;
    expected: number = 128 + 100;
    startString: string = '';
    concatString: string = 'Ab';

    /**
     * @Setup
     */
    setup(): void {
        let util = new StringUtil();
        this.startString = util.randomString(this.stringSize);
        this.concatString = dummy();
        this.expected = this.stringSize + this.loopCount;
    }

    /**
     * @Benchmark
     * @Tags strings
     */
    public testConcat(): string {
        let result: string = this.startString;
        for (let i = 0; i < this.loopCount; i++) {
            result = result.concat(this.concatString);
        }
        return result.substring(this.loopCount / 2, this.loopCount);
    }

    /**
     * @Benchmark
     * @Tags math
     */
    public testSum(): number {
        let r = sum(this.loopCount, this.stringSize);
        if (r !== this.expected) {
            throw Error('Sum is wrong!');
        }
        return r;
    }

}
