#ifndef SWIFT_SEMA_TYPECHECKBITWISE_H
#define SWIFT_SEMA_TYPECHECKBITWISE_H

#include "language/AST/ProtocolConformance.h"
#include "language/AST/TypeCheckRequests.h"

namespace language {
class ProtocolConformance;
class NominalTypeDecl;

/// Check that \p conformance, a conformance of some nominal type to
/// BitwiseCopyable, is valid.
bool checkBitwiseCopyableConformance(ProtocolConformance *conformance,
                                     bool isImplicit);

/// Produce an implicit conformance of \p nominal to BitwiseCopyable if it is
/// valid to do so.
ProtocolConformance *
deriveImplicitBitwiseCopyableConformance(NominalTypeDecl *nominal);
} // end namespace language

#endif // SWIFT_SEMA_TYPECHECKBITWISE_H
