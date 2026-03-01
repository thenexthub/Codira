/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Issue 17337 Smartcasts in loops
tags: [compile-only]
---*/

class TreeRoot extends TreeNode {
}

class TreeNode {
    private nodeParent: TreeNode | TreeRoot = new TreeNode

    get depth(): int {
        let count: int = 0;
        let parent = this.nodeParent
        while(!(parent instanceof TreeRoot)) {
            parent = parent.nodeParent
            count++;
        }
        return count;
    }
}

