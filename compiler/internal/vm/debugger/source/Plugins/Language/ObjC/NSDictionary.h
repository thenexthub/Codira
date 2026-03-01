//===-- NSDictionary.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_NSDICTIONARY_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_NSDICTIONARY_H

#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/DataFormatters/TypeSynthetic.h"
#include "lldb/Utility/ConstString.h"
#include "lldb/Utility/Stream.h"
#include "lldb/ValueObject/ValueObject.h"

#include <map>
#include <memory>

namespace lldb_private {
namespace formatters {
template <bool name_entries>
bool NSDictionarySummaryProvider(ValueObject &valobj, Stream &stream,
                                 const TypeSummaryOptions &options);

extern template bool
NSDictionarySummaryProvider<true>(ValueObject &, Stream &,
                                  const TypeSummaryOptions &);

extern template bool
NSDictionarySummaryProvider<false>(ValueObject &, Stream &,
                                   const TypeSummaryOptions &);

SyntheticChildrenFrontEnd *
NSDictionarySyntheticFrontEndCreator(CXXSyntheticChildren *,
                                     lldb::ValueObjectSP);

class NSDictionary_Additionals {
public:
  class AdditionalFormatterMatching {
  public:
    class Matcher {
    public:
      virtual ~Matcher() = default;
      virtual bool Match(ConstString class_name) = 0;

      typedef std::unique_ptr<Matcher> UP;
    };
    class Prefix : public Matcher {
    public:
      Prefix(ConstString p);
      ~Prefix() override = default;
      bool Match(ConstString class_name) override;

    private:
      ConstString m_prefix;
    };
    class Full : public Matcher {
    public:
      Full(ConstString n);
      ~Full() override = default;
      bool Match(ConstString class_name) override;

    private:
      ConstString m_name;
    };
    typedef Matcher::UP MatcherUP;

    MatcherUP GetFullMatch(ConstString n) { return std::make_unique<Full>(n); }

    MatcherUP GetPrefixMatch(ConstString p) {
      return std::make_unique<Prefix>(p);
    }
  };

  template <typename FormatterType>
  using AdditionalFormatter =
      std::pair<AdditionalFormatterMatching::MatcherUP, FormatterType>;

  template <typename FormatterType>
  using AdditionalFormatters = std::vector<AdditionalFormatter<FormatterType>>;

  static AdditionalFormatters<CXXFunctionSummaryFormat::Callback> &
  GetAdditionalSummaries();

  static AdditionalFormatters<CXXSyntheticChildren::CreateFrontEndCallback> &
  GetAdditionalSynthetics();
};
} // namespace formatters
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_NSDICTIONARY_H
