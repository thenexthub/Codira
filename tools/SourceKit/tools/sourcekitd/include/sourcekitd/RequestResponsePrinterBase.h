//===--- RequestResponsePrinterBase.h - -------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKITD_REQUESTRESPONSEPRINTERBASE_H
#define LLVM_SOURCEKITD_REQUESTRESPONSEPRINTERBASE_H

#include "sourcekitd/sourcekitd.h"
#include "sourcekitd/Logging.h"
#include "language/Basic/StringExtras.h"
#include <vector>

namespace llvm {
  class StringRef;
  template <typename T> class ArrayRef;
  class raw_ostream;
}
namespace SourceKit {
  class UIdent;
}

namespace sourcekitd {

template <typename VisitorImplClass, typename VisitedType>
class RequestResponsePrinterBase {
  llvm::raw_ostream &OS;
  unsigned Indent;
  bool PrintAsJSON;
public:
  typedef std::vector<std::pair<SourceKit::UIdent, VisitedType>> DictMap;

  RequestResponsePrinterBase(llvm::raw_ostream &OS, unsigned Indent = 0, 
                             bool PrintAsJSON = false)
    : OS(OS), Indent(Indent), PrintAsJSON(PrintAsJSON) { }

  void visitNull() {
    OS << "<<NULL>>";
  }

  void visitDictionary(const DictMap &Map) {
    OS << "{\n";
    Indent += 2;
    for (unsigned i = 0, e = Map.size(); i != e; ++i) {
      auto &Pair = Map[i];
      OS.indent(Indent);
      if (PrintAsJSON) {
        visitString(Pair.first.getName());
      } else {
        OSColor(OS, DictKeyColor) << Pair.first.getName();
      }
      OS << ": ";
      static_cast<VisitorImplClass *>(this)->visit(Pair.second);
      if (i < e-1)
        OS << ',';
      OS << '\n';
    }
    Indent -= 2;
    OS.indent(Indent) << '}';
  }

  void visitArray(llvm::ArrayRef<VisitedType> Arr) {
    OS << "[\n";
    Indent += 2;
    for (unsigned i = 0, e = Arr.size(); i != e; ++i) {
      auto Obj = Arr[i];
      OS.indent(Indent);
      static_cast<VisitorImplClass *>(this)->visit(Obj);
      if (i < e-1)
        OS << ',';
      OS << '\n';
    }
    Indent -= 2;
    OS.indent(Indent) << ']';
  }

  void visitInt64(int64_t Val) {
    OS << Val;
  }

  void visitBool(bool Val) {
    OS << Val;
  }

  void visitDouble(double Val) { OS << Val; }

  void visitString(llvm::StringRef Str) {
    OS << '\"';
    // Avoid raw_ostream's write_escaped, we don't want to escape unicode
    // characters because it will be invalid JSON.
    swift::writeEscaped(Str, OS);
    OS << '\"';
  }

  void visitUID(llvm::StringRef UID) {
    if (PrintAsJSON) {
      visitString(UID);
    } else {
      OSColor(OS, UIDColor) << UID;
    }
  }

  void visitData(const void *Data, size_t Size) {
    // FIXME: We should probably print the real data here
    OS << "<data>";
  }
};

}

#endif
