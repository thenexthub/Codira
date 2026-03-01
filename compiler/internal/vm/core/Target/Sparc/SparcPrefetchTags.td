//===- SparcPrefetchTags.td --------------------------------*- tablegen -*-===//
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
// This file defines the symbolic operands permitted for various kinds of
// SPARCv9 prefetches.
//
//===----------------------------------------------------------------------===//

class PrefetchTag<string name, bits<8> op> {
  string Name = name;
  bits<8> Encoding = op;
}

def PrefetchTagsList : GenericTable {
  let FilterClass = "PrefetchTag";
  let Fields = ["Name", "Encoding"];

  let PrimaryKey = [ "Encoding" ];
  let PrimaryKeyName = "lookupPrefetchTagByEncoding";
}

def lookupPrefetchTagByName : SearchIndex {
  let Table = PrefetchTagsList;
  let Key = [ "Name" ];
}

def : PrefetchTag<"n_reads", 0x0>;
def : PrefetchTag<"one_read", 0x1>;
def : PrefetchTag<"n_writes", 0x2>;
def : PrefetchTag<"one_write", 0x3>;
def : PrefetchTag<"page", 0x4>;
def : PrefetchTag<"unified", 0x11>;
def : PrefetchTag<"n_reads_strong", 0x14>;
def : PrefetchTag<"one_read_strong", 0x15>;
def : PrefetchTag<"n_writes_strong", 0x16>;
def : PrefetchTag<"one_write_strong", 0x17>;
