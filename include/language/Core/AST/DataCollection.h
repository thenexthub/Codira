//===--- DatatCollection.h --------------------------------------*- C++ -*-===//
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
/// \file
/// This file declares helper methods for collecting data from AST nodes.
///
/// To collect data from Stmt nodes, subclass ConstStmtVisitor and include
/// StmtDataCollectors.inc after defining the macros that you need. This
/// provides data collection implementations for most Stmt kinds. Note
/// that the code requires some conditions to be met:
///
///   - There must be a method addData(const T &Data) that accepts strings,
///     integral types as well as QualType. All data is forwarded using
///     to this method.
///   - The ASTContext of the Stmt must be accessible by the name Context.
///
/// It is also possible to override individual visit methods. Have a look at
/// the DataCollector in lib/Analysis/CloneDetection.cpp for a usage example.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_DATACOLLECTION_H
#define LANGUAGE_CORE_AST_DATACOLLECTION_H

#include "language/Core/AST/ASTContext.h"

namespace language::Core {
namespace data_collection {

/// Returns a string that represents all macro expansions that expanded into the
/// given SourceLocation.
///
/// If 'getMacroStack(A) == getMacroStack(B)' is true, then the SourceLocations
/// A and B are expanded from the same macros in the same order.
std::string getMacroStack(SourceLocation Loc, ASTContext &Context);

/// Utility functions for implementing addData() for a consumer that has a
/// method update(StringRef)
template <class T>
void addDataToConsumer(T &DataConsumer, toolchain::StringRef Str) {
  DataConsumer.update(Str);
}

template <class T> void addDataToConsumer(T &DataConsumer, const QualType &QT) {
  addDataToConsumer(DataConsumer, QT.getAsString());
}

template <class T, class Type>
std::enable_if_t<std::is_integral<Type>::value || std::is_enum<Type>::value ||
                 std::is_convertible<Type, size_t>::value // for toolchain::hash_code
                 >
addDataToConsumer(T &DataConsumer, Type Data) {
  DataConsumer.update(StringRef(reinterpret_cast<char *>(&Data), sizeof(Data)));
}

} // end namespace data_collection
} // end namespace language::Core

#endif // LANGUAGE_CORE_AST_DATACOLLECTION_H
