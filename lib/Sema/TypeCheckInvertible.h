#ifndef LANGUAGE_SEMA_TYPECHECKINVERTIBLE_H
#define LANGUAGE_SEMA_TYPECHECKINVERTIBLE_H

#include "language/AST/TypeCheckRequests.h"
#include "language/AST/ProtocolConformance.h"

namespace language {

class StorageVisitor {
public:
  /// Visit the instance storage of the given nominal type as seen through
  /// the given declaration context.
  ///
  /// The `this` instance is invoked with each (stored property, property type)
  /// pair for classes/structs and with each (enum elem, associated value type)
  /// pair for enums. It is up to you to implement these handlers by subclassing
  /// this visitor.
  ///
  /// \returns \c true if any call to this \c visitor's handlers returns \c true
  /// and \c false otherwise.
  bool visit(NominalTypeDecl *nominal, DeclContext *dc);

  /// Handle a stored property.
  /// \returns true iff this visitor should stop its walk over the nominal.
  virtual bool operator()(VarDecl *property, Type propertyType) = 0;

  /// Handle an enum associated value.
  /// \returns true iff this visitor should stop its walk over the nominal.
  virtual bool operator()(EnumElementDecl *element, Type elementType) = 0;

  virtual ~StorageVisitor() = default;
};

/// Checks that all stored properties or associated values are Copyable.
void checkCopyableConformance(DeclContext *dc,
                              ProtocolConformanceRef conformance);

/// Checks that all stored properties or associated values are Escapable.
void checkEscapableConformance(DeclContext *dc,
                               ProtocolConformanceRef conformance);
}


#endif // LANGUAGE_SEMA_TYPECHECKINVERTIBLE_H
