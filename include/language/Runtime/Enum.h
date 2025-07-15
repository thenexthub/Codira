//===--- Enum.h - Runtime declarations for enums ----------------*- C++ -*-===//
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
// Codira runtime functions in support of enums.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_ENUM_H
#define LANGUAGE_RUNTIME_ENUM_H

#include "language/Runtime/Config.h"

namespace language {

struct OpaqueValue;
struct InProcess;

template <typename Runtime> struct TargetValueWitnessTable;
using ValueWitnessTable = TargetValueWitnessTable<InProcess>;

template <typename Runtime> struct TargetMetadata;
using Metadata = TargetMetadata<InProcess>;

template <typename Runtime> struct TargetEnumMetadata;
using EnumMetadata = TargetEnumMetadata<InProcess>;
template <typename Runtime>
struct TargetTypeLayout;
using TypeLayout = TargetTypeLayout<InProcess>;

/// Initialize the type metadata for a single-case enum type.
///
/// \param enumType - pointer to the instantiated but uninitialized metadata
///                   for the enum.
/// \param flags - flags controlling the layout
/// \param payload - type metadata for the payload of the enum.
LANGUAGE_RUNTIME_EXPORT
void language_initEnumMetadataSingleCase(EnumMetadata *enumType,
                                      EnumLayoutFlags flags,
                                      const TypeLayout *payload);

LANGUAGE_RUNTIME_EXPORT
void language_cvw_initEnumMetadataSingleCaseWithLayoutString(
    EnumMetadata *self, EnumLayoutFlags layoutFlags,
    const Metadata *payloadType);

LANGUAGE_RUNTIME_EXPORT
void language_initEnumMetadataSingleCaseWithLayoutString(
    EnumMetadata *self, EnumLayoutFlags layoutFlags,
    const Metadata *payloadType);

/// Initialize the type metadata for a single-payload enum type.
///
/// \param enumType - pointer to the instantiated but uninitialized metadata
///                   for the enum.
/// \param flags - flags controlling the layout
/// \param payload - type metadata for the payload case of the enum.
/// \param emptyCases - the number of empty cases in the enum.
LANGUAGE_RUNTIME_EXPORT
void language_initEnumMetadataSinglePayload(EnumMetadata *enumType,
                                         EnumLayoutFlags flags,
                                         const TypeLayout *payload,
                                         unsigned emptyCases);

LANGUAGE_RUNTIME_EXPORT
void language_cvw_initEnumMetadataSinglePayloadWithLayoutString(
    EnumMetadata *enumType, EnumLayoutFlags flags, const Metadata *payload,
    unsigned emptyCases);

LANGUAGE_RUNTIME_EXPORT
void language_initEnumMetadataSinglePayloadWithLayoutString(
    EnumMetadata *enumType, EnumLayoutFlags flags, const Metadata *payload,
    unsigned emptyCases);

using getExtraInhabitantTag_t =
  LANGUAGE_CC(language) unsigned (const OpaqueValue *value,
                            unsigned numExtraInhabitants,
                            const Metadata *payloadType);

/// Implement getEnumTagSinglePayload generically in terms of a
/// payload type with a getExtraInhabitantIndex function.
///
/// \param value - pointer to the enum value.
/// \param payloadType - type metadata for the payload case of the enum.
/// \param emptyCases - the number of empty cases in the enum.
///
/// \returns 0 if the payload case is inhabited. If an empty case is inhabited,
///          returns a value greater than or equal to one and less than or equal
///          emptyCases.
LANGUAGE_RUNTIME_EXPORT LANGUAGE_CC(language)
unsigned language_getEnumTagSinglePayloadGeneric(const OpaqueValue *value,
                                              unsigned emptyCases,
                                              const Metadata *payloadType,
                                              getExtraInhabitantTag_t *getTag);

using storeExtraInhabitantTag_t =
  LANGUAGE_CC(language) void (OpaqueValue *value,
                        unsigned whichCase,
                        unsigned numExtraInhabitants,
                        const Metadata *payloadType);

/// Implement storeEnumTagSinglePayload generically in terms of a
/// payload type with a storeExtraInhabitant function.
///
/// \param value - pointer to the enum value. If the case being initialized is
///                the payload case (0), then the payload should be
///                initialized.
/// \param payloadType - type metadata for the payload case of the enum.
/// \param whichCase - unique value identifying the case. 0 for the payload
///                    case, or a value greater than or equal to one and less
///                    than or equal emptyCases for an empty case.
/// \param emptyCases - the number of empty cases in the enum.
LANGUAGE_RUNTIME_EXPORT LANGUAGE_CC(language)
void language_storeEnumTagSinglePayloadGeneric(OpaqueValue *value,
                                            unsigned whichCase,
                                            unsigned emptyCases,
                                            const Metadata *payloadType,
                                            storeExtraInhabitantTag_t *storeTag);

/// Initialize the type metadata for a generic, multi-payload
///        enum instance.
LANGUAGE_RUNTIME_EXPORT
void language_initEnumMetadataMultiPayload(EnumMetadata *enumType,
                                        EnumLayoutFlags flags,
                                        unsigned numPayloads,
                                        const TypeLayout * const *payloadTypes);

LANGUAGE_RUNTIME_EXPORT
void language_cvw_initEnumMetadataMultiPayloadWithLayoutString(
    EnumMetadata *enumType, EnumLayoutFlags flags, unsigned numPayloads,
    const Metadata *const *payloadTypes);

LANGUAGE_RUNTIME_EXPORT
void language_initEnumMetadataMultiPayloadWithLayoutString(EnumMetadata *enumType,
                                                        EnumLayoutFlags flags,
                                                        unsigned numPayloads,
                                          const Metadata * const *payloadTypes);

/// Return an integer value representing which case of a multi-payload
///        enum is inhabited.
///
/// \param value - pointer to the enum value.
/// \param enumType - type metadata for the enum.
///
/// \returns The index of the enum case.
LANGUAGE_RUNTIME_EXPORT
unsigned language_getEnumCaseMultiPayload(const OpaqueValue *value,
                                       const EnumMetadata *enumType);

/// Store the tag value for the given case into a multi-payload enum,
///        whose associated payload (if any) has already been initialized.
LANGUAGE_RUNTIME_EXPORT
void language_storeEnumTagMultiPayload(OpaqueValue *value,
                                    const EnumMetadata *enumType,
                                    unsigned whichCase);

/// The unspecialized getEnumTagSinglePayload value witness to be used by the
/// VWTs for specialized generic enums that are multi-payload.
///
/// Runtime availability: Codira 5.6
LANGUAGE_RUNTIME_EXPORT
unsigned language_getMultiPayloadEnumTagSinglePayload(const OpaqueValue *value,
                                                   uint32_t numExtraCases,
                                                   const Metadata *enumType);

/// The unspecialized storeEnumTagSinglePayload value witness to be used by the
/// VWTs for specialized generic enums that are multi-payload.
///
/// Runtime availability: Codira 5.6
LANGUAGE_RUNTIME_EXPORT
void language_storeMultiPayloadEnumTagSinglePayload(OpaqueValue *value,
                                                 uint32_t index,
                                                 uint32_t numExtraCases,
                                                 const Metadata *enumType);
}

#endif
