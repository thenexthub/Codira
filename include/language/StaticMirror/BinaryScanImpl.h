//===---------------- BinaryScanImpl.h - Swift Compiler ------------------===//
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
//
// Implementation details of the binary scanning C API
//
//===----------------------------------------------------------------------===//
#ifndef SWIFT_C_BINARY_SCAN_IMPL_H
#define SWIFT_C_BINARY_SCAN_IMPL_H

#include "language-c/StaticMirror/BinaryScan.h"

namespace language {
namespace static_mirror {
class BinaryScanningTool;
}
} // namespace language

struct swift_static_mirror_conformance_info_s {
  swift_static_mirror_string_ref_t type_name;
  swift_static_mirror_string_ref_t mangled_type_name;
  swift_static_mirror_string_ref_t protocol_name;
};

struct swift_static_mirror_type_alias_s {
  swift_static_mirror_string_ref_t type_alias_name;
  swift_static_mirror_string_ref_t substituted_type_name;
  swift_static_mirror_string_ref_t substituted_type_mangled_name;
  swift_static_mirror_string_set_t *opaque_protocol_requirements_set;
  swift_static_mirror_type_alias_set_t *opaque_same_type_requirements_set;
};

struct swift_static_mirror_associated_type_info_s {
  swift_static_mirror_string_ref_t mangled_type_name;
  swift_static_mirror_type_alias_set_t *type_alias_set;
};

struct swift_static_mirror_enum_case_info_s {
  swift_static_mirror_string_ref_t label;
};

struct swift_static_mirror_property_info_s {
  swift_static_mirror_string_ref_t label;
  swift_static_mirror_string_ref_t type_name;
  swift_static_mirror_string_ref_t mangled_type_name;
};

struct swift_static_mirror_field_info_s {
  swift_static_mirror_string_ref_t mangled_type_name;
  swift_static_mirror_property_info_set_t *property_set;
  swift_static_mirror_enum_case_info_set_t *enum_case_set;
};

#endif // SWIFT_C_BINARY_SCAN_IMPL_H
