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

#include <cstdlib>
#include "disassembler.h"
#include <gtest/gtest.h>
#include <string>

using namespace testing::ext;
namespace panda::disasm {

static const std::string MODULE_REQUEST_FILE_NAME = GRAPH_TEST_ABC_DIR "module-requests-annotation-import.abc";
static const std::string SLOT_NUMBER_FILE_NAME = GRAPH_TEST_ABC_DIR "slot-number-annotation.abc";

class DisassemblerSystemAnnotationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    bool ValidateAnnotation(std::optional<std::vector<std::string>> &annotations, const std::string &annotation_name)
    {
        std::vector<std::string> ann = annotations.value();
        const auto ann_iter = std::find(ann.begin(), ann.end(), annotation_name);
        if (ann_iter != ann.end()) {
            return true;
        }

        return false;
    }
};

/**
* @tc.name: disassembler_system_annotation_test_001
* @tc.desc: get module request annotation of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerSystemAnnotationTest, disassembler_system_annotation_test_001, TestSize.Level1)
{
    static const std::string METHOD_NAME = "module-requests-annotation-import.#*#funcD";
    static const std::string ANNOTATION_NAME = "L_ESConcurrentModuleRequestsAnnotation";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(MODULE_REQUEST_FILE_NAME, false, false);
    std::optional<std::vector<std::string>> annotations = disasm.GetAnnotationByMethodName(METHOD_NAME);
    ASSERT_NE(annotations, std::nullopt);
    bool hasAnnotation = ValidateAnnotation(annotations, ANNOTATION_NAME);
    EXPECT_TRUE(hasAnnotation);
}

/**
* @tc.name: disassembler_system_annotation_test_002
* @tc.desc: get solt number annotation of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerSystemAnnotationTest, disassembler_system_annotation_test_002, TestSize.Level1)
{
    static const std::string METHOD_NAME = "#*#funcA";
    static const std::string ANNOTATION_NAME = "L_ESSlotNumberAnnotation";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(SLOT_NUMBER_FILE_NAME, false, false);
    std::optional<std::vector<std::string>> annotations = disasm.GetAnnotationByMethodName(METHOD_NAME);
    ASSERT_NE(annotations, std::nullopt);
    bool hasAnnotation = ValidateAnnotation(annotations, ANNOTATION_NAME);
    EXPECT_TRUE(hasAnnotation);
}
};
