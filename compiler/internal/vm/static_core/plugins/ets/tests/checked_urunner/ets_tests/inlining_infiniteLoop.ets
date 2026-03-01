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

//! CHECKER      Check inlining infiniteLoop
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: ""
//! METHOD       "inlining_infiniteLoop.ETSGLOBAL::testInfiniteLoop"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "ReturnInlined", 1
//! EVENT        /Inline,inlining_infiniteLoop.ETSGLOBAL::testInfiniteLoop,inlining_infiniteLoop.MyClass::<ctor>,9,STATIC,INF_LOOP/
//!
//! METHOD       "inlining_infiniteLoop.MyClass::<ctor>"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "end", 0

class MyIterable<T> implements IterableIterator<T> {
    public item : T;

    public next() : IteratorResult<T> {
        return new IteratorResult<T>(true, undefined)
    }

    public $_iterator() : IterableIterator<T> {
        return this;
    }
}

class MyClass {
    public constructor(element: Iterable<Int>) {
        let it = element.$_iterator();
        while(true) {
            const v = it.next();
            if (v.done) {}
        }
    }
}

function testInfiniteLoop() : void {
    let a = new MyIterable<Int>();
    let myClass = new MyClass(a);
}

//! CHECKER      Check inlining not infiniteLoop
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-regex='.*test.*'", entry: "inlining_infiniteLoop.ETSGLOBAL::testNotInfiniteLoop"
//! METHOD       "inlining_infiniteLoop.ETSGLOBAL::testNotInfiniteLoop"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "ReturnInlined", 4

class MyClass1 {
    public constructor(element: Iterable<Int>) {
        let it = element.$_iterator();
        let count = 5;
        while(count > 0) {
            const v = it.next();
            if (v.done) {}
            count++;
        }
    }
}

function testNotInfiniteLoop() : void {
    let a = new MyIterable<Int>();
    let myClass = new MyClass1(a);
}
