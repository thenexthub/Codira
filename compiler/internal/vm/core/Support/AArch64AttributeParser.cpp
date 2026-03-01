//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://toolchain.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#include "vm/core/Support/AArch64AttributeParser.h"
#include "vm/core/Support/AArch64BuildAttributes.h"

std::vector<toolchain::SubsectionAndTagToTagName> &
toolchain::AArch64AttributeParser::returnTagsNamesMap() {
  static std::vector<SubsectionAndTagToTagName> TagsNamesMap = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  return TagsNamesMap;
}

toolchain::AArch64BuildAttrSubsections toolchain::extractBuildAttributesSubsections(
    const toolchain::AArch64AttributeParser &Attributes) {

  toolchain::AArch64BuildAttrSubsections SubSections;
  auto GetPauthValue = [&Attributes](unsigned Tag) {
    return Attributes.getAttributeValue("aeabi_pauthabi", Tag).value_or(0);
  };
  SubSections.Pauth.TagPlatform =
      GetPauthValue(toolchain::AArch64BuildAttributes::TAG_PAUTH_PLATFORM);
  SubSections.Pauth.TagSchema =
      GetPauthValue(toolchain::AArch64BuildAttributes::TAG_PAUTH_SCHEMA);

  auto GetFeatureValue = [&Attributes](unsigned Tag) {
    return Attributes.getAttributeValue("aeabi_feature_and_bits", Tag)
        .value_or(0);
  };
  SubSections.AndFeatures |=
      GetFeatureValue(toolchain::AArch64BuildAttributes::TAG_FEATURE_BTI);
  SubSections.AndFeatures |=
      GetFeatureValue(toolchain::AArch64BuildAttributes::TAG_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      GetFeatureValue(toolchain::AArch64BuildAttributes::TAG_FEATURE_GCS) << 2;

  return SubSections;
}
