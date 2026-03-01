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

//! CHECKER       AOT IR Builder, check StringBuilder append calls merging
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*append[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append18"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::chain_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging  (--gc-type=stw)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part4.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::append18"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString2"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString3"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString4"
//! INST_NEXT     "Intrinsic.StdCoreSbAppendString "
//!
//! METHOD        "ets_string_builder_append_merge_part4.ETSGLOBAL::chain_append4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1

//! CHECKER       JIT IR Builder, check StringBuilder append calls merging (--gc-type=g1-gc)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--gc-type=g1-gc --no-async-jit=false --compiler-regex='.*append[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_append_merge_part4.ETSGLOBAL::main"

function append2(str0: string, str1: string): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();  // to prevent replacing append calls with concat intrinsic

    sb.append(str0);
    sb.append(str1);

    return sb.toString();           // applied, 2
}

function append3(str0: string, str1: string, str2: string): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append(str0);
    sb.append(str1);
    sb.append(str2);

    return sb.toString();           // applied, 3
}

function append4(str0: string, str1: string, str2: string, str3: string): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append(str0);
    sb.append(str1);
    sb.append(str2);
    sb.append(str3);

    return sb.toString();           // applied, 4
}

function append5(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");

    return sb.toString();           // applied, 4+1
}

function append6(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");

    return sb.toString();           // applied, 4+2
}

function append7(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");

    return sb.toString();           // applied, 4+3
}

function append8(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");

    return sb.toString();           // applied, 4+4
}

function append18(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0");
    sb.append(1 as int);
    sb.append("2");
    sb.append("3");
    sb.append(4 as int);
    sb.append("5");
    sb.append("6");
    sb.append("7");
    sb.append(8 as int);
    sb.append("9");
    sb.append("10");
    sb.append("11");
    sb.append(12 as int);
    sb.append("13");
    sb.append("14");
    sb.append("15");
    sb.append("16");
    sb.append("17");

    return sb.toString();           // applied, 1+int+2+int+3+int+3+int+4+1
}

function chain_append4(): string {
    let sb = new StringBuilder();
    if (sb != null) sb.toString();

    sb.append("0").append("1").append("2").append("3");

    return sb.toString();           // applied, 4
}

