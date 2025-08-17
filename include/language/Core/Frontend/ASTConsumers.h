//===--- ASTConsumers.h - ASTConsumer implementations -----------*- C++ -*-===//
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
// AST Consumers.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_FRONTEND_ASTCONSUMERS_H
#define LANGUAGE_CORE_FRONTEND_ASTCONSUMERS_H

#include "language/Core/AST/ASTDumperUtils.h"
#include "language/Core/Basic/LLVM.h"
#include <memory>

namespace language::Core {

class ASTConsumer;

// AST pretty-printer: prints out the AST in a format that is close to the
// original C code.  The output is intended to be in a format such that
// clang could re-parse the output back into the same AST, but the
// implementation is still incomplete.
std::unique_ptr<ASTConsumer> CreateASTPrinter(std::unique_ptr<raw_ostream> OS,
                                              StringRef FilterString);

// AST dumper: dumps the raw AST in human-readable form to the given output
// stream, or stdout if OS is nullptr.
std::unique_ptr<ASTConsumer>
CreateASTDumper(std::unique_ptr<raw_ostream> OS, StringRef FilterString,
                bool DumpDecls, bool Deserialize, bool DumpLookups,
                bool DumpDeclTypes, ASTDumpOutputFormat Format);

std::unique_ptr<ASTConsumer>
CreateASTDumper(raw_ostream &OS, StringRef FilterString, bool DumpDecls,
                bool Deserialize, bool DumpLookups, bool DumpDeclTypes,
                ASTDumpOutputFormat Format);

// AST Decl node lister: prints qualified names of all filterable AST Decl
// nodes.
std::unique_ptr<ASTConsumer> CreateASTDeclNodeLister();

// Graphical AST viewer: for each function definition, creates a graph of
// the AST and displays it with the graph viewer "dotty".  Also outputs
// function declarations to stderr.
std::unique_ptr<ASTConsumer> CreateASTViewer();

} // end clang namespace

#endif
