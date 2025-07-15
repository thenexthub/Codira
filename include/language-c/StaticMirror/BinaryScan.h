//===--- BinaryScan.h - C API for Codira Binary Scanning ---*- C -*-===//
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
// This C API is primarily intended to serve as a "static mirror" library
// for querying Codira type information from binary object files.
//
//===----------------------------------------------------------------------===//

#include "StaticMirrorMacros.h"
#include "language-c/CommonString/CommonString.h"

#ifndef LANGUAGE_C_BINARY_SCAN_H
#define LANGUAGE_C_BINARY_SCAN_H

/// The version constants for the CodiraStaticMirror C API.
/// LANGUAGESTATICMIRROR_VERSION_MINOR should increase when there are API additions.
/// LANGUAGESTATICMIRROR_VERSION_MAJOR is intended for "major" source/ABI breaking changes.
#define LANGUAGESTATICMIRROR_VERSION_MAJOR 0
#define LANGUAGESTATICMIRROR_VERSION_MINOR 5 // Added opaque associated type's same-type requirements

LANGUAGESTATICMIRROR_BEGIN_DECLS

//=== Public Binary Scanner Data Types ------------------------------------===//

typedef languagescan_string_ref_t language_static_mirror_string_ref_t;
typedef languagescan_string_set_t language_static_mirror_string_set_t;

/// Container of the configuration state for binary static mirror scanning
/// instance
typedef void *language_static_mirror_t;

/// Opaque container to a conformance type info of a given protocol conformance.
typedef struct language_static_mirror_conformance_info_s
    *language_static_mirror_conformance_info_t;

/// Opaque container to a field info (property type or enum case)
typedef struct language_static_mirror_field_info_s
    *language_static_mirror_field_info_t;

/// Opaque container to a property type info
typedef struct language_static_mirror_property_info_s
    *language_static_mirror_property_info_t;

/// Opaque container to an enum case info
typedef struct language_static_mirror_enum_case_info_s
    *language_static_mirror_enum_case_info_t;

/// Opaque container to details of an associated type specification.
typedef struct language_static_mirror_type_alias_s
    *language_static_mirror_type_alias_t;

/// Opaque container to an associated type (typealias) info of a given type.
typedef struct language_static_mirror_associated_type_info_s
    *language_static_mirror_associated_type_info_t;

typedef struct {
  language_static_mirror_type_alias_t *type_aliases;
  size_t count;
} language_static_mirror_type_alias_set_t;

typedef struct {
  language_static_mirror_associated_type_info_t *associated_type_infos;
  size_t count;
} language_static_mirror_associated_type_info_set_t;

typedef struct {
  language_static_mirror_conformance_info_t *conformances;
  size_t count;
} language_static_mirror_conformances_set_t;

typedef struct {
  language_static_mirror_property_info_t *properties;
  size_t count;
} language_static_mirror_property_info_set_t;

typedef struct {
  language_static_mirror_enum_case_info_t *enum_cases;
  size_t count;
} language_static_mirror_enum_case_info_set_t;

typedef struct {
  language_static_mirror_field_info_t *field_infos;
  size_t count;
} language_static_mirror_field_info_set_t;

// language_static_mirror_conformance_info query methods
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_conformance_info_get_type_name(
        language_static_mirror_conformance_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_conformance_info_get_protocol_name(
        language_static_mirror_conformance_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_conformance_info_get_mangled_type_name(
        language_static_mirror_conformance_info_t);
LANGUAGESTATICMIRROR_PUBLIC void language_static_mirror_conformance_info_dispose(
    language_static_mirror_conformance_info_t);

// language_static_mirror_associated_type_info query methods
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_type_alias_get_type_alias_name(
        language_static_mirror_type_alias_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_type_alias_get_substituted_type_name(
        language_static_mirror_type_alias_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_type_alias_get_substituted_type_mangled_name(
        language_static_mirror_type_alias_t);
LANGUAGESTATICMIRROR_PUBLIC languagescan_string_set_t *
language_static_mirror_type_alias_get_opaque_type_protocol_requirements(
        language_static_mirror_type_alias_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_type_alias_set_t *
language_static_mirror_type_alias_get_opaque_type_same_type_requirements(
        language_static_mirror_type_alias_t);

// language_static_mirror_associated_type_info query methods
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_associated_type_info_get_mangled_type_name(
        language_static_mirror_associated_type_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_type_alias_set_t *
    language_static_mirror_associated_type_info_get_type_alias_set(
        language_static_mirror_associated_type_info_t);

// language_static_mirror_field_info query methods
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_field_info_get_mangled_type_name(
        language_static_mirror_field_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_property_info_set_t *
    language_static_mirror_field_info_get_property_info_set(
        language_static_mirror_field_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_enum_case_info_set_t *
    language_static_mirror_field_info_get_enum_case_info_set(
        language_static_mirror_field_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_property_info_get_label(
        language_static_mirror_property_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_property_info_get_type_name(
        language_static_mirror_property_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_property_info_get_mangled_type_name(
        language_static_mirror_property_info_t);
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_string_ref_t
    language_static_mirror_enum_case_info_get_label(
        language_static_mirror_enum_case_info_t);

/// Create an \c language_static_mirror_t instance.
/// The returned \c language_static_mirror_t is owned by the caller and must be
/// disposed of using \c language_static_mirror_dispose .
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_t
language_static_mirror_create(int, const char **, const char *);

LANGUAGESTATICMIRROR_PUBLIC void
    language_static_mirror_dispose(language_static_mirror_t);

/// Identify and collect all types conforming to any of the protocol names
/// specified as arguments
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_conformances_set_t *
language_static_mirror_conformances_set_create(language_static_mirror_t, int,
                                            const char **);

LANGUAGESTATICMIRROR_PUBLIC void language_static_mirror_conformances_set_dispose(
    language_static_mirror_conformances_set_t *);

/// Identify and collect all associated types of a given input type (by mangled
/// name)
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_associated_type_info_set_t *
language_static_mirror_associated_type_info_set_create(language_static_mirror_t,
                                                    const char *);

/// Identify and collect associated types of all discovered types.
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_associated_type_info_set_t *
    language_static_mirror_all_associated_type_info_set_create(
        language_static_mirror_t);

LANGUAGESTATICMIRROR_PUBLIC void
language_static_mirror_associated_type_info_set_dispose(
    language_static_mirror_associated_type_info_set_t *);

/// Identify and collect all field types of a given input type (by mangled name)
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_field_info_set_t *
language_static_mirror_field_info_set_create(language_static_mirror_t, const char *);

/// Identify and collect field types of all discovered types.
LANGUAGESTATICMIRROR_PUBLIC language_static_mirror_field_info_set_t *
    language_static_mirror_all_field_info_set_create(language_static_mirror_t);

LANGUAGESTATICMIRROR_PUBLIC void language_static_mirror_field_info_set_dispose(
    language_static_mirror_field_info_set_t *);

LANGUAGESTATICMIRROR_END_DECLS

#endif // LANGUAGE_C_BINARY_SCAN_H