function main() : int {
    arktest.assertEQ(append5(), "01234", 'Wrong result at append5()');
    arktest.assertEQ(append6(), "012345", 'Wrong result at append6()');
    arktest.assertEQ(append7(), "0123456", 'Wrong result at append7()');
    arktest.assertEQ(append8(), "01234567", 'Wrong result at append8()');

    arktest.assertEQ(append18(), "01234567891011121314151617",  'Wrong result at append18');
    arktest.assertEQ(chain_append4(), "0123",  'Wrong result at chain_append4');

    const asciiString = "_xyz"

    arktest.assertEQ(append2(asciiString, asciiString), "_xyz_xyz",  'Wrong result at append2(asciiString, asciiString)');
    arktest.assertEQ(append2("str0"     , asciiString), "str0_xyz",  'Wrong result at append2("str0"     , asciiString)');
    arktest.assertEQ(append2(asciiString, "str1"     ), "_xyzstr1",  'Wrong result at append2(asciiString, "str1"     )');
    arktest.assertEQ(append2("str0"     , "str1"     ), "str0str1",  'Wrong result at append2("str0"     , "str1"     )');

    arktest.assertEQ(append3(asciiString, asciiString, asciiString), "_xyz_xyz_xyz", 'Wrong result at append3(asciiString, asciiString, asciiString)');
    arktest.assertEQ(append3("str0"     , asciiString, asciiString), "str0_xyz_xyz", 'Wrong result at append3("str0"     , asciiString, asciiString)');
    arktest.assertEQ(append3(asciiString, "str1"     , asciiString), "_xyzstr1_xyz", 'Wrong result at append3(asciiString, "str1"     , asciiString)');
    arktest.assertEQ(append3(asciiString, asciiString, "str2"     ), "_xyz_xyzstr2", 'Wrong result at append3(asciiString, asciiString, "str2"     )');
    arktest.assertEQ(append3("str0"     , "str1"     , asciiString), "str0str1_xyz", 'Wrong result at append3("str0"     , "str1"     , asciiString)');
    arktest.assertEQ(append3("str0"     , asciiString, "str2"     ), "str0_xyzstr2", 'Wrong result at append3("str0"     , asciiString, "str2"     )');
    arktest.assertEQ(append3(asciiString, "str1"     , "str2"     ), "_xyzstr1str2", 'Wrong result at append3(asciiString, "str1"     , "str2"     )');
    arktest.assertEQ(append3("str0"     , "str1"     , "str2"     ), "str0str1str2", 'Wrong result at append3("str0"     , "str1"     , "str2"     )');

    arktest.assertEQ(append4(asciiString, asciiString, asciiString, asciiString), "_xyz_xyz_xyz_xyz", 'Wrong result at append4(asciiString, asciiString, asciiString, asciiString)');
    arktest.assertEQ(append4("str0"     , asciiString, asciiString, asciiString), "str0_xyz_xyz_xyz", 'Wrong result at append4("str0"     , asciiString, asciiString, asciiString)');
    arktest.assertEQ(append4(asciiString, "str1"     , asciiString, asciiString), "_xyzstr1_xyz_xyz", 'Wrong result at append4(asciiString, "str1"     , asciiString, asciiString)');
    arktest.assertEQ(append4(asciiString, asciiString, "str2"     , asciiString), "_xyz_xyzstr2_xyz", 'Wrong result at append4(asciiString, asciiString, "str2"     , asciiString)');
    arktest.assertEQ(append4(asciiString, asciiString, asciiString, "str3"     ), "_xyz_xyz_xyzstr3", 'Wrong result at append4(asciiString, asciiString, asciiString, "str3"     )');
    arktest.assertEQ(append4("str0"     , "str1"     , asciiString, asciiString), "str0str1_xyz_xyz", 'Wrong result at append4("str0"     , "str1"     , asciiString, asciiString)');
    arktest.assertEQ(append4("str0"     , asciiString, "str2"     , asciiString), "str0_xyzstr2_xyz", 'Wrong result at append4("str0"     , asciiString, "str2"     , asciiString)');
    arktest.assertEQ(append4(asciiString, "str1"     , "str2"     , asciiString), "_xyzstr1str2_xyz", 'Wrong result at append4(asciiString, "str1"     , "str2"     , asciiString)');
    arktest.assertEQ(append4(asciiString, "str1"     , asciiString, "str3"     ), "_xyzstr1_xyzstr3", 'Wrong result at append4(asciiString, "str1"     , asciiString, "str3"     )');
    arktest.assertEQ(append4(asciiString, asciiString, "str2"     , "str3"     ), "_xyz_xyzstr2str3", 'Wrong result at append4(asciiString, asciiString, "str2"     , "str3"     )');
    arktest.assertEQ(append4("str0"     , "str1"     , "str2"     , asciiString), "str0str1str2_xyz", 'Wrong result at append4("str0"     , "str1"     , "str2"     , asciiString)');
    arktest.assertEQ(append4("str0"     , "str1"     , asciiString, "str3"     ), "str0str1_xyzstr3", 'Wrong result at append4("str0"     , "str1"     , asciiString, "str3"     )');
    arktest.assertEQ(append4("str0"     , asciiString, "str2"     , "str3"     ), "str0_xyzstr2str3", 'Wrong result at append4("str0"     , asciiString, "str2"     , "str3"     )');
    arktest.assertEQ(append4(asciiString, "str1"     , "str2"     , "str3"     ), "_xyzstr1str2str3", 'Wrong result at append4(asciiString, "str1"     , "str2"     , "str3"     )');
    arktest.assertEQ(append4("str0"     , "str1"     , "str2"     , "str3"     ), "str0str1str2str3", 'Wrong result at append4("str0"     , "str1"     , "str2"     , "str3"     )');

    const utf16String = "红旗";

    arktest.assertEQ(append2(utf16String, utf16String), "红旗红旗",  'Wrong result at append2(utf16String, utf16String)');
    arktest.assertEQ(append2("str0"     , utf16String), "str0红旗",  'Wrong result at append2("str0"    , utf16String)');
    arktest.assertEQ(append2(utf16String, "str1"     ), "红旗str1",  'Wrong result at append2(utf16String, "str1"    )');
    arktest.assertEQ(append2("str0"     , "str1"     ), "str0str1",  'Wrong result at append2("str0"    , "str1"    )');

    arktest.assertEQ(append3(utf16String, utf16String, utf16String), "红旗红旗红旗", 'Wrong result at append3(utf16String, utf16String, utf16String)');
    arktest.assertEQ(append3("str0"     , utf16String, utf16String), "str0红旗红旗", 'Wrong result at append3("str0"    , utf16String, utf16String)');
    arktest.assertEQ(append3(utf16String, "str1"     , utf16String), "红旗str1红旗", 'Wrong result at append3(utf16String, "str1"    , utf16String)');
    arktest.assertEQ(append3(utf16String, utf16String, "str2"     ), "红旗红旗str2", 'Wrong result at append3(utf16String, utf16String, "str2"    )');
    arktest.assertEQ(append3("str0"     , "str1"     , utf16String), "str0str1红旗", 'Wrong result at append3("str0"    , "str1"    , utf16String)');
    arktest.assertEQ(append3("str0"     , utf16String, "str2"     ), "str0红旗str2", 'Wrong result at append3("str0"    , utf16String, "str2"    )');
    arktest.assertEQ(append3(utf16String, "str1"     , "str2"     ), "红旗str1str2", 'Wrong result at append3(utf16String, "str1"    , "str2"    )');
    arktest.assertEQ(append3("str0"     , "str1"     , "str2"     ), "str0str1str2", 'Wrong result at append3("str0"    , "str1"    , "str2"    )');

    arktest.assertEQ(append4(utf16String, utf16String, utf16String, utf16String), "红旗红旗红旗红旗", 'Wrong result at append4(utf16String, utf16String, utf16String, utf16String)');
    arktest.assertEQ(append4("str0"     , utf16String, utf16String, utf16String), "str0红旗红旗红旗", 'Wrong result at append4("str0"    , utf16String, utf16String, utf16String)');
    arktest.assertEQ(append4(utf16String, "str1"     , utf16String, utf16String), "红旗str1红旗红旗", 'Wrong result at append4(utf16String, "str1"    , utf16String, utf16String)');
    arktest.assertEQ(append4(utf16String, utf16String, "str2"     , utf16String), "红旗红旗str2红旗", 'Wrong result at append4(utf16String, utf16String, "str2"    , utf16String)');
    arktest.assertEQ(append4(utf16String, utf16String, utf16String, "str3"     ), "红旗红旗红旗str3", 'Wrong result at append4(utf16String, utf16String, utf16String, "str3"    )');
    arktest.assertEQ(append4("str0"     , "str1"     , utf16String, utf16String), "str0str1红旗红旗", 'Wrong result at append4("str0"    , "str1"    , utf16String, utf16String)');
    arktest.assertEQ(append4("str0"     , utf16String, "str2"     , utf16String), "str0红旗str2红旗", 'Wrong result at append4("str0"    , utf16String, "str2"    , utf16String)');
    arktest.assertEQ(append4(utf16String, "str1"     , "str2"     , utf16String), "红旗str1str2红旗", 'Wrong result at append4(utf16String, "str1"    , "str2"    , utf16String)');
    arktest.assertEQ(append4(utf16String, "str1"     , utf16String, "str3"     ), "红旗str1红旗str3", 'Wrong result at append4(utf16String, "str1"    , utf16String, "str3"    )');
    arktest.assertEQ(append4(utf16String, utf16String, "str2"     , "str3"     ), "红旗红旗str2str3", 'Wrong result at append4(utf16String, utf16String, "str2"    , "str3"    )');
    arktest.assertEQ(append4("str0"     , "str1"     , "str2"     , utf16String), "str0str1str2红旗", 'Wrong result at append4("str0"    , "str1"    , "str2"    , utf16String)');
    arktest.assertEQ(append4("str0"     , "str1"     , utf16String, "str3"     ), "str0str1红旗str3", 'Wrong result at append4("str0"    , "str1"    , utf16String, "str3"    )');
    arktest.assertEQ(append4("str0"     , utf16String, "str2"     , "str3"     ), "str0红旗str2str3", 'Wrong result at append4("str0"    , utf16String, "str2"    , "str3"    )');
    arktest.assertEQ(append4(utf16String, "str1"     , "str2"     , "str3"     ), "红旗str1str2str3", 'Wrong result at append4(utf16String, "str1"    , "str2"    , "str3"    )');
    arktest.assertEQ(append4("str0"     , "str1"     , "str2"     , "str3"     ), "str0str1str2str3", 'Wrong result at append4("str0"    , "str1"    , "str2"    , "str3"    )');

    const emptyString = "";

    arktest.assertEQ(append2(emptyString, emptyString), ""        , 'Wrong result at append2(emptyString, emptyString)');
    arktest.assertEQ(append2("str0"     , emptyString), "str0"    , 'Wrong result at append2("str0"     , emptyString)');
    arktest.assertEQ(append2(emptyString, "str1"     ), "str1"    , 'Wrong result at append2(emptyString, "str1"     )');
    arktest.assertEQ(append2("str0"     , "str1"     ), "str0str1", 'Wrong result at append2("str0"     , "str1"     )');

    arktest.assertEQ(append3(emptyString, emptyString, emptyString), ""            , 'Wrong result at append3(emptyString, emptyString, emptyString)');
    arktest.assertEQ(append3("str0"     , emptyString, emptyString), "str0"        , 'Wrong result at append3("str0"     , emptyString, emptyString)');
    arktest.assertEQ(append3(emptyString, "str1"     , emptyString), "str1"        , 'Wrong result at append3(emptyString, "str1"     , emptyString)');
    arktest.assertEQ(append3(emptyString, emptyString, "str2"     ), "str2"        , 'Wrong result at append3(emptyString, emptyString, "str2"     )');
    arktest.assertEQ(append3("str0"     , "str1"     , emptyString), "str0str1"    , 'Wrong result at append3("str0"     , "str1"     , emptyString)');
    arktest.assertEQ(append3("str0"     , emptyString, "str2"     ), "str0str2"    , 'Wrong result at append3("str0"     , emptyString, "str2"     )');
    arktest.assertEQ(append3(emptyString, "str1"     , "str2"     ), "str1str2"    , 'Wrong result at append3(emptyString, "str1"     , "str2"     )');
    arktest.assertEQ(append3("str0"     , "str1"     , "str2"     ), "str0str1str2", 'Wrong result at append3("str0"     , "str1"     , "str2"     )');

    arktest.assertEQ(append4(emptyString, emptyString, emptyString, emptyString), ""                , 'Wrong result at append4(emptyString, emptyString, emptyString, emptyString)');
    arktest.assertEQ(append4("str0"     , emptyString, emptyString, emptyString), "str0"            , 'Wrong result at append4("str0"     , emptyString, emptyString, emptyString)');
    arktest.assertEQ(append4(emptyString, "str1"     , emptyString, emptyString), "str1"            , 'Wrong result at append4(emptyString, "str1"     , emptyString, emptyString)');
    arktest.assertEQ(append4(emptyString, emptyString, "str2"     , emptyString), "str2"            , 'Wrong result at append4(emptyString, emptyString, "str2"     , emptyString)');
    arktest.assertEQ(append4(emptyString, emptyString, emptyString, "str3"     ), "str3"            , 'Wrong result at append4(emptyString, emptyString, emptyString, "str3"     )');
    arktest.assertEQ(append4("str0"     , "str1"     , emptyString, emptyString), "str0str1"        , 'Wrong result at append4("str0"     , "str1"     , emptyString, emptyString)');
    arktest.assertEQ(append4("str0"     , emptyString, "str2"     , emptyString), "str0str2"        , 'Wrong result at append4("str0"     , emptyString, "str2"     , emptyString)');
    arktest.assertEQ(append4(emptyString, "str1"     , "str2"     , emptyString), "str1str2"        , 'Wrong result at append4(emptyString, "str1"     , "str2"     , emptyString)');
    arktest.assertEQ(append4(emptyString, "str1"     , emptyString, "str3"     ), "str1str3"        , 'Wrong result at append4(emptyString, "str1"     , emptyString, "str3"     )');
    arktest.assertEQ(append4(emptyString, emptyString, "str2"     , "str3"     ), "str2str3"        , 'Wrong result at append4(emptyString, emptyString, "str2"     , "str3"     )');
    arktest.assertEQ(append4("str0"     , "str1"     , "str2"     , emptyString), "str0str1str2"    , 'Wrong result at append4("str0"     , "str1"     , "str2"     , emptyString)');
    arktest.assertEQ(append4("str0"     , "str1"     , emptyString, "str3"     ), "str0str1str3"    , 'Wrong result at append4("str0"     , "str1"     , emptyString, "str3"     )');
    arktest.assertEQ(append4("str0"     , emptyString, "str2"     , "str3"     ), "str0str2str3"    , 'Wrong result at append4("str0"     , emptyString, "str2"     , "str3"     )');
    arktest.assertEQ(append4(emptyString, "str1"     , "str2"     , "str3"     ), "str1str2str3"    , 'Wrong result at append4(emptyString, "str1"     , "str2"     , "str3"     )');
    arktest.assertEQ(append4("str0"     , "str1"     , "str2"     , "str3"     ), "str0str1str2str3", 'Wrong result at append4("str0"     , "str1"     , "str2"     , "str3"     )');

    return 0
}

//! CHECKER       BugFix: Wrong index at slowpath implementation of StringBuilder.append2 JIT (IARLQ5/19441)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-reserve-string-builder-buffer=false", entry: "ets_string_builder_append_merge_part4.ETSGLOBAL::bugfix_19441"
//! EVENTS_COUNT  /SbBufferRealloc,\d+,0x[0-9a-fA-F]+,32.*/,4

function bugfix_19441_append17() : string {
    let sb = new StringBuilder();

    // Fill StringBuilder internal buffer with 16 elements
    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");
    sb.append("8");
    sb.append("9");
    sb.append("10");
    sb.append("11");
    sb.append("12");
    sb.append("13");
    sb.append("14");
    sb.append("15");

    // Call to StringBuilder.append via slowpath
    sb.append("16");

    return sb.toString();
}

function bugfix_19441_append18() : string {
    let sb = new StringBuilder();

    // Fill StringBuilder internal buffer with 16 elements
    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");
    sb.append("8");
    sb.append("9");
    sb.append("10");
    sb.append("11");
    sb.append("12");
    sb.append("13");
    sb.append("14");
    sb.append("15");

    // Call to StringBuilder.append2 via slowpath
    sb.append("16");
    sb.append("17");

    return sb.toString();
}

function bugfix_19441_append19() : string {
    let sb = new StringBuilder();

    // Fill StringBuilder internal buffer with 16 elements
    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");
    sb.append("8");
    sb.append("9");
    sb.append("10");
    sb.append("11");
    sb.append("12");
    sb.append("13");
    sb.append("14");
    sb.append("15");

    // Call to StringBuilder.append3 via slowpath
    sb.append("16");
    sb.append("17");
    sb.append("18");

    return sb.toString();
}

function bugfix_19441_append20() : string {
    let sb = new StringBuilder();

    // Fill StringBuilder internal buffer with 16 elements
    sb.append("0");
    sb.append("1");
    sb.append("2");
    sb.append("3");
    sb.append("4");
    sb.append("5");
    sb.append("6");
    sb.append("7");
    sb.append("8");
    sb.append("9");
    sb.append("10");
    sb.append("11");
    sb.append("12");
    sb.append("13");
    sb.append("14");
    sb.append("15");

    // Call to StringBuilder.append4 via slowpath
    sb.append("16");
    sb.append("17");
    sb.append("18");
    sb.append("19");

    return sb.toString();
}

function bugfix_19441() : int {
    arktest.assertEQ(bugfix_19441_append17(), "012345678910111213141516"      , 'Wrong result at bugfix_19441_append17()');
    arktest.assertEQ(bugfix_19441_append18(), "01234567891011121314151617"    , 'Wrong result at bugfix_19441_append18()');
    arktest.assertEQ(bugfix_19441_append19(), "0123456789101112131415161718"  , 'Wrong result at bugfix_19441_append19()');
    arktest.assertEQ(bugfix_19441_append20(), "012345678910111213141516171819", 'Wrong result at bugfix_19441_append20()');

    return 0;
}
