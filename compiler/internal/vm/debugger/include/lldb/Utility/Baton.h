//===-- Baton.h -------------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_BATON_H
#define LLDB_UTILITY_BATON_H

#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-public.h"

#include "llvm/Support/raw_ostream.h"

#include <memory>

namespace lldb_private {
class Stream;
}

namespace lldb_private {

/// \class Baton Baton.h "lldb/Core/Baton.h"
/// A class designed to wrap callback batons so they can cleanup
///        any acquired resources
///
/// This class is designed to be used by any objects that have a callback
/// function that takes a baton where the baton might need to
/// free/delete/close itself.
///
/// The default behavior is to not free anything. Subclasses can free any
/// needed resources in their destructors.
class Baton {
public:
  Baton() = default;
  virtual ~Baton() = default;

  virtual void *data() = 0;

  virtual void GetDescription(llvm::raw_ostream &s,
                              lldb::DescriptionLevel level,
                              unsigned indentation) const = 0;
};

class UntypedBaton : public Baton {
public:
  UntypedBaton(void *Data) : m_data(Data) {}
  ~UntypedBaton() override {
    // The default destructor for an untyped baton does NOT attempt to clean up
    // anything in m_data.
  }

  void *data() override { return m_data; }
  void GetDescription(llvm::raw_ostream &s, lldb::DescriptionLevel level,
                      unsigned indentation) const override;

  void *m_data; // Leave baton public for easy access
};

template <typename T> class TypedBaton : public Baton {
public:
  explicit TypedBaton(std::unique_ptr<T> Item) : Item(std::move(Item)) {}

  T *getItem() { return Item.get(); }
  const T *getItem() const { return Item.get(); }

  void *data() override { return Item.get(); }
  void GetDescription(llvm::raw_ostream &s, lldb::DescriptionLevel level,
                      unsigned indentation) const override {}

protected:
  std::unique_ptr<T> Item;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_BATON_H
