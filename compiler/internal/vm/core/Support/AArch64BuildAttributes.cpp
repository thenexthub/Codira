//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/AArch64BuildAttributes.h"
#include "vm/core/ADT/StringSwitch.h"

using namespace vm::core;
using namespace vm::core::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  switch (Vendor) {
  case AEABI_FEATURE_AND_BITS:
    return "aeabi_feature_and_bits";
  case AEABI_PAUTHABI:
    return "aeabi_pauthabi";
  case VENDOR_UNKNOWN:
    return "";
  default:
    assert(0 && "Vendor name error");
    return "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  return StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  switch (Optional) {
  case REQUIRED:
    return "required";
  case OPTIONAL:
    return "optional";
  case OPTIONAL_NOT_FOUND:
  default:
    return "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  return StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  return "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  switch (Type) {
  case ULEB128:
    return "uleb128";
  case NTBS:
    return "ntbs";
  case TYPE_NOT_FOUND:
  default:
    return "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  return StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  return "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  switch (PauthABITag) {
  case TAG_PAUTH_PLATFORM:
    return "Tag_PAuth_Platform";
  case TAG_PAUTH_SCHEMA:
    return "Tag_PAuth_Schema";
  case PAUTHABI_TAG_NOT_FOUND:
  default:
    return "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  return StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  switch (FeatureAndBitsTag) {
  case TAG_FEATURE_BTI:
    return "Tag_Feature_BTI";
  case TAG_FEATURE_PAC:
    return "Tag_Feature_PAC";
  case TAG_FEATURE_GCS:
    return "Tag_Feature_GCS";
  case FEATURE_AND_BITS_TAG_NOT_FOUND:
  default:
    return "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  return StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
