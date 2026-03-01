//===-- DILAST.cpp --------------------------------------------------------===//
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

#include "lldb/ValueObject/DILAST.h"
#include "llvm/Support/ErrorHandling.h"

namespace lldb_private::dil {

llvm::Expected<lldb::ValueObjectSP> ErrorNode::Accept(Visitor *v) const {
  llvm_unreachable("Attempting to Visit a DIL ErrorNode.");
}

llvm::Expected<lldb::ValueObjectSP> IdentifierNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP> MemberOfNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP> UnaryOpNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP>
ArraySubscriptNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP>
BitFieldExtractionNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP>
IntegerLiteralNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP> FloatLiteralNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP>
BooleanLiteralNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

llvm::Expected<lldb::ValueObjectSP> CastNode::Accept(Visitor *v) const {
  return v->Visit(*this);
}

} // namespace lldb_private::dil
