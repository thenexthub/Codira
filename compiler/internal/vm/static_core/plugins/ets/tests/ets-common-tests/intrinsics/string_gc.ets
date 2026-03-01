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

function doFullGC() : void
{
    try {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
    } catch (e) {
        arktest.assertTrue(false, "Unexpected during GC");
    }
}

function main() : int
{
    let treeStr : String = '';
    let smallStr : String = '0123456789';
    let treeCnt : int = 100;
    let slicedCnt : int = 100;

    for (let i : int = 0; i < treeCnt; i++) {
        treeStr = treeStr.concat(smallStr);
    }

    doFullGC();

    let subStr : String = '';
    for (let i : int = 0; i < slicedCnt; i++) {
        subStr = treeStr.substring(0, treeStr.getLength() - i);
        arktest.assertEQ(subStr.getLength(), treeStr.getLength() - i);
    }

    arktest.assertEQ(treeStr.getLength(), treeCnt * smallStr.getLength());
    arktest.assertEQ(subStr.getLength(), treeStr.getLength() - (slicedCnt - 1));

    let treeStrCopy : String = treeStr;
    let subStrCopy : String = subStr;

    doFullGC();

    arktest.assertEQ(treeStrCopy, treeStr);
    arktest.assertEQ(subStrCopy, subStr);

    arktest.assertEQ(treeStr.getLength(), treeCnt * smallStr.getLength());
    arktest.assertEQ(subStr.getLength(), treeStr.getLength() - (slicedCnt - 1));
    return 0;
}