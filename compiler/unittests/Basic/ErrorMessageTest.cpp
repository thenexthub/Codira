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

#include "gtest/gtest.h"
#include "Codira/AST/Node.h"
#include "Codira/Basic/Display.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;

TEST(ErrorMessageTest, MassageTest)
{
    static std::unordered_map<std::string, int> columnMap = {
        {"/*中中中*/ aaa 2\n", 15},
        {"\t/*中*/ aaa 2\n", 15},
        {"/*Ａ Ｂ Ｃ Ｄ Ｅ Ｆ Ｇ Ｈ Ｉ Ｊ Ｋ Ｌ Ｍ Ｎ Ｏ Ｐ Ｑ Ｒ Ｓ Ｔ Ｕ Ｖ Ｗ Ｘ Ｙ Ｚ*/ aaa 2\n", 86},
        {"/*∀ ∁ ∂ ∃ ∄ ∅ ∆ ∇ ∈ ∉ ∊ ∋ ∌ ∍ ∎ ∏ ∐ ∑ − ∓ ∔ ∕  ∗ °  √ ∛ ∜*/ aaa 2\n", 64},
        {"/*እው ሰላም ነው. እንዴት ነህ?*/ aaa 2\n", 28},
        {"/**Je t’aime*/ aaa 2\n", 19},
        {"/*Σ΄αγαπώ (Se agapo)*/ aaa 2\n", 27},
        {"/*你好。 你好吗？*/ aaa 2\n", 24},
        {"/*愛してる*/ aaa 2\n", 17},
        {"/*사랑해 (Saranghae)*/ aaa 2\n", 27},
        {"/*Я тебя люблю (Ya tebya liubliu)*/ aaa 2\n", 40},
        {"/* ?*/ aaa 2\n", 11},
        {"/*நீங்கள் எப்படி இருக்கிறீர்கள்?*/ aaa 2\n", 31},
        {"/*ਤੁਸੀਂ ਕਿਵੇਂ ਹੋ?*/ aaa 2\n", 19},
        {"/*👩  <200d>🔬  */ aaa 2\n", 21},
        {"/*𝓽𝓱𝓲𝓼 𝓲𝓼 𝓬𝓸𝓸𝓵*/ aaa 2\n", 21},
        {"/*(-■_■)*/ aaa 2\n", 15},
        {"/*(☞ﾟ∀ﾟ)☞*/ aaa 2\n", 16},
        {"/*         */ aaa 2\n", 18},
        {"/*／人 ◕ ‿‿ ◕ 人＼*/ aaa 2\n", 25},
        {"/*▣ ■ □ ▢ ◯ ▲ ▶ ► ▼ ◆ ◢ ◣ ◤ ◥*/ aaa 2\n", 36},
        {"/*₁₂₃₄*/ aaa 2\n", 13},
        {"/*Μένω στους Παξούς*/ aaa 2\n", 26},
    };
    std::string code = "func test() {\n";
    std::for_each(columnMap.begin(), columnMap.end(), [&](auto& c) { code += c.first; });
    code += "}\n";

    SourceManager sm;
    DiagnosticEngine diag;
    diag.SetSourceManager(&sm);
    std::unique_ptr<Parser> parser = std::make_unique<Parser>(1, code, diag, sm);
    parser->ParseTopLevel();

    std::vector<Diagnostic> ds = diag.GetCategoryDiagnostic(DiagCategory::PARSE);

    EXPECT_TRUE(!ds.empty()) << "Expected error in parser";
    std::for_each(ds.begin(), ds.end(), [&](auto& d) {
        auto source = sm.GetContentBetween(
            d.start.fileID, Position(d.start.line, 1), Position(d.start.line, std::numeric_limits<int>::max()));
        if (columnMap.count(source)) {
            auto printColumn = GetSpaceBeforeTarget(source, d.mainHint.range.begin.column);
            EXPECT_EQ(printColumn.length(), columnMap.at(source)) << "Column space differ in line" << source;
        }
    });
}
