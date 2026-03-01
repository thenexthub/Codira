//===-- UniqueDWARFASTType.cpp --------------------------------------------===//
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

#include "UniqueDWARFASTType.h"
#include "SymbolFileDWARF.h"

#include "lldb/Core/Declaration.h"
#include "lldb/Target/Language.h"

using namespace lldb_private::plugin::dwarf;
using namespace llvm::dwarf;

static bool IsStructOrClassTag(llvm::dwarf::Tag Tag) {
  return Tag == llvm::dwarf::Tag::DW_TAG_class_type ||
         Tag == llvm::dwarf::Tag::DW_TAG_structure_type;
}

static bool IsSizeAndDeclarationMatching(UniqueDWARFASTType const &udt,
                                         DWARFDIE const &die,
                                         const lldb_private::Declaration &decl,
                                         const int32_t byte_size,
                                         bool is_forward_declaration) {

  // If they are not both definition DIEs or both declaration DIEs, then
  // don't check for byte size and declaration location, because declaration
  // DIEs usually don't have those info.
  if (udt.m_is_forward_declaration != is_forward_declaration)
    return true;

  if (udt.m_byte_size > 0 && byte_size > 0 && udt.m_byte_size != byte_size)
    return false;

  // For C++, we match the behaviour of
  // DWARFASTParserClang::GetUniqueTypeNameAndDeclaration. We rely on the
  // one-definition-rule: for a given fully qualified name there exists only one
  // definition, and there should only be one entry for such name, so ignore
  // location of where it was declared vs. defined.
  if (lldb_private::Language::LanguageIsCPlusPlus(
          SymbolFileDWARF::GetLanguage(*die.GetCU())))
    return true;

  return udt.m_declaration == decl;
}

UniqueDWARFASTType *UniqueDWARFASTTypeList::Find(
    const DWARFDIE &die, const lldb_private::Declaration &decl,
    const int32_t byte_size, bool is_forward_declaration) {
  for (UniqueDWARFASTType &udt : m_collection) {
    // Make sure the tags match
    if (udt.m_die.Tag() == die.Tag() || (IsStructOrClassTag(udt.m_die.Tag()) &&
                                         IsStructOrClassTag(die.Tag()))) {

      if (!IsSizeAndDeclarationMatching(udt, die, decl, byte_size,
                                        is_forward_declaration))
        continue;

      // The type has the same name, and was defined on the same file and
      // line. Now verify all of the parent DIEs match.
      DWARFDIE parent_arg_die = die.GetParent();
      DWARFDIE parent_pos_die = udt.m_die.GetParent();
      bool match = true;
      bool done = false;
      while (!done && match && parent_arg_die && parent_pos_die) {
        const dw_tag_t parent_arg_tag = parent_arg_die.Tag();
        const dw_tag_t parent_pos_tag = parent_pos_die.Tag();
        if (parent_arg_tag == parent_pos_tag ||
            (IsStructOrClassTag(parent_arg_tag) &&
             IsStructOrClassTag(parent_pos_tag))) {
          switch (parent_arg_tag) {
          case DW_TAG_class_type:
          case DW_TAG_structure_type:
          case DW_TAG_union_type:
          case DW_TAG_namespace: {
            const char *parent_arg_die_name = parent_arg_die.GetName();
            if (parent_arg_die_name == nullptr) {
              // Anonymous (i.e. no-name) struct
              match = false;
            } else {
              const char *parent_pos_die_name = parent_pos_die.GetName();
              if (parent_pos_die_name == nullptr ||
                  ((parent_arg_die_name != parent_pos_die_name) &&
                   strcmp(parent_arg_die_name, parent_pos_die_name)))
                match = false;
            }
          } break;

          case DW_TAG_compile_unit:
          case DW_TAG_partial_unit:
            done = true;
            break;
          default:
            break;
          }
        }
        parent_arg_die = parent_arg_die.GetParent();
        parent_pos_die = parent_pos_die.GetParent();
      }

      if (match) {
        return &udt;
      }
    }
  }
  return nullptr;
}
