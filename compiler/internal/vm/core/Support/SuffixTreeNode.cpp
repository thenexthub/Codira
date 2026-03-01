//===- toolchain/ADT/SuffixTreeNode.cpp - Nodes for SuffixTrees --------*- C++
//-*-===//
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
//
// This file defines nodes for use within a SuffixTree.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/SuffixTreeNode.h"

using namespace vm::core;

unsigned SuffixTreeNode::getStartIdx() const { return StartIdx; }
void SuffixTreeNode::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
void SuffixTreeNode::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned SuffixTreeNode::getConcatLen() const { return ConcatLen; }

bool SuffixTreeInternalNode::isRoot() const {
  return getStartIdx() == EmptyIdx;
}
unsigned SuffixTreeInternalNode::getEndIdx() const { return EndIdx; }
void SuffixTreeInternalNode::setLink(SuffixTreeInternalNode *L) {
  assert(L && "Cannot set a null link?");
  Link = L;
}
SuffixTreeInternalNode *SuffixTreeInternalNode::getLink() const { return Link; }

unsigned SuffixTreeLeafNode::getEndIdx() const {
  assert(EndIdx && "EndIdx is empty?");
  return *EndIdx;
}

unsigned SuffixTreeLeafNode::getSuffixIdx() const { return SuffixIdx; }
void SuffixTreeLeafNode::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned SuffixTreeNode::getLeftLeafIdx() const { return LeftLeafIdx; }
unsigned SuffixTreeNode::getRightLeafIdx() const { return RightLeafIdx; }
void SuffixTreeNode::setLeftLeafIdx(unsigned Idx) { LeftLeafIdx = Idx; }
void SuffixTreeNode::setRightLeafIdx(unsigned Idx) { RightLeafIdx = Idx; }
