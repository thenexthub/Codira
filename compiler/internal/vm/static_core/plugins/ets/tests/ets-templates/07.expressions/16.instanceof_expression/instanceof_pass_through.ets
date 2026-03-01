/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: Any instanceof expression in the form expr instanceof T
      The result of an instanceof expression is true if the actual type of evaluated expr is a subtype of T
---*/

interface IName {
    name: string
}

class Holder<X> {
}

enum Color { Red, Green, Blue }

type AStringAlias = string

type TT = {{c.type}}
type FC = (input: TT) => Any

class Pass {
    static pass(input: TT): TT {
        return input
    }

    dpass(input: TT): TT {
        return input
    }
}

function returnAsAny(input: TT): Any {
    return input
}

let test: TT = {{c.init}}

function main() {
    let runner: Pass = new Pass

    let fCall: FC = returnAsAny

    let d1 = Pass.pass(test)
    let d2 = fCall(test)
    let d3 = ((input: Any): Any => { return input })(test)

    let d4 = ((input: Any): Any => { return input })(fCall(Pass.pass(runner.dpass(test))))

    arktest.assertTrue(d1 instanceof TT, "Typecheck failed for D1")
    arktest.assertTrue(d2 instanceof TT, "Typecheck failed for D2")
    arktest.assertTrue(d3 instanceof TT, "Typecheck failed for D3")
    arktest.assertTrue(d4 instanceof TT, "Typecheck failed for D4")

}

{% endfor %}
