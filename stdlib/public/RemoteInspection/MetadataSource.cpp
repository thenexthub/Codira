//===--- MetadataSource.cpp - Codira Metadata Sources for Reflection -------===//
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

#if LANGUAGE_ENABLE_REFLECTION

#include "language/RemoteInspection/MetadataSource.h"
#include <iostream>

using namespace language;
using namespace reflection;

class PrintMetadataSource
  : public MetadataSourceVisitor<PrintMetadataSource, void> {
  std::ostream &stream;
  unsigned Indent;

  std::ostream &indent(unsigned Amount) {
    for (unsigned i = 0; i < Amount; ++i)
      stream << " ";
    return stream;
  }

  std::ostream &printHeader(std::string Name) {
    indent(Indent) << "(" << Name;
    return stream;
  }

  std::ostream &printField(std::string name, std::string value) {
    if (!name.empty())
      stream << " " << name << "=" << value;
    else
      stream << " " << value;
    return stream;
  }

  void printRec(const MetadataSource *MS) {
    stream << "\n";

    Indent += 2;
    visit(MS);
    Indent -= 2;
  }

  void closeForm() {
    stream << ")";
  }

public:
  PrintMetadataSource(std::ostream &stream, unsigned Indent)
      : stream(stream), Indent(Indent) {}

  void
  visitClosureBindingMetadataSource(const ClosureBindingMetadataSource *CB) {
    printHeader("closure_binding");
    printField("index", std::to_string(CB->getIndex()));
    closeForm();
  }

  void
  visitReferenceCaptureMetadataSource(const ReferenceCaptureMetadataSource *RC){
    printHeader("reference_capture");
    printField("index", std::to_string(RC->getIndex()));
    closeForm();
  }

  void
  visitMetadataCaptureMetadataSource(const MetadataCaptureMetadataSource *MC){
    printHeader("metadata_capture");
    printField("index", std::to_string(MC->getIndex()));
    closeForm();
  }

  void
  visitGenericArgumentMetadataSource(const GenericArgumentMetadataSource *GA) {
    printHeader("generic_argument");
    printField("index", std::to_string(GA->getIndex()));
    printRec(GA->getSource());
    closeForm();
  }

  void visitSelfMetadataSource(const SelfMetadataSource *S) {
    printHeader("self");
    closeForm();
  }

  void
  visitSelfWitnessTableMetadataSource(const SelfWitnessTableMetadataSource *W) {
    printHeader("self_witness_table");
    closeForm();
  }
};

void MetadataSource::dump() const { dump(std::cerr, 0); }

void MetadataSource::dump(std::ostream &stream, unsigned Indent) const {
  PrintMetadataSource(stream, Indent).visit(this);
  stream << "\n";
}

#endif
