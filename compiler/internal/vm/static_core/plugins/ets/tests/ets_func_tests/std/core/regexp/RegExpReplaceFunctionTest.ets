/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

function test1(shor: String): int {
    let failure = 0
    let ctr = new Int(0);

    let lambda: (str: String, args: Object[]) => String = (str: String, args: Object[]): String => {
        failure += checkTestResult(str, "x")
        failure += checkTestResult(args[0] as Double, new Double(0.0))
        failure += checkTestResult(args[1].toString(), shor)
        let ans = ctr.toString();
        ctr++;
        return ans;
    }
    failure += checkTestResult(shor.replace("x", lambda), "0axbxcx")
    failure += checkTestResult(ctr, 1)
    return failure
}

function test2(shor: String): int {
    let failure = 0
    let ctr = new Int(0);

    let lambda: (str: String, args: Object[]) => String = (str: String, args: Object[]): String => {
        failure += checkTestResult(str, "x")
        failure += checkTestResult(args[0] as Double, new Double(0.0))
        failure += checkTestResult(args[1].toString(), shor)
        let ans = ctr.toString();
        ctr++;
        return ans;
    }
    failure += checkTestResult(shor.replace(new RegExp("x"), lambda), "0axbxcx")
    failure += checkTestResult(ctr, 1)
    return failure
}


function test3(shor: String): int {
    let failure = 0
    let ctr = new Int(0);

    let lambda: (str: String, args: Object[]) => String = (str: String, args: Object[]): String => {
        failure += checkTestResult(str, "x")
        failure += checkTestResult(args[0] as Double, new Double((ctr * 2).toDouble()))
        failure += checkTestResult(args[1].toString(), shor)
        let ans = ctr.toString();
        ctr++;
        return ans;
    }
    failure += checkTestResult(shor.replace(new RegExp("x", "g"), lambda), "0a1b2c3")
    failure += checkTestResult(ctr, 4)
    return failure
}

function test4(shor: String): int {
    let failure = 0
    let ctr = new Int(0);

    let test: String[] = ["a", "b", "c"]
    let lambda: (str: String, args: Object[]) => String = (str: String, args: Object[]): String => {
        failure += checkTestResult(str, "x")
        failure += checkTestResult(args[0].toString(), "x")
        failure += checkTestResult(args[1].toString(), test[ctr])
        failure += checkTestResult(args[2] as Double, new Double((ctr * 2).toDouble()))
        failure += checkTestResult(args[3].toString(), shor)
        let ans = ctr.toString();
        ctr++;
        return ans;
    }
    failure += checkTestResult(shor.replace(new RegExp("(x)(?=(.))", "g"), lambda), "0a1b2cx")
    failure += checkTestResult(ctr, 3)
    return failure
}

function test5(shor: String): int {
    let failure = 0

    let lambda: (str: String, args: Object[]) => String = (str: String, args: Object[]): String => {
        return str.toUpperCase()
    }
    failure += checkTestResult(shor.replace(new RegExp("(.)", "g"), lambda), "ABCD")
    return failure
}

function test6(): int {
    let longstring = "xyzzy"
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    longstring = longstring + longstring
    let str = "<" + longstring + ">"
    let actual = str.replace(new RegExp("<(.*)>", "g"), "$1$1")
    return checkTestResult(actual, longstring + longstring)
}

function test7(): int {
    let failure = 0
    let str1 = "foo"
    let str2 = "•foo"

    failure += checkTestResult(str1.replace(new RegExp("(f)|(g)"), "$2"), "oo")
    failure += checkTestResult(str1.replace(new RegExp("(f)|(g)"), "$1"), "foo")
    failure += checkTestResult(str1.replace(new RegExp("(q)|(z)"), "$2"), "foo")
    failure += checkTestResult(str2.replace(new RegExp("(f)|(g)"), "$2"), "•oo")
    failure += checkTestResult(str2.replace(new RegExp("(f)|(g)"), "$1"), "•foo")
    failure += checkTestResult(str2.replace(new RegExp("(q)|(z)"), "$2"), "•foo")

    return failure
}

function test8(): int {
    let failure = 0
    let pattern = '(?<fst>.)(?<snd>.)|(?<thd>x)'
    for (let flags of ['', 'u']) {
        let re = new RegExp(pattern, flags)
        failure += checkTestResult('cd', 'abcd'.replace(re, '$<42$1>'))
        failure += checkTestResult('cd', 'abcd'.replace(re, '$<fth>'))
        failure += checkTestResult('cd', 'abcd'.replace(re, '$<$1>'))
        failure += checkTestResult('cd', 'abcd'.replace(re, '$<>'))
        failure += checkTestResult('cd', 'abcd'.replace(re, '$<thd>'))
        failure += checkTestResult('bacd', 'abcd'.replace(re, '$<snd>$<fst>'))
    }
    for (let flags of ['g', 'gu']) {
        let re = new RegExp(pattern, flags)
        failure += checkTestResult('', 'abcd'.replace(re, '$<42$1>'))
        failure += checkTestResult('', 'abcd'.replace(re, '$<fth>'))
        failure += checkTestResult('', 'abcd'.replace(re, '$<$1>'))
        failure += checkTestResult('', 'abcd'.replace(re, '$<>'))
        failure += checkTestResult('', 'abcd'.replace(re, '$<thd>'))
        failure += checkTestResult('badc', 'abcd'.replace(re, '$<snd>$<fst>'))
    }
    return failure
}

function main(): int {
    let str = "xaxbxcx"
    let failure = 0
    failure += test1(str)
    failure += test2(str)
    failure += test3(str)
    failure += test4(str)
    failure += test5("abcd")
    failure += test6()
    failure += test7()
    failure += test8()
    console.println(failure)
    return failure
}

function checkTestResult(actual: String, expected: String): int {
    if (actual.equals(expected)) return 0
    return 1
}

function checkTestResult(actual: Int, expected: Int): int {
    if (actual.equals(expected)) return 0
    return 1
}

const DBL_EPSILON: double = 1.0e-6;
const doubleEpsilon: double = 2.22045e-16
function checkTestResult(actual: Double, expected: Double): int {
    if (expected.isNaN()) {
        if (actual.isNaN()) return 0;
        else return 1;
    }

    if (expected == Double.POSITIVE_INFINITY) {
        if (actual == Double.POSITIVE_INFINITY) return 0;
        else return 1;
    }

    if (expected == Double.NEGATIVE_INFINITY) {
        if (actual == Double.NEGATIVE_INFINITY) return 0;
        else return 1;
    }
    if (actual == expected) {
        return 0;
    }
    let diff: double;
    if (actual > expected) {
        diff = actual - expected;
    } else {
        diff = expected - actual;
    }
    if (diff < DBL_EPSILON) {
        return 0;
    } else {
        return 1;
    }
}