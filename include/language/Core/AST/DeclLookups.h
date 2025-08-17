//===- DeclLookups.h - Low-level interface to all names in a DC -*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
//  This file defines DeclContext::all_lookups_iterator.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_DECLLOOKUPS_H
#define LANGUAGE_CORE_AST_DECLLOOKUPS_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclContextInternals.h"
#include "language/Core/AST/DeclarationName.h"
#include "language/Core/AST/ExternalASTSource.h"
#include <cstddef>
#include <iterator>

namespace language::Core {

/// all_lookups_iterator - An iterator that provides a view over the results
/// of looking up every possible name.
class DeclContext::all_lookups_iterator {
  StoredDeclsMap::iterator It, End;

public:
  using value_type = lookup_result;
  using reference = lookup_result;
  using pointer = lookup_result;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;

  all_lookups_iterator() = default;
  all_lookups_iterator(StoredDeclsMap::iterator It,
                       StoredDeclsMap::iterator End)
      : It(It), End(End) {}

  DeclarationName getLookupName() const { return It->first; }

  reference operator*() const { return It->second.getLookupResult(); }
  pointer operator->() const { return It->second.getLookupResult(); }

  all_lookups_iterator& operator++() {
    // Filter out using directives. They don't belong as results from name
    // lookup anyways, except as an implementation detail. Users of the API
    // should not expect to get them (or worse, rely on it).
    do {
      ++It;
    } while (It != End &&
             It->first == DeclarationName::getUsingDirectiveName());

    return *this;
  }

  all_lookups_iterator operator++(int) {
    all_lookups_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  friend bool operator==(all_lookups_iterator x, all_lookups_iterator y) {
    return x.It == y.It;
  }

  friend bool operator!=(all_lookups_iterator x, all_lookups_iterator y) {
    return x.It != y.It;
  }
};

inline DeclContext::lookups_range DeclContext::lookups() const {
  DeclContext *Primary = const_cast<DeclContext*>(this)->getPrimaryContext();
  if (Primary->hasExternalVisibleStorage())
    getParentASTContext().getExternalSource()->completeVisibleDeclsMap(Primary);
  if (StoredDeclsMap *Map = Primary->buildLookup())
    return lookups_range(all_lookups_iterator(Map->begin(), Map->end()),
                         all_lookups_iterator(Map->end(), Map->end()));

  // Synthesize an empty range. This requires that two default constructed
  // versions of these iterators form a valid empty range.
  return lookups_range(all_lookups_iterator(), all_lookups_iterator());
}

inline DeclContext::lookups_range
DeclContext::noload_lookups(bool PreserveInternalState) const {
  DeclContext *Primary = const_cast<DeclContext*>(this)->getPrimaryContext();
  if (!PreserveInternalState)
    Primary->loadLazyLocalLexicalLookups();
  if (StoredDeclsMap *Map = Primary->getLookupPtr())
    return lookups_range(all_lookups_iterator(Map->begin(), Map->end()),
                         all_lookups_iterator(Map->end(), Map->end()));

  // Synthesize an empty range. This requires that two default constructed
  // versions of these iterators form a valid empty range.
  return lookups_range(all_lookups_iterator(), all_lookups_iterator());
}

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_DECLLOOKUPS_H
