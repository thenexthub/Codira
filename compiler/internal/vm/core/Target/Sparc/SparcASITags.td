//===- SparcASITags.td -------------------------------------*- tablegen -*-===//
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
// SPARCv9 ASI.
//
//===----------------------------------------------------------------------===//

class ASITag<string name, string alt_name, bits<8> op> {
  string Name = name;
  // A maximum of one alias is supported right now.
  string AltName = alt_name;
  bits<8> Encoding = op;
}

def ASITagsList : GenericTable {
  let FilterClass = "ASITag";
  let Fields = ["Name", "AltName", "Encoding"];

  let PrimaryKey = [ "Encoding" ];
  let PrimaryKeyName = "lookupASITagByEncoding";
}

def lookupASITagByName : SearchIndex {
  let Table = ASITagsList;
  let Key = [ "Name" ];
}

def lookupASITagByAltName : SearchIndex {
  let Table = ASITagsList;
  let Key = [ "AltName" ];
}

def : ASITag<"ASI_N", "ASI_NUCLEUS", 0x4>;
def : ASITag<"ASI_N_L", "ASI_NUCLEUS_LITTLE", 0xC>;
def : ASITag<"ASI_AIUP", "ASI_AS_IF_USER_PRIMARY", 0x10>;
def : ASITag<"ASI_AIUS", "ASI_AS_IF_USER_SECONDARY", 0x11>;
def : ASITag<"ASI_AIUP_L", "ASI_AS_IF_USER_PRIMARY_LITTLE", 0x18>;
def : ASITag<"ASI_AIUS_L", "ASI_AS_IF_USER_SECONDARY_LITTLE", 0x19>;
def : ASITag<"ASI_P", "ASI_PRIMARY", 0x80>;
def : ASITag<"ASI_S", "ASI_SECONDARY", 0x81>;
def : ASITag<"ASI_PNF", "ASI_PRIMARY_NOFAULT", 0x82>;
def : ASITag<"ASI_SNF", "ASI_SECONDARY_NOFAULT", 0x83>;
def : ASITag<"ASI_P_L", "ASI_PRIMARY_LITTLE", 0x88>;
def : ASITag<"ASI_S_L", "ASI_SECONDARY_LITTLE", 0x89>;
def : ASITag<"ASI_PNF_L", "ASI_PRIMARY_NOFAULT_LITTLE", 0x8A>;
def : ASITag<"ASI_SNF_L", "ASI_SECONDARY_NOFAULT_LITTLE", 0x8B>;
