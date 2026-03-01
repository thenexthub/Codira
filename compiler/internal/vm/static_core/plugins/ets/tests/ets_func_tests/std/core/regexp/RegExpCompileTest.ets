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

function compareLength(rea: RegExpExecArray | null, len: int): int {
    if (rea == null && len != 0) {
        return 1
    }
    if (rea != null && rea.length != len) {
        return 1
    }
    return 0
}

function test(): int {
    let fails: int = 0
    let re = new RegExp('[a-z]bbc')
    fails += compareLength(re.exec('xxxabbcxxx'), 1)
    re.compile('[A-Z]bbc')
    fails += compareLength(re.exec('xxxabbcxxx'), 0)
    re.compile('Hello.')
    fails += compareLength(re.exec('Hello\n'), 0)
    re.compile('Hello.', 's')
    fails += compareLength(re.exec('Hello\n'), 1)
    re.compile('Hello.', 'y')
    fails += compareLength(re.exec('Hello\n'), 0)
    return fails
}

function main(): int {
    return test()
}
