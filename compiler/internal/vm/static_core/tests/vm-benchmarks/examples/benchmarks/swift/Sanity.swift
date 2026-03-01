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
 */
class Sanity {

    /**
     * @Param 10, 100
     */
    var size: Int?

    var resource: [Int] = [0, 0, 0, 0, 0]

    /**
     * @Setup
     */
    func FillArray() {
        resource = [46, 44, 21, 37, 84]
    }

    /**
     * @Benchmark -mi 3
     */
    func test() -> Int {
        var sum = 0;
        var end = size ?? 100
        for i in 0..<end {
            sum += resource[i % 5];
        }
        return Int(sum)
    }
}
