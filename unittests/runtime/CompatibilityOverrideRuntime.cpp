//===--- CompatibilityOverrideRuntime.cpp - Compatibility override tests ---===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#if defined(__APPLE__) && defined(__MACH__)

#define LANGUAGE_TARGET_LIBRARY_NAME languageRuntime

#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverride.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/Metadata.h"
#include "gtest/gtest.h"

#include <stdio.h>

using namespace language;

bool EnableOverride;
bool Ran;

namespace  {
  template<typename T>
  T getEmptyValue() {
    return (T)0;
  }

  template<>
  MetadataResponse getEmptyValue<MetadataResponse>() {
    return MetadataResponse{nullptr, MetadataState::Complete};
  }

  template <>
  TypeLookupErrorOr<TypeInfo> getEmptyValue<TypeLookupErrorOr<TypeInfo>>() {
    return TypeInfo();
  }
}

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs)   \
  static ccAttrs ret name##Override(COMPATIBILITY_UNPAREN_WITH_COMMA(          \
      typedArgs) Original_##name originalImpl) {                               \
    if (!EnableOverride)                                                       \
      return originalImpl COMPATIBILITY_PAREN(namedArgs);                      \
    Ran = true;                                                                \
    return getEmptyValue<ret>();                                               \
  }
#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverrideRuntime.def"

struct OverrideSection {
  uintptr_t version;
  
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name name;
#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverrideRuntime.def"
};

OverrideSection RuntimeOverrides
    __attribute__((section("__DATA," COMPATIBILITY_OVERRIDE_SECTION_NAME_languageRuntime))) = {
        0,
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  name ## Override,
#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverrideRuntime.def"
};

class CompatibilityOverrideRuntimeTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    // This check is a bit pointless, as long as you trust the section
    // attribute to work correctly, but it ensures that the override
    // section actually gets linked into the test executable. If it's not
    // used then it disappears and the real tests begin to fail.
    ASSERT_EQ(RuntimeOverrides.version, static_cast<uintptr_t>(0));

    EnableOverride = true;
    Ran = false;
  }
  
  virtual void TearDown() {
    EnableOverride = false;
    ASSERT_TRUE(Ran);
  }
};

TEST_F(CompatibilityOverrideRuntimeTest, test_language_dynamicCast) {
  auto Result = language_dynamicCast(nullptr, nullptr, nullptr, nullptr,
                                  DynamicCastFlags::Default);
  ASSERT_FALSE(Result);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_dynamicCastClass) {
  auto Result = language_dynamicCastClass(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastClassUnconditional) {
  auto Result = language_dynamicCastClassUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_dynamicCastObjCClass) {
  auto Result = language_dynamicCastObjCClass(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastObjCClassUnconditional) {
  auto Result = language_dynamicCastObjCClassUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_dynamicCastForeignClass) {
  auto Result = language_dynamicCastForeignClass(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastForeignClassUnconditional) {
  auto Result = language_dynamicCastForeignClassUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_dynamicCastUnknownClass) {
  auto Result = language_dynamicCastUnknownClass(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastUnknownClassUnconditional) {
  auto Result = language_dynamicCastUnknownClassUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_dynamicCastMetatype) {
  auto Result = language_dynamicCastMetatype(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastMetatypeUnconditional) {
  auto Result = language_dynamicCastMetatypeUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastObjCClassMetatype) {
  auto Result = language_dynamicCastObjCClassMetatype(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastObjCClassMetatypeUnconditional) {
  auto Result = language_dynamicCastObjCClassMetatypeUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastForeignClassMetatype) {
  auto Result = language_dynamicCastForeignClassMetatype(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_dynamicCastForeignClassMetatypeUnconditional) {
  auto Result = language_dynamicCastForeignClassMetatypeUnconditional(nullptr, nullptr, nullptr, 0, 0);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_conformsToProtocol) {
  auto Result = language_conformsToProtocol(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_conformsToProtocol2) {
  auto Result = language_conformsToProtocol2(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_conformsToProtocolCommon) {
  auto Result = language_conformsToProtocolCommon(nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_getTypeByMangledNode) {
  Demangler demangler;
  auto Result = language_getTypeByMangledNode(MetadataState::Abstract,
                                           demangler, nullptr, nullptr, nullptr,nullptr);
  ASSERT_EQ(Result.getType().getMetadata(), nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest, test_language_getTypeByMangledName) {
  auto Result = language_getTypeByMangledName(MetadataState::Abstract,
                                           "", nullptr, nullptr, nullptr);
  ASSERT_EQ(Result.getType().getMetadata(), nullptr);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_getAssociatedTypeWitnessSlow) {
  auto Result = language_getAssociatedTypeWitnessSlow(MetadataState::Complete,
                                                   nullptr, nullptr,
                                                   nullptr, nullptr);
  ASSERT_EQ(Result.Value, nullptr);
  ASSERT_EQ(Result.State, MetadataState::Complete);
}

TEST_F(CompatibilityOverrideRuntimeTest,
       test_language_getAssociatedConformanceWitnessSlow) {
  auto Result = language_getAssociatedConformanceWitnessSlow(
                                                   nullptr, nullptr, nullptr,
                                                   nullptr, nullptr);
  ASSERT_EQ(Result, nullptr);
}

#endif
