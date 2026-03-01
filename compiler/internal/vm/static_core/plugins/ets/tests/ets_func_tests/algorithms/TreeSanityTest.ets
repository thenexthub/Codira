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


// Some sanity tests on basic container algorithms

const tests: FixedArray<FixedArray<FixedArray<int>>> = [
    [[8, 3, 1, 2, 3, 1, 0], [0, 1, 2, 3, 8]],
    [[1], [1]],
    [[4+4, 3, +1, 2, +3, 1, -0, +0, -1], [-1, 0, 1, 2, 3, 8]]
];

function cmp(x: FixedArray<int>, y: FixedArray<int>): boolean {
    if (x.length != y.length) {
        return false;
    }
    for (let i: int = 0; i < x.length; i++) {
        if (x[i] != y[i]) {
            return false;
        }
    }
    return true;
}

class TreeNode {
    private left: TreeNode | null;
    private right: TreeNode | null;
    private item: int;

    constructor(item: int) {
        this.left = null;
        this.right = null;
        this.item = item;
    }

    constructor(left: TreeNode, right: TreeNode, item: int) {
        this.left = left;
        this.right = right;
        this.item = item;
    }

    public push(item: int): void {
        if (item < this.item) {
            if (this.left == null) {
                this.left = new TreeNode(item);
            } else {
                this.left!.push(item);
            }
        } else if (item == this.item) {
            return;
        } else {
            if (this.right == null) {
                this.right = new TreeNode(item);
            } else {
                this.right!.push(item);
            }
        }
    }

    public traverse(): FixedArray<int> {
        let l: FixedArray<int> = [];
        let r: FixedArray<int> = [];
        if (this.left != null) {
            l = this.left!.traverse();
        }
        if (this.right != null) {
            r = this.right!.traverse();
        }
        let items: FixedArray<int> = concat(l, [this.item]);
        return concat(items, r);
    }
}

function testTree(data: FixedArray<FixedArray<int>>): boolean {
    let unsorted: FixedArray<int> = copyOf(data[0]);
    let sorted: FixedArray<int> = copyOf(data[1]);
    let tree = new TreeNode(unsorted[0]);
    for (let i: int = 1; i < unsorted.length; i++) {
        tree.push(unsorted[i]);
    }
    return cmp(tree.traverse(), sorted);
}

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name);
        return 0;
    }
    console.println("FAILED: " + name);
    return 1;
}

function main(): int {
    let failures: int = 0;
    failures += test(testTree(tests[0]), "Test Binary Tree 0");
    failures += test(testTree(tests[1]), "Test Binary Tree 1");
    failures += test(testTree(tests[2]), "Test Binary Tree 2");
    if (failures == 0) {
        console.println("PASSED: All tests run");
    } else {
        console.println("FAILED: All tests run");
    }
    return failures;
}
