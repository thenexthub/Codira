/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! CHECKER       Compare double JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::Compare_.*'", entry: "ets_double_compare.ETSGLOBAL::main"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_more__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_less__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_more_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_less_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_ne__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_more__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_less__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_more_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_less_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_ne__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"

//! CHECKER       Compare double AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: ""
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_more__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_less__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_more_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_less_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_ne__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_more__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_less__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_more_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_less_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_eq__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! METHOD        "ets_double_compare.ETSGLOBAL::Compare_const_ne__noinline__"
//! PASS_AFTER    "Codegen"
//! INST_NOT      "Fcmp"
//! INST          "Compare"
//! RUN           entry: "ets_double_compare.ETSGLOBAL::main"

function Compare_more__noinline__(str1 : String, str2 : String): boolean {
    return str1.length < str2.length
}

function Compare_less__noinline__(str1 : String, str2 : String): boolean {
    return str1.length > str2.length
}

function Compare_more_eq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length <= str2.length
}

function Compare_less_eq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length >= str2.length
}

function Compare_eq__noinline__(str1 : String, str2 : String): boolean {
    return str1.length == str2.length
}

function Compare_ne__noinline__(str1 : String, str2 : String): boolean {
    return str1.length != str2.length
}

function Compare_const_more__noinline__(str : String) : boolean {
    if (str.length < 4.0) {
        return str.length < 2147483647.0;
    }
    return false;
}

function Compare_const_less__noinline__(str : String) : boolean {
    if (str.length > 4.0) {
        return str.length > -2147483648.0;
    }
    return false;
}

function Compare_const_more_eq__noinline__(str : String) : boolean {
    if (str.length <= 4.0) {
        return str.length <= 2147483648.0;
    }
    return false;
}

function Compare_const_less_eq__noinline__(str : String) : boolean {
    if (str.length >= 4.0) {
        return str.length >= -2147483649.0;
    }
    return false;
}

function Compare_const_eq__noinline__(str : String) : boolean {
    return str.length == 4.0;
}

function Compare_const_ne__noinline__(str : String) : boolean {
    return str.length != 4.0;
}

function main(): int {
    if (Compare_more__noinline__("abcde", "abcd")) {
        return 1
    }
    if (!Compare_more__noinline__("abcd", "abcde")) {
        return 2
    }
    if (Compare_more__noinline__("abcd", "abcd")) {
        return 3
    }

    if (!Compare_less__noinline__("abcde", "abcd")) {
        return 4
    }
    if (Compare_less__noinline__("abcd", "abcde")) {
        return 5
    }
    if (Compare_less__noinline__("abcd", "abcd")) {
        return 6
    }

    if (Compare_more_eq__noinline__("abcde", "abcd")) {
        return 7
    }
    if (!Compare_more_eq__noinline__("abcd", "abcde")) {
        return 8
    }
    if (!Compare_more_eq__noinline__("abcd", "abcd")) {
        return 9
    }

    if (!Compare_less_eq__noinline__("abcde", "abcd")) {
        return 10
    }
    if (Compare_less_eq__noinline__("abcd", "abcde")) {
        return 11
    }
    if (!Compare_less_eq__noinline__("abcd", "abcd")) {
        return 12
    }

    if (Compare_eq__noinline__("abcde", "abcd")) {
        return 13
    }
    if (Compare_eq__noinline__("abcd", "abcde")) {
        return 14
    }
    if (!Compare_eq__noinline__("abcd", "abcd")) {
        return 15
    }

    if (!Compare_ne__noinline__("abcde", "abcd")) {
        return 16
    }
    if (!Compare_ne__noinline__("abcd", "abcde")) {
        return 17
    }
    if (Compare_ne__noinline__("abcd", "abcd")) {
        return 18
    }

    if (!Compare_const_more__noinline__("abc")) {
        return 19
    }
    if (Compare_const_more__noinline__("abcd")) {
        return 20
    }
    if (Compare_const_more__noinline__("abcde")) {
        return 21
    }

    if (Compare_const_less__noinline__("abc")) {
        return 22
    }
    if (Compare_const_less__noinline__("abcd")) {
        return 23
    }
    if (!Compare_const_less__noinline__("abcde")) {
        return 24
    }

    if (!Compare_const_more_eq__noinline__("abc")) {
        return 25
    }
    if (!Compare_const_more_eq__noinline__("abcd")) {
        return 26
    }
    if (Compare_const_more_eq__noinline__("abcde")) {
        return 27
    }

    if (Compare_const_less_eq__noinline__("abc")) {
        return 28
    }
    if (!Compare_const_less_eq__noinline__("abcd")) {
        return 29
    }
    if (!Compare_const_less_eq__noinline__("abcde")) {
        return 30
    }

    if (Compare_const_eq__noinline__("abc")) {
        return 31
    }
    if (!Compare_const_eq__noinline__("abcd")) {
        return 32
    }
    if (Compare_const_eq__noinline__("abcde")) {
        return 33
    }

    if (!Compare_const_ne__noinline__("abc")) {
        return 34
    }
    if (Compare_const_ne__noinline__("abcd")) {
        return 35
    }
    if (!Compare_const_ne__noinline__("abcde")) {
        return 36
    }
    return 0
}
