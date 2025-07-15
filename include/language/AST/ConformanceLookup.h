//===--- ConformanceLookup.h - Global conformance lookup --------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_CONFORMANCELOOKUP_H
#define LANGUAGE_AST_CONFORMANCELOOKUP_H

#include "toolchain/ADT/ArrayRef.h"
#include <optional>

namespace language {

class CanType;
class Type;
class ProtocolConformanceRef;
class ProtocolDecl;

/// Global conformance lookup, does not check conditional requirements.
///
/// \param type The type for which we are computing conformance.
///
/// \param protocol The protocol to which we are computing conformance.
///
/// \param allowMissing When \c true, the resulting conformance reference
/// might include "missing" conformances, which are synthesized for some
/// protocols as an error recovery mechanism.
///
/// \returns An invalid conformance if the search failed, otherwise an
/// abstract, concrete or pack conformance, depending on the lookup type.
ProtocolConformanceRef lookupConformance(Type type, ProtocolDecl *protocol,
                                         bool allowMissing = false);

/// Global conformance lookup, checks conditional requirements.
/// Requires a contextualized type.
///
/// \param type The type for which we are computing conformance. Must not
/// contain type parameters.
///
/// \param protocol The protocol to which we are computing conformance.
///
/// \param allowMissing When \c true, the resulting conformance reference
/// might include "missing" conformances, which are synthesized for some
/// protocols as an error recovery mechanism.
///
/// \returns An invalid conformance if the search failed, otherwise an
/// abstract, concrete or pack conformance, depending on the lookup type.
ProtocolConformanceRef checkConformance(Type type, ProtocolDecl *protocol,
                                        // Note: different default from
                                        // lookupConformance
                                        bool allowMissing = true);

/// Global conformance lookup, checks conditional requirements.
/// Accepts interface types without context. If the conformance cannot be
/// definitively established without the missing context, returns \c nullopt.
///
/// \param type The type for which we are computing conformance. Must not
/// contain type parameters.
///
/// \param protocol The protocol to which we are computing conformance.
///
/// \param allowMissing When \c true, the resulting conformance reference
/// might include "missing" conformances, which are synthesized for some
/// protocols as an error recovery mechanism.
///
/// \returns An invalid conformance if the search definitively failed. An
/// abstract, concrete or pack conformance, depending on the lookup type,
/// if the search succeeded. `std::nullopt` if the type could have
/// conditionally conformed depending on the context of the interface types.
std::optional<ProtocolConformanceRef>
checkConformanceWithoutContext(Type type,
                               ProtocolDecl *protocol,
                               // Note: different default from
                               // lookupConformance
                               bool allowMissing = true);


/// Look for the conformance of the given existential type to the given
/// protocol.
ProtocolConformanceRef lookupExistentialConformance(Type type,
                                                    ProtocolDecl *protocol);

/// Collect the conformances of \c fromType to each of the protocols of an
/// existential type's layout.
toolchain::ArrayRef<ProtocolConformanceRef>
collectExistentialConformances(CanType fromType, CanType existential,
                               bool allowMissing = false);

}

#endif // LANGUAGE_AST_CONFORMANCELOOKUP_H