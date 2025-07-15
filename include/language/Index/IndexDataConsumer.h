//===--- IndexDataConsumer.h - Consumer of indexing information -*- C++ -*-===//
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

#ifndef LANGUAGE_INDEX_INDEXDATACONSUMER_H
#define LANGUAGE_INDEX_INDEXDATACONSUMER_H

#include "language/Index/IndexSymbol.h"

namespace language {
namespace index {

class IndexDataConsumer {
  virtual void anchor();

public:
  enum Action {Skip, Abort, Continue};

  virtual ~IndexDataConsumer() {}

  virtual bool enableWarnings() { return false; }
  virtual bool indexLocals() { return false; }

  virtual void failed(StringRef error) = 0;
  virtual void warning(StringRef warning) {}

  virtual bool startDependency(StringRef name, StringRef path, bool isClangModule,
                               bool isSystem) = 0;
  virtual bool finishDependency(bool isClangModule) = 0;
  virtual Action startSourceEntity(const IndexSymbol &symbol) = 0;
  virtual bool finishSourceEntity(SymbolInfo symInfo, SymbolRoleSet roles) = 0;

  virtual void finish() {}
};

} // end namespace index
} // end namespace language

#endif // LANGUAGE_INDEX_INDEXDATACONSUMER_H
