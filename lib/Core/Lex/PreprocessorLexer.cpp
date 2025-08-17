//===- PreprocessorLexer.cpp - C Language Family Lexer --------------------===//
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
//  This file implements the PreprocessorLexer and Token interfaces.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Lex/PreprocessorLexer.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Lex/Token.h"
#include <cassert>

using namespace language::Core;

void PreprocessorLexer::anchor() {}

PreprocessorLexer::PreprocessorLexer(Preprocessor *pp, FileID fid)
    : PP(pp), FID(fid) {
  if (pp)
    InitialNumSLocEntries = pp->getSourceManager().local_sloc_entry_size();
}

/// After the preprocessor has parsed a \#include, lex and
/// (potentially) macro expand the filename.
void PreprocessorLexer::LexIncludeFilename(Token &FilenameTok) {
  assert(ParsingFilename == false && "reentered LexIncludeFilename");

  // We are now parsing a filename!
  ParsingFilename = true;

  // Lex the filename.
  if (LexingRawMode)
    IndirectLex(FilenameTok);
  else
    PP->Lex(FilenameTok);

  // We should have obtained the filename now.
  ParsingFilename = false;
}

/// getFileEntry - Return the FileEntry corresponding to this FileID.  Like
/// getFileID(), this only works for lexers with attached preprocessors.
OptionalFileEntryRef PreprocessorLexer::getFileEntry() const {
  return PP->getSourceManager().getFileEntryRefForID(getFileID());
}
