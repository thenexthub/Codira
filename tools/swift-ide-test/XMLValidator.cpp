//===--- XMLValidator.cpp - XML validation --------------------------------===//
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

#include "XMLValidator.h"

#ifdef SWIFT_HAVE_LIBXML

// libxml headers use their own variant of documentation comments that Clang
// does not understand well.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <libxml/xmlerror.h>

#pragma clang diagnostic pop

using namespace language;

struct XMLValidator::Implementation {
  std::string SchemaFileName;
  xmlRelaxNGParserCtxtPtr RNGParser;
  xmlRelaxNGPtr Schema;
};

XMLValidator::XMLValidator() : Impl(new Implementation()) {}

XMLValidator::~XMLValidator() { delete Impl; }

void XMLValidator::setSchema(StringRef FileName) {
  assert(Impl->SchemaFileName.empty());
  Impl->SchemaFileName = FileName.str();
}

XMLValidator::Status XMLValidator::validate(const std::string &XML) {
  if (Impl->SchemaFileName.empty())
    return Status{ErrorCode::NoSchema, ""};

  if (!Impl->RNGParser) {
    Impl->RNGParser = xmlRelaxNGNewParserCtxt(Impl->SchemaFileName.c_str());
    Impl->Schema = xmlRelaxNGParse(Impl->RNGParser);
  }
  if (!Impl->RNGParser)
    return Status{ErrorCode::InternalError, ""};

  if (!Impl->Schema)
    return Status{ErrorCode::BadSchema, ""};

  xmlDocPtr Doc = xmlParseDoc(reinterpret_cast<const xmlChar *>(XML.data()));
  if (!Doc)
    return Status{ErrorCode::NotWellFormed, xmlGetLastError()->message};

  xmlRelaxNGValidCtxtPtr ValidationCtxt = xmlRelaxNGNewValidCtxt(Impl->Schema);
  int ValidationStatus = xmlRelaxNGValidateDoc(ValidationCtxt, Doc);
  Status Result;
  if (ValidationStatus == 0) {
    Result = Status{ErrorCode::Valid, ""};
  } else if (ValidationStatus > 0) {
    Result = Status{ErrorCode::NotValid, xmlGetLastError()->message};
  } else
    Result = Status{ErrorCode::InternalError, ""};

  xmlRelaxNGFreeValidCtxt(ValidationCtxt);
  xmlFreeDoc(Doc);

  return Result;
}

#else // !SWIFT_HAVE_LIBXML

using namespace language;

struct XMLValidator::Implementation {};

XMLValidator::XMLValidator() : Impl(new Implementation()) {}

XMLValidator::~XMLValidator() { delete Impl; }

void XMLValidator::setSchema(StringRef FileName) {}

XMLValidator::Status XMLValidator::validate(const std::string &XML) {
  return Status{ErrorCode::NotCompiledIn, ""};
}

#endif // SWIFT_HAVE_LIBXML

