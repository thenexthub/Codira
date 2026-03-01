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
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include "abc2program_driver.h"
#include "abc2program_test_utils.h"
#include "common/abc_file_utils.h"
#include "dump_utils.h"
#include "modifiers.h"

using namespace testing::ext;

namespace panda::abc2program {

struct FuncName {
    std::string es = "default.#~ES=#ES";
    std::string ets = "ets.#~AT=#AT";
    std::string js = "js.#~JS=#JS";
    std::string ts = "ts.#*#TS";
};

struct RecordName {
    std::string defaultRec = "default";
    std::string ets = "ets";
    std::string js = "js";
    std::string ts = "ts";
    std::string tsAnno = "ts.TsAnno";
    std::string etsStrAnno = "ets.EtsStringAnno";
    std::string pandaString = "panda.String";
};

const FuncName FUNC_NAME;
const RecordName RECORD_NAME;
const std::string SOURCE_LANG_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "sourceLang.abc";

static const pandasm::Function *GetFunction(const std::string &name,
                                            const pandasm::Program &program)
{
    auto it = program.function_table.find(name);
    if (it == program.function_table.end()) {
        return nullptr;
    }
    return &(it->second);
}

static const pandasm::Record *GetRecord(const std::string &name,
                                        const pandasm::Program &program)
{
    return &(program.record_table.find(name)->second);
}

class Abc2ProgramSourceLangTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(SOURCE_LANG_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
        es_function_ = GetFunction(FUNC_NAME.es, *prog_);
        ASSERT_NE(es_function_, nullptr);
        js_function_ = GetFunction(FUNC_NAME.js, *prog_);
        ASSERT_NE(js_function_, nullptr);
        ts_function_ = GetFunction(FUNC_NAME.ts, *prog_);
        ASSERT_NE(ts_function_, nullptr);
        ets_function_ = GetFunction(FUNC_NAME.ets, *prog_);
        ASSERT_NE(ets_function_, nullptr);

        es_record_ = GetRecord(RECORD_NAME.defaultRec, *prog_);
        ASSERT_NE(es_record_, nullptr);
        js_record_ = GetRecord(RECORD_NAME.js, *prog_);
        ASSERT_NE(js_record_, nullptr);
        ts_record_ = GetRecord(RECORD_NAME.ts, *prog_);
        ASSERT_NE(ts_record_, nullptr);
        ets_record_ = GetRecord(RECORD_NAME.ets, *prog_);
        ASSERT_NE(ets_record_, nullptr);

        ets_str_anno_record_ = GetRecord(RECORD_NAME.etsStrAnno, *prog_);
        ASSERT_NE(ets_str_anno_record_, nullptr);
        ts_anno_record_ = GetRecord(RECORD_NAME.tsAnno, *prog_);
        ASSERT_NE(ts_anno_record_, nullptr);
        panda_string_record_ = GetRecord(RECORD_NAME.pandaString, *prog_);
        ASSERT_NE(panda_string_record_, nullptr);
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
    const pandasm::Function *es_function_ = nullptr;
    const pandasm::Function *js_function_ = nullptr;
    const pandasm::Function *ts_function_ = nullptr;
    const pandasm::Function *ets_function_ = nullptr;

    const pandasm::Record *es_record_ = nullptr;
    const pandasm::Record *ts_record_ = nullptr;
    const pandasm::Record *js_record_ = nullptr;
    const pandasm::Record *ets_record_ = nullptr;
    const pandasm::Record *ets_str_anno_record_ = nullptr;
    const pandasm::Record *ts_anno_record_ = nullptr;
    const pandasm::Record *panda_string_record_ = nullptr;
};

/*------------------------------------- Cases of release mode below -------------------------------------*/

/**
 * @tc.name: abc2program_hello_world_test_dump
 * @tc.desc: check dump result in release mode.
 * @tc.type: FUNC
 * @tc.require: IADG92
 */
HWTEST_F(Abc2ProgramSourceLangTest, abc2program_source_lang_test, TestSize.Level1)
{
    EXPECT_EQ(es_function_->language, pandasm::extensions::Language::ECMASCRIPT);
    EXPECT_EQ(js_function_->language, pandasm::extensions::Language::JAVASCRIPT);
    EXPECT_EQ(ts_function_->language, pandasm::extensions::Language::TYPESCRIPT);
    EXPECT_EQ(ets_function_->language, pandasm::extensions::Language::ARKTS);

    EXPECT_EQ(es_record_->language, pandasm::extensions::Language::ECMASCRIPT);
    EXPECT_EQ(ts_record_->language, pandasm::extensions::Language::TYPESCRIPT);
    EXPECT_EQ(js_record_->language, pandasm::extensions::Language::JAVASCRIPT);
    EXPECT_EQ(ets_record_->language, pandasm::extensions::Language::ARKTS);

    EXPECT_EQ(ets_str_anno_record_->language, pandasm::extensions::Language::ARKTS);
    EXPECT_EQ(ts_anno_record_->language, pandasm::extensions::Language::TYPESCRIPT);
    EXPECT_EQ(panda_string_record_->language, pandasm::extensions::Language::ECMASCRIPT);
}
};  // panda::abc2program