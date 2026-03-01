/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <memory>
#include <sstream>

#include <android-base/file.h>
#include <memory_trace/TraceInfo.h>

#include "OpGenAnalysis.h"
#include "OpOverlapAnalysis.h"

class AnalysisTest : public ::testing::Test {
 protected:
  template <typename Analysis>
  void Gather(std::stringstream& res) {
    auto analysis = std::make_unique<Analysis>();
    analysis->Gather(*trace_info_);
    analysis->StreamResult(res, "  ");
  }

  void InitTraceInfo(const char* contents) {
    TemporaryFile tf;
    ASSERT_NE(-1, tf.fd);
    ASSERT_TRUE(android::base::WriteStringToFd(contents, tf.fd));
    close(tf.fd);

    trace_info_.reset(new memory_trace::TraceInfo);
    trace_info_->Init(tf.path);
  }

 private:
  std::unique_ptr<memory_trace::TraceInfo> trace_info_;
};

TEST_F(AnalysisTest, Average) {
  const char* const entries =
      "1234: malloc 0x0 32 1 1101\n"
      "1234: malloc 0x10 32 2001 3001\n"
      "1234: malloc 0x20 32 4001 4901\n";
  ASSERT_NO_FATAL_FAILURE(InitTraceInfo(entries));

  std::stringstream result;
  Gather<OpAverageAnalysis>(result);

  std::string expected =
      "  malloc: avg exec time = 1000.0 ns\n"
      "  calloc: avg exec time = 0.0 ns\n"
      "  memalign: avg exec time = 0.0 ns\n"
      "  realloc: avg exec time = 0.0 ns\n"
      "  free: avg exec time = 0.0 ns\n";
  EXPECT_EQ(expected, result.str());
}

TEST_F(AnalysisTest, MinMax) {
  const char* const entries =
      "1234: malloc 0x0 32 1 1001\n"
      "1234: malloc 0x10 32 2001 3001\n"
      "1234: malloc 0x20 32 4001 5001\n";
  ASSERT_NO_FATAL_FAILURE(InitTraceInfo(entries));

  std::stringstream result;
  Gather<OpMinMaxAnalysis>(result);

  std::string expected =
      "  malloc: min exec time = 1000 ns, max exec time = 1000 ns\n"
      "  calloc: min exec time = 0 ns, max exec time = 0 ns\n"
      "  memalign: min exec time = 0 ns, max exec time = 0 ns\n"
      "  realloc: min exec time = 0 ns, max exec time = 0 ns\n"
      "  free: min exec time = 0 ns, max exec time = 0 ns\n";
  EXPECT_EQ(expected, result.str());
}

TEST_F(AnalysisTest, Overlap) {
  const char* const entries =
      "1234: malloc 0x0 32 1 1001\n"
      "1235: malloc 0x30 256 600 800\n"
      "1236: malloc 0x10 48 500 2500\n"
      "1237: malloc 0x20 128 2200 3200\n";
  ASSERT_NO_FATAL_FAILURE(InitTraceInfo(entries));

  std::stringstream result;
  Gather<OpOverlapAnalysis>(result);

  EXPECT_EQ("  Overlap of all operations is 1001 ns\n", result.str());
}
