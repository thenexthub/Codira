#include "language/IDE/CodeCompletion.h"
#include "gtest/gtest.h"

using namespace language;
using namespace ide;

static std::string replaceAtWithNull(const std::string &S) {
  std::string Result = S;
  for (char &C : Result) {
    if (C == '@')
      C = '\0';
  }
  return Result;
}

TEST(CodeCompletionToken, FindInEmptyFile) {
  std::string Source = "";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(~0U, Offset);
  EXPECT_EQ("", Clean);
}

TEST(CodeCompletionToken, FindNonExistent) {
  std::string Source = "fn zzz() {}";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(~0U, Offset);
  EXPECT_EQ(Source, Clean);
}

TEST(CodeCompletionToken, RemovesOtherTokens) {
  std::string Source = "fn zzz() {#^B^#}";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(~0U, Offset);
  EXPECT_EQ("fn zzz() {}", Clean);
}

TEST(CodeCompletionToken, FindBegin) {
  std::string Source = "#^A^# fn";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(0U, Offset);
  EXPECT_EQ(replaceAtWithNull("@ fn"), Clean);
}

TEST(CodeCompletionToken, FindEnd) {
  std::string Source = "fn #^A^#";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(5U, Offset);
  EXPECT_EQ(replaceAtWithNull("fn @"), Clean);
}

TEST(CodeCompletionToken, FindSingleLine) {
  std::string Source = "fn zzz() {#^A^#}";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(12U, Offset);
  EXPECT_EQ(replaceAtWithNull("fn zzz() {@}"), Clean);
}

TEST(CodeCompletionToken, FindMultiline) {
  std::string Source =
      "fn zzz() {\n"
      "  1 + #^A^#\r\n"
      "}\n";
  unsigned Offset;
  std::string Clean = removeCodeCompletionTokens(Source, "A", &Offset);
  EXPECT_EQ(19U, Offset);
  EXPECT_EQ(replaceAtWithNull("fn zzz() {\n  1 + @\r\n}\n"), Clean);
}

