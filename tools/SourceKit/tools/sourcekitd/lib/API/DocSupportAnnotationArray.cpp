//===--- DocSupportAnnotationArray.cpp ------------------------------------===//
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

#include "sourcekitd/DocSupportAnnotationArray.h"
#include "sourcekitd/DictionaryKeys.h"
#include "sourcekitd/CompactArray.h"
#include "SourceKit/Core/LangSupport.h"
#include "toolchain/Support/MemoryBuffer.h"


using namespace SourceKit;
using namespace sourcekitd;

struct DocSupportAnnotationArrayBuilder::Implementation {
  CompactArrayBuilder<UIdent, std::optional<StringRef>,
                      std::optional<StringRef>, unsigned, unsigned>
      Builder;
};

DocSupportAnnotationArrayBuilder::DocSupportAnnotationArrayBuilder()
  : Impl(*new Implementation()) {

}

DocSupportAnnotationArrayBuilder::~DocSupportAnnotationArrayBuilder() {
  delete &Impl;
}

void DocSupportAnnotationArrayBuilder::add(const DocEntityInfo &Info) {
  std::optional<StringRef> NameOpt;
  if (!Info.Name.empty())
    NameOpt = Info.Name;
  std::optional<StringRef> USROpt;
  if (!Info.USR.empty())
    USROpt = Info.USR;
  Impl.Builder.addEntry(Info.Kind,
                        NameOpt,
                        USROpt,
                        Info.Offset,
                        Info.Length);
}

std::unique_ptr<toolchain::MemoryBuffer>
DocSupportAnnotationArrayBuilder::createBuffer() {
  return Impl.Builder.createBuffer(CustomBufferKind::DocSupportAnnotationArray);
}

namespace {

class DocSupportAnnotationArray {
public:
  typedef CompactArrayReader<sourcekitd_uid_t,
                             const char *,
                             const char *,
                             unsigned,
                             unsigned> CompactArrayReaderTy;

  static bool
  dictionary_apply(void *Buf, size_t Index,
                   sourcekitd_variant_dictionary_applier_f_t applier,
                   void *context) {
    CompactArrayReaderTy Reader(Buf);

    sourcekitd_uid_t Kind;
    const char *Name;
    const char *USR;
    unsigned Offset;
    unsigned Length;

    Reader.readEntries(Index,
                  Kind,
                  Name,
                  USR,
                  Offset,
                  Length);

#define APPLY(K, Ty, Field)                                                    \
  do {                                                                         \
    sourcekitd_uid_t key = SKDUIDFromUIdent(K);                                \
    sourcekitd_variant_t var = make##Ty##Variant(Field);                       \
    if (!applier(key, var, context))                                           \
      return false;                                                            \
  } while (0)

    APPLY(KeyKind, UID, Kind);
    if (Name) {
      APPLY(KeyName, String, Name);
    }
    if (USR) {
      APPLY(KeyUSR, String, USR);
    }
    APPLY(KeyOffset, Int, Offset);
    APPLY(KeyLength, Int, Length);

    return true;
  }
};

} // end anonymous namespace

VariantFunctions *
sourcekitd::getVariantFunctionsForDocSupportAnnotationArray() {
  return &CompactArrayFuncs<DocSupportAnnotationArray>::Funcs;
}
