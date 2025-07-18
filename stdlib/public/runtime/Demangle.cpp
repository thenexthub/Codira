//===----------------------------------------------------------------------===//
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

#include "Private.h"
#include "language/ABI/TypeIdentity.h"
#include "language/Basic/Range.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/Portability.h"
#include "language/Strings.h"

#include <vector>
#include <inttypes.h>

#if LANGUAGE_OBJC_INTEROP
#include <objc/runtime.h>
#endif

using namespace language;

Demangle::NodePointer
language::_buildDemanglingForContext(const ContextDescriptor *context,
                                  toolchain::ArrayRef<NodePointer> demangledGenerics,
                                  Demangle::Demangler &Dem) {
  unsigned usedDemangledGenerics = 0;
  NodePointer node = nullptr;

  // Walk up the context tree.
  toolchain::SmallVector<const ContextDescriptor *, 8> descriptorPath;
  {
    const ContextDescriptor *parent = context;
    while (parent) {
      descriptorPath.push_back(parent);
      parent = parent->Parent;
    }
  }

  auto getGenericArgsTypeListForContext =
    [&](const ContextDescriptor *context) -> NodePointer {
      if (demangledGenerics.empty())
        return nullptr;

      if (context->getKind() == ContextDescriptorKind::Anonymous)
        return nullptr;
      
      auto generics = context->getGenericContext();
      if (!generics)
        return nullptr;
      
      auto numParams = generics->getGenericContextHeader().NumParams;
      if (numParams <= usedDemangledGenerics)
        return nullptr;
      
      auto genericArgsList = Dem.createNode(Node::Kind::TypeList);
      for (unsigned e = generics->getGenericContextHeader().NumParams;
           usedDemangledGenerics < e;
           ++usedDemangledGenerics) {
        genericArgsList->addChild(demangledGenerics[usedDemangledGenerics],
                                  Dem);
      }
      return genericArgsList;
    };
  
  for (auto component : toolchain::reverse(descriptorPath)) {
    switch (auto kind = component->getKind()) {
    case ContextDescriptorKind::Module: {
      assert(node == nullptr && "module should be top level");
      auto name = toolchain::cast<ModuleContextDescriptor>(component)->Name.get();
      node = Dem.createNode(Node::Kind::Module, name);
      break;
    }
    
    case ContextDescriptorKind::Extension: {
      auto extension = toolchain::cast<ExtensionContextDescriptor>(component);
      // Demangle the extension self type.
      auto selfType = Dem.demangleType(extension->getMangledExtendedContext(),
                                       ResolveToDemanglingForContext(Dem));
      if (selfType->getKind() == Node::Kind::Type)
        selfType = selfType->getChild(0);
      
      // Substitute in the generic arguments.
      auto genericArgsList = getGenericArgsTypeListForContext(component);
      
      if (selfType->getKind() == Node::Kind::BoundGenericEnum
          || selfType->getKind() == Node::Kind::BoundGenericStructure
          || selfType->getKind() == Node::Kind::BoundGenericClass
          || selfType->getKind() == Node::Kind::BoundGenericOtherNominalType) {
        if (genericArgsList) {
          auto substSelfType = Dem.createNode(selfType->getKind());
          substSelfType->addChild(selfType->getChild(0), Dem);
          substSelfType->addChild(genericArgsList, Dem);
          selfType = substSelfType;
        } else {
          // TODO: Use the unsubstituted type if we can't handle the
          // substitutions yet.
          selfType = selfType->getChild(0)->getChild(0);
        }
      }
      
      auto extNode = Dem.createNode(Node::Kind::Extension);
      extNode->addChild(node, Dem);
      extNode->addChild(selfType, Dem);
      
      // TODO: Turn the generic signature into a demangling as the third
      // generic argument.
      
      node = extNode;
      break;
    }

    case ContextDescriptorKind::Protocol: {
      auto protocol = toolchain::cast<ProtocolDescriptor>(component);
      auto name = protocol->Name.get();

      auto protocolNode = Dem.createNode(Node::Kind::Protocol);
      protocolNode->addChild(node, Dem);
      auto nameNode = Dem.createNode(Node::Kind::Identifier, name);
      protocolNode->addChild(nameNode, Dem);

      node = protocolNode;
      break;
    }

    default:
      // Form a type context demangling for type contexts.
      if (auto type = toolchain::dyn_cast<TypeContextDescriptor>(component)) {
        auto identity = ParsedTypeIdentity::parse(type);

        Node::Kind nodeKind;
        Node::Kind genericNodeKind;
        switch (kind) {
        case ContextDescriptorKind::Class:
          nodeKind = Node::Kind::Class;
          genericNodeKind = Node::Kind::BoundGenericClass;
          break;
        case ContextDescriptorKind::Struct:
          nodeKind = Node::Kind::Structure;
          genericNodeKind = Node::Kind::BoundGenericStructure;
          break;
        case ContextDescriptorKind::Enum:
          nodeKind = Node::Kind::Enum;
          genericNodeKind = Node::Kind::BoundGenericEnum;
          break;
        default:
          // We don't know about this kind of type. Use an "other type" mangling
          // for it.
          nodeKind = Node::Kind::OtherNominalType;
          genericNodeKind = Node::Kind::BoundGenericOtherNominalType;
          break;
        }
        
        // Override the node kind if this is a Clang-imported type so we give it
        // a stable mangling.
        if (identity.isCTypedef()) {
          nodeKind = Node::Kind::TypeAlias;
        } else if (nodeKind != Node::Kind::Structure &&
                   _isCImportedTagType(type, identity)) {
          nodeKind = Node::Kind::Structure;
        }
        
        auto typeNode = Dem.createNode(nodeKind);
        typeNode->addChild(node, Dem);
        auto nameNode = Dem.createNode(Node::Kind::Identifier,
                                       identity.getABIName());
        if (identity.isAnyRelatedEntity()) {
          auto kindNode = Dem.createNode(Node::Kind::Identifier,
                                     identity.getRelatedEntityName());
          auto relatedName = Dem.createNode(Node::Kind::RelatedEntityDeclName);
          relatedName->addChild(kindNode, Dem);
          relatedName->addChild(nameNode, Dem);
          nameNode = relatedName;
        }
        typeNode->addChild(nameNode, Dem);
        node = typeNode;
        
        // Apply generic arguments if the context is generic.
        if (auto genericArgsList = getGenericArgsTypeListForContext(component)){
          auto unspecializedType = Dem.createNode(Node::Kind::Type);
          unspecializedType->addChild(node, Dem);

          auto genericNode = Dem.createNode(genericNodeKind);
          genericNode->addChild(unspecializedType, Dem);
          genericNode->addChild(genericArgsList, Dem);
          node = genericNode;
        }
        
        break;
      }

      // This runtime doesn't understand this context, or it's a context with
      // no richer runtime information available about it (such as an anonymous
      // context). Use an unstable mangling to represent the context by its
      // pointer identity.
      char addressBuf[sizeof(void*) * 2 + 1 + 1];
      snprintf(addressBuf, sizeof(addressBuf), "$%" PRIxPTR, (uintptr_t)component);
      
      auto anonNode = Dem.createNode(Node::Kind::AnonymousContext);
      CharVector addressStr;
      addressStr.append(addressBuf, Dem);
      auto name = Dem.createNode(Node::Kind::Identifier, addressStr);
      anonNode->addChild(name, Dem);
      anonNode->addChild(node, Dem);
      
      // Collect generic arguments if the context is generic.
      auto genericArgsList = getGenericArgsTypeListForContext(component);
      if (!genericArgsList)
        genericArgsList = Dem.createNode(Node::Kind::TypeList);
      anonNode->addChild(genericArgsList, Dem);
      
      node = anonNode;
      
      break;
    }
  }
  
  // Wrap the final result in a top-level Type node.
  auto top = Dem.createNode(Node::Kind::Type);
  top->addChild(node, Dem);
  return top;
}

// FIXME: This stuff should be merged with the existing logic in
// include/language/RemoteInspection/TypeRefBuilder.h as part of the rewrite
// to change stdlib reflection over to using remote mirrors.

Demangle::NodePointer
language::_language_buildDemanglingForMetadata(const Metadata *type,
                                         Demangle::Demangler &Dem);

static Demangle::NodePointer
_buildDemanglerForBuiltinType(const Metadata *type, Demangle::Demangler &Dem) {
#define BUILTIN_TYPE(Symbol, Name) \
  if (type == &METADATA_SYM(Symbol).base) \
    return Dem.createNode(Node::Kind::BuiltinTypeName, Name);
#if !LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES
#define BUILTIN_VECTOR_TYPE(ElementSymbol, ElementName, Width)
#endif
#include "language/Runtime/BuiltinTypes.def"
  return nullptr;
}

// Build a demangled type tree for a type pack.
static Demangle::NodePointer
_buildDemanglingForMetadataPack(MetadataPackPointer pack, size_t count,
                                Demangle::Demangler &Dem) {
  using namespace Demangle;

  auto node = Dem.createNode(Node::Kind::Pack);

  for (size_t i = 0; i < count; ++i) {
    auto elt = _language_buildDemanglingForMetadata(pack.getElements()[i], Dem);
    if (elt == nullptr)
      return nullptr;

    node->addChild(elt, Dem);
  }

  return node;
}

static Demangle::NodePointer
_buildDemanglingForValue(intptr_t value, GenericValueDescriptor valueDescriptor,
                         Demangle::Demangler &Dem) {
  switch (valueDescriptor.Type) {
  case GenericValueType::Int: {
    if (value < 0) {
      return Dem.createNode(Node::Kind::NegativeInteger, value);
    }

    return Dem.createNode(Node::Kind::Integer, value);
  }
  }
}

/// Build an array of demangling trees for each generic argument to the given
/// generic type context descriptor.
///
/// Note:
/// - The input array has an entry for those generic parameter descriptors which
///   are key arguments only.
/// - The output array has an entry for each generic parameter descriptor,
///   whether or not it is a key argument.
///
/// The generic parameters which are not key arguments were made non-canonical
/// by constraining them to a concrete type or another generic parameter.
///
/// We figure out their type by looking at the same-type requirements of the
/// generic context. We demangle their type from the requirement, using the
/// generic arguments area as the substitution map; this gives us the metadata
/// for their argument. Then we convert the metadata to a mangling.
///
/// The output array is flat; the generic parameters of each depth are
/// concatenated together.
static bool _buildDemanglingForGenericArgs(
    const TypeContextDescriptor *description,
    const Metadata *const *genericArgs,
    toolchain::SmallVectorImpl<NodePointer> &demangledGenerics,
    Demangle::Demangler &Dem) {
  auto generics = description->getGenericContext();
  if (!generics)
    return true;

  auto packHeader = generics->getGenericPackShapeHeader();
  auto packDescriptors = generics->getGenericPackShapeDescriptors();

  toolchain::SmallVector<MetadataPackOrValue> allGenericArgs;

  auto numKeyArgs = 0;
  for (auto param : generics->getGenericParams()) {
    if (param.hasKeyArgument()) {
      numKeyArgs += 1;
    }
  }

  // _gatherWrittenGenericParameters expects to immediately read key generic
  // arguments, so skip past the shape classes if we have any.
  auto nonShapeClassGenericArgs = genericArgs + packHeader.NumShapeClasses;

  toolchain::ArrayRef<const void *> genericArgsRef(
      reinterpret_cast<const void * const *>(nonShapeClassGenericArgs), numKeyArgs);

  if (!_gatherWrittenGenericParameters(description, genericArgsRef,
                                       allGenericArgs, Dem)) {
    return false;
  }

  auto argIndex = 0;
  auto packIndex = 0;

  for (auto param : generics->getGenericParams()) {
    auto genericArg = allGenericArgs[argIndex];

    switch (param.getKind()) {
    case GenericParamKind::Type: {
      auto metadata = genericArg.getMetadata();
      auto genericArgDemangling =
        _language_buildDemanglingForMetadata(metadata, Dem);

      if (!genericArgDemangling) {
        return false;
      }

      demangledGenerics.push_back(genericArgDemangling);
      break;
    }

    case GenericParamKind::TypePack: {
      auto packDescriptor = packDescriptors[packIndex];
      assert(packDescriptor.Kind == GenericPackKind::Metadata);
      assert(packDescriptor.ShapeClass < packHeader.NumShapeClasses);

      // Arg index is not interested in the shape classes, but the pack
      // descriptor's index is in terms of the shape classes.
      assert(packDescriptor.Index == argIndex + packHeader.NumShapeClasses);

      MetadataPackPointer pack(genericArg.getMetadataPack());
      size_t count = reinterpret_cast<size_t>(genericArgs[packDescriptor.ShapeClass]);

      auto genericArgDemangling = _buildDemanglingForMetadataPack(pack, count, Dem);
      if (genericArgDemangling == nullptr)
        return false;

      demangledGenerics.push_back(genericArgDemangling);
      packIndex += 1;
      break;
    }

    case GenericParamKind::Value: {
      // FIXME: We really want to get the specific value descriptor when
      // building a demangling, but when all value parameters become non-key
      // we lose the value header. May need to do extra work to walk up parents
      // and find this parameter's specific value descriptor.

      auto valueDescriptor = GenericValueDescriptor{GenericValueType::Int};
      auto value = genericArg.getValue();
      auto genericArgDemangling = _buildDemanglingForValue(value,
                                                           valueDescriptor,
                                                           Dem);

      if (!genericArgDemangling)
        return false;

      demangledGenerics.push_back(genericArgDemangling);
      break;
    }

    default:
      // We don't know about this kind of parameter.
      return false;
    }

    argIndex += 1;
  }

  return true;
}

/// Build a demangled type tree for a nominal type.
static Demangle::NodePointer
_buildDemanglingForNominalType(const Metadata *type, Demangle::Demangler &Dem) {
  using namespace Demangle;

  // Get the context descriptor from the type metadata.
  const TypeContextDescriptor *description;

  switch (type->getKind()) {
  case MetadataKind::Class: {
    auto classType = static_cast<const ClassMetadata *>(type);

    if (!classType->isTypeMetadata()) {
      return nullptr;
    }

#if LANGUAGE_OBJC_INTEROP
    // Peek through artificial subclasses.
    while (classType->isArtificialSubclass())
      classType = classType->Superclass;
#endif
    description = classType->getDescription();
    break;
  }
  case MetadataKind::Enum:
  case MetadataKind::Optional: {
    auto enumType = static_cast<const EnumMetadata *>(type);
    description = enumType->Description;
    break;
  }
  case MetadataKind::Struct: {
    auto structType = static_cast<const StructMetadata *>(type);
    description = structType->Description;
    break;
  }
  case MetadataKind::ForeignClass: {
    auto foreignType = static_cast<const ForeignClassMetadata *>(type);
    description = foreignType->Description;
    break;
  }
  case MetadataKind::ForeignReferenceType: {
    auto foreignType = static_cast<const ForeignReferenceTypeMetadata *>(type);
    description = foreignType->Description;
    break;
  }
  default:
    return nullptr;
  }

  toolchain::SmallVector<NodePointer, 8> demangledGenerics;
  if (description->isGeneric()) {
    auto genericArgs = description->getGenericArguments(type);
    // Gather the complete set of generic arguments that must be written to
    // form this type.
    if (!_buildDemanglingForGenericArgs(description, genericArgs,
                                        demangledGenerics, Dem))
      return nullptr;
  }

  return _buildDemanglingForContext(description, demangledGenerics, Dem);
}

static Demangle::NodePointer
_replaceGeneralizationArg(Demangle::NodePointer type,
                          SubstGenericParametersFromMetadata substitutions,
                          Demangle::Demangler &Dem) {
  assert(type->getKind() == Node::Kind::Type);
  auto genericParam = type->getChild(0);

  if (genericParam->getKind() != Node::Kind::DependentGenericParamType)
    return type;

  auto depth = genericParam->getChild(0)->getIndex();
  auto index = genericParam->getChild(1)->getIndex();

  auto arg = substitutions.getMetadata(depth, index);
  assert(arg.isMetadata());
  return _language_buildDemanglingForMetadata(arg.getMetadata(), Dem);
}

static Demangle::NodePointer
_buildDemanglingForExtendedExistential(const Metadata *type,
                                       Demangle::Demangler &Dem) {
  auto ee = cast<ExtendedExistentialTypeMetadata>(type);

  auto demangledExistential = Dem.demangleType(ee->Shape->ExistentialType.get(),
                                               ResolveToDemanglingForContext(Dem));

  if (!ee->Shape->hasGeneralizationSignature())
    return demangledExistential;

  SubstGenericParametersFromMetadata substitutions(ee->Shape,
                                              ee->getGeneralizationArguments());

  // Dig out the requirement list.
  auto constrainedExistential = demangledExistential->getChild(0);
  assert(constrainedExistential->getKind() == Node::Kind::ConstrainedExistential);
  auto reqList = constrainedExistential->getChild(1);
  assert(reqList->getKind() == Node::Kind::ConstrainedExistentialRequirementList);

  auto newReqList = Dem.createNode(Node::Kind::ConstrainedExistentialRequirementList);

  for (auto req : *reqList) {
    // Currently, the only requirements that can create generalization arguments
    // are same types.
    if (req->getKind() != Node::Kind::DependentGenericSameTypeRequirement) {
      newReqList->addChild(req, Dem);
      continue;
    }

    auto lhs = _replaceGeneralizationArg(req->getChild(0), substitutions, Dem);
    auto rhs = _replaceGeneralizationArg(req->getChild(1), substitutions, Dem);

    auto newReq = Dem.createNode(Node::Kind::DependentGenericSameTypeRequirement);
    newReq->addChild(lhs, Dem);
    newReq->addChild(rhs, Dem);

    newReqList->addChild(newReq, Dem);
  }

  constrainedExistential->replaceChild(1, newReqList);

  return demangledExistential;
}

// Build a demangled type tree for a type.
//
// FIXME: This should use MetadataReader.h.
Demangle::NodePointer
language::_language_buildDemanglingForMetadata(const Metadata *type,
                                         Demangle::Demangler &Dem) {
  using namespace Demangle;

  switch (type->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::Enum:
  case MetadataKind::Optional:
  case MetadataKind::Struct:
  case MetadataKind::ForeignClass:
  case MetadataKind::ForeignReferenceType:
    return _buildDemanglingForNominalType(type, Dem);
  case MetadataKind::ObjCClassWrapper: {
#if LANGUAGE_OBJC_INTEROP
    auto objcWrapper = static_cast<const ObjCClassWrapperMetadata *>(type);
    const char *className = class_getName(objcWrapper->getObjCClassObject());
    
    auto module = Dem.createNode(Node::Kind::Module, MANGLING_MODULE_OBJC);
    auto node = Dem.createNode(Node::Kind::Class);
    node->addChild(module, Dem);
    node->addChild(Dem.createNode(Node::Kind::Identifier,
                                       toolchain::StringRef(className)), Dem);
    
    return node;
#else
    assert(false && "no ObjC interop");
    return nullptr;
#endif
  }
  case MetadataKind::Existential: {
    auto exis = static_cast<const ExistentialTypeMetadata *>(type);
    auto protocols = exis->getProtocols();

    auto type_list = Dem.createNode(Node::Kind::TypeList);
    auto proto_list = Dem.createNode(Node::Kind::ProtocolList);
    proto_list->addChild(type_list, Dem);

    // The protocol descriptors should be pre-sorted since the compiler will
    // only ever make a language_getExistentialTypeMetadata invocation using
    // its canonical ordering of protocols.

    for (auto protocol : protocols) {
#if LANGUAGE_OBJC_INTEROP
      if (protocol.isObjC()) {
        // The protocol name is mangled as a type symbol, with the _Tt prefix.
        StringRef ProtoName(protocol.getName());
        NodePointer protocolNode = Dem.demangleSymbol(ProtoName);

        // ObjC protocol names aren't mangled.
        if (!protocolNode) {
          auto module = Dem.createNode(Node::Kind::Module,
                                            MANGLING_MODULE_OBJC);
          auto node = Dem.createNode(Node::Kind::Protocol);
          node->addChild(module, Dem);
          node->addChild(Dem.createNode(Node::Kind::Identifier, ProtoName),
                         Dem);
          auto typeNode = Dem.createNode(Node::Kind::Type);
          typeNode->addChild(node, Dem);
          type_list->addChild(typeNode, Dem);
          continue;
        }

        // Dig out the protocol node.
        // Global -> (Protocol|TypeMangling)
        protocolNode = protocolNode->getChild(0);
        if (protocolNode->getKind() == Node::Kind::TypeMangling) {
          protocolNode = protocolNode->getChild(0); // TypeMangling -> Type
          protocolNode = protocolNode->getChild(0); // Type -> ProtocolList
          protocolNode = protocolNode->getChild(0); // ProtocolList -> TypeList
          protocolNode = protocolNode->getChild(0); // TypeList -> Type

          assert(protocolNode->getKind() == Node::Kind::Type);
          assert(protocolNode->getChild(0)->getKind() == Node::Kind::Protocol);
        } else {
          assert(protocolNode->getKind() == Node::Kind::Protocol);
        }

        type_list->addChild(protocolNode, Dem);
        continue;
      }
#endif

      auto protocolNode =
          _buildDemanglingForContext(protocol.getCodiraProtocol(), {}, Dem);
      if (!protocolNode)
        return nullptr;

      type_list->addChild(protocolNode, Dem);
    }

    if (auto superclass = exis->getSuperclassConstraint()) {
      // If there is a superclass constraint, we mangle it specially.
      auto result = Dem.createNode(Node::Kind::ProtocolListWithClass);
      auto superclassNode = _language_buildDemanglingForMetadata(superclass, Dem);
      if (!superclassNode)
        return nullptr;

      result->addChild(proto_list, Dem);
      result->addChild(superclassNode, Dem);
      return result;
    }

    if (exis->isClassBounded()) {
      // Check if the class constraint is implied by any of our
      // protocols.
      bool requiresClassImplicit = false;

      for (auto protocol : protocols) {
        if (protocol.getClassConstraint() == ProtocolClassConstraint::Class)
          requiresClassImplicit = true;
      }

      // If it was implied, we don't do anything special.
      if (requiresClassImplicit)
        return proto_list;

      // If the existential type has an explicit AnyObject constraint,
      // we must mangle it as such.
      auto result = Dem.createNode(Node::Kind::ProtocolListWithAnyObject);
      result->addChild(proto_list, Dem);
      return result;
    }

    // Just a simple composition of protocols.
    return proto_list;
  }
  case MetadataKind::ExtendedExistential: {
    return _buildDemanglingForExtendedExistential(type, Dem);
  }
  case MetadataKind::ExistentialMetatype: {
    auto metatype = static_cast<const ExistentialMetatypeMetadata *>(type);
    auto instance = _language_buildDemanglingForMetadata(metatype->InstanceType,
                                                      Dem);
    if (!instance)
      return nullptr;
    auto node = Dem.createNode(Node::Kind::ExistentialMetatype);
    node->addChild(instance, Dem);
    return node;
  }
  case MetadataKind::Function: {
    auto fn = static_cast<const FunctionTypeMetadata *>(type);

    Node::Kind kind;
    switch (fn->getConvention()) {
    case FunctionMetadataConvention::Codira:
      if (!fn->isEscaping())
        kind = Node::Kind::NoEscapeFunctionType;
      else
        kind = Node::Kind::FunctionType;
      break;
    case FunctionMetadataConvention::Block:
      kind = Node::Kind::ObjCBlock;
      break;
    case FunctionMetadataConvention::CFunctionPointer:
      kind = Node::Kind::CFunctionPointer;
      break;
    case FunctionMetadataConvention::Thin:
      kind = Node::Kind::ThinFunctionType;
      break;
    }

    toolchain::SmallVector<std::pair<NodePointer, bool>, 8> inputs;
    for (unsigned i = 0, e = fn->getNumParameters(); i < e; ++i) {
      auto param = fn->getParameter(i);
      auto flags = fn->getParameterFlags(i);
      auto input = _language_buildDemanglingForMetadata(param, Dem);

      // If we failed to build the demangling for an input, we have to fail
      // building the demangling for the function type too.
      if (!input)
        return nullptr;

      auto wrapInput = [&](Node::Kind kind) {
        auto parent = Dem.createNode(kind);
        parent->addChild(input, Dem);
        input = parent;
      };
      if (flags.isNoDerivative()) {
        wrapInput(Node::Kind::NoDerivative);
      }
      switch (flags.getOwnership()) {
      case ParameterOwnership::Default:
        /* nothing */
        break;
      case ParameterOwnership::InOut:
        wrapInput(Node::Kind::InOut);
        break;
      case ParameterOwnership::Shared:
        wrapInput(Node::Kind::Shared);
        break;
      case ParameterOwnership::Owned:
        wrapInput(Node::Kind::Owned);
        break;
      }
      if (flags.isIsolated()) {
        wrapInput(Node::Kind::Isolated);
      }
      if (flags.isSending()) {
        wrapInput(Node::Kind::Sending);
      }

      inputs.push_back({input, flags.isVariadic()});
    }

    NodePointer totalInput = nullptr;
    switch (inputs.size()) {
    case 1: {
      auto singleParam = inputs.front();

      // If the sole unlabeled parameter has a non-tuple type, encode
      // the parameter list as a single type.
      if (!singleParam.second) {
        auto singleType = singleParam.first;
        if (singleType->getKind() == Node::Kind::Type)
          singleType = singleType->getFirstChild();
        if (singleType->getKind() != Node::Kind::Tuple) {
          totalInput = singleParam.first;
          break;
        }
      }

      // Otherwise it requires a tuple wrapper.
      LANGUAGE_FALLTHROUGH;
    }

    // This covers both none and multiple parameters.
    default:
      auto tuple = Dem.createNode(Node::Kind::Tuple);
      for (auto &input : inputs) {
        NodePointer eltType;
        bool isVariadic;
        std::tie(eltType, isVariadic) = input;

        // Tuple element := variadic-marker label? type
        auto tupleElt = Dem.createNode(Node::Kind::TupleElement);

        if (isVariadic)
          tupleElt->addChild(Dem.createNode(Node::Kind::VariadicMarker), Dem);

        if (eltType->getKind() == Node::Kind::Type) {
          tupleElt->addChild(eltType, Dem);
        } else {
          auto type = Dem.createNode(Node::Kind::Type);
          type->addChild(eltType, Dem);
          tupleElt->addChild(type, Dem);
        }

        tuple->addChild(tupleElt, Dem);
      }
      totalInput = tuple;
      break;
    }

    NodePointer parameters = Dem.createNode(Node::Kind::ArgumentTuple);
    NodePointer paramType = Dem.createNode(Node::Kind::Type);

    paramType->addChild(totalInput, Dem);
    parameters->addChild(paramType, Dem);

    NodePointer resultTy = _language_buildDemanglingForMetadata(fn->ResultType,
                                                             Dem);
    if (!resultTy)
      return nullptr;

    NodePointer result = Dem.createNode(Node::Kind::ReturnType);
    result->addChild(resultTy, Dem);

    auto funcNode = Dem.createNode(kind);
    // Add the components of the function-signature production in the same
    // order that the demangler would.
    // TODO: C function type would go here
    if (fn->getExtendedFlags().hasSendingResult())
      funcNode->addChild(Dem.createNode(Node::Kind::SendingResultFunctionType),
                         Dem);
    if (fn->hasGlobalActor()) {
      auto globalActorTypeNode =
          _language_buildDemanglingForMetadata(fn->getGlobalActor(), Dem);
      if (!globalActorTypeNode)
        return nullptr;
      NodePointer globalActorNode =
          Dem.createNode(Node::Kind::GlobalActorFunctionType);
      globalActorNode->addChild(globalActorTypeNode, Dem);
      funcNode->addChild(globalActorNode, Dem);
    } else if (fn->getExtendedFlags().isIsolatedAny()) {
      funcNode->addChild(Dem.createNode(
          Node::Kind::IsolatedAnyFunctionType), Dem);
    } else if (fn->getExtendedFlags().isNonIsolatedCaller()) {
      funcNode->addChild(Dem.createNode(
        Node::Kind::NonIsolatedCallerFunctionType), Dem);
    }
    switch (fn->getDifferentiabilityKind().Value) {
    case FunctionMetadataDifferentiabilityKind::NonDifferentiable:
      break;
    case FunctionMetadataDifferentiabilityKind::Forward:
      funcNode->addChild(Dem.createNode(
          Node::Kind::DifferentiableFunctionType,
          (Node::IndexType)MangledDifferentiabilityKind::Forward), Dem);
      break;
    case FunctionMetadataDifferentiabilityKind::Reverse:
      funcNode->addChild(Dem.createNode(
          Node::Kind::DifferentiableFunctionType,
          (Node::IndexType)MangledDifferentiabilityKind::Reverse), Dem);
      break;
    case FunctionMetadataDifferentiabilityKind::Normal:
      funcNode->addChild(Dem.createNode(
          Node::Kind::DifferentiableFunctionType,
          (Node::IndexType)MangledDifferentiabilityKind::Normal), Dem);
      break;
    case FunctionMetadataDifferentiabilityKind::Linear:
      funcNode->addChild(Dem.createNode(
          Node::Kind::DifferentiableFunctionType,
          (Node::IndexType)MangledDifferentiabilityKind::Linear), Dem);
      break;
    }
    if (fn->isThrowing()) {
      if (auto thrownError = fn->getThrownError()) {
        auto thrownErrorTypeNode =
          _language_buildDemanglingForMetadata(thrownError, Dem);
        if (!thrownErrorTypeNode)
          return nullptr;
        NodePointer thrownErrorNode =
            Dem.createNode(Node::Kind::TypedThrowsAnnotation);
        thrownErrorNode->addChild(thrownErrorTypeNode, Dem);
        funcNode->addChild(thrownErrorNode, Dem);
      } else {
        funcNode->addChild(Dem.createNode(Node::Kind::ThrowsAnnotation), Dem);
      }
    }
    if (fn->isSendable()) {
      funcNode->addChild(
          Dem.createNode(Node::Kind::ConcurrentFunctionType), Dem);
    }
    if (fn->isAsync())
      funcNode->addChild(Dem.createNode(Node::Kind::AsyncAnnotation), Dem);

    funcNode->addChild(parameters, Dem);
    funcNode->addChild(result, Dem);
    return funcNode;
  }
  case MetadataKind::Metatype: {
    auto metatype = static_cast<const MetatypeMetadata *>(type);
    auto instance = _language_buildDemanglingForMetadata(metatype->InstanceType,
                                                      Dem);
    if (!instance)
      return nullptr;

    auto typeNode = Dem.createNode(Node::Kind::Type);
    typeNode->addChild(instance, Dem);
    auto node = Dem.createNode(Node::Kind::Metatype);
    node->addChild(typeNode, Dem);
    return node;
  }
  case MetadataKind::Tuple: {
    auto tuple = static_cast<const TupleTypeMetadata *>(type);
    const char *labels = tuple->Labels;
    auto tupleNode = Dem.createNode(Node::Kind::Tuple);
    for (unsigned i = 0, e = tuple->NumElements; i < e; ++i) {
      auto elt = Dem.createNode(Node::Kind::TupleElement);

      // Add a label child if applicable:
      if (labels) {
        // Look for the next space in the labels string.
        if (const char *space = strchr(labels, ' ')) {
          // If there is one, and the label isn't empty, add a label child.
          if (labels != space) {
            auto eltName =
              Dem.createNode(Node::Kind::TupleElementName,
                                  toolchain::StringRef(labels, space - labels));
            elt->addChild(eltName, Dem);
          }

          // Skip past the space.
          labels = space + 1;
        }
      }

      // Add the element type child.
      auto eltType =
        _language_buildDemanglingForMetadata(tuple->getElement(i).Type, Dem);
      if (!eltType)
        return nullptr;

      if (eltType->getKind() == Node::Kind::Type) {
        elt->addChild(eltType, Dem);
      } else {
        auto type = Dem.createNode(Node::Kind::Type);
        type->addChild(eltType, Dem);
        elt->addChild(type, Dem);
      }

      // Add the completed element to the tuple.
      tupleNode->addChild(elt, Dem);
    }
    return tupleNode;
  }
  case MetadataKind::HeapLocalVariable:
  case MetadataKind::HeapGenericLocalVariable:
  case MetadataKind::ErrorObject:
    break;
  case MetadataKind::Opaque:
  default: {
    if (auto builtinType = _buildDemanglerForBuiltinType(type, Dem))
      return builtinType;
    
    // FIXME: Some opaque types do have manglings, but we don't have enough info
    // to figure them out.
    break;
  }
  }
  // Not a type.
  return nullptr;
}

Demangle::NodePointer
language::_buildDemanglingForGenericType(const TypeContextDescriptor *description,
                                      const void *const *arguments,
                                      Demangle::Demangler &Dem) {
  auto kind = description->getKind();
  if (kind != ContextDescriptorKind::Class &&
      kind != ContextDescriptorKind::Enum &&
      kind != ContextDescriptorKind::Struct)
    return nullptr;

  toolchain::SmallVector<NodePointer, 8> demangledGenerics;
  if (!_buildDemanglingForGenericArgs(description,
                                      (const Metadata *const *)arguments,
                                      demangledGenerics, Dem))
    return nullptr;

  return _buildDemanglingForContext(description, demangledGenerics, Dem);
}

// NB: This function is not used directly in the Codira codebase, but is
// exported for Xcode support and is used by the sanitizers. Please coordinate
// before changing.
//
/// Demangles a Codira symbol name.
///
/// \param mangledName is the symbol name that needs to be demangled.
/// \param mangledNameLength is the length of the string that should be
/// demangled.
/// \param outputBuffer is the user provided buffer where the demangled name
/// will be placed. If nullptr, a new buffer will be malloced. In that case,
/// the user of this API is responsible for freeing the returned buffer.
/// \param outputBufferSize is the size of the output buffer. If the demangled
/// name does not fit into the outputBuffer, the output will be truncated and
/// the size will be updated, indicating how large the buffer should be.
/// \param flags can be used to select the demangling style. TODO: We should
//// define what these will be.
/// \returns the demangled name. Returns nullptr if the input String is not a
/// Codira mangled name.
LANGUAGE_RUNTIME_EXPORT
char *language_demangle(const char *mangledName,
                     size_t mangledNameLength,
                     char *outputBuffer,
                     size_t *outputBufferSize,
                     uint32_t flags) {
  if (flags != 0) {
    language::fatalError(0, "Only 'flags' value of '0' is currently supported.");
  }
  if (outputBuffer != nullptr && outputBufferSize == nullptr) {
    language::fatalError(0, "'outputBuffer' is passed but the size is 'nullptr'.");
  }

  // Check if we are dealing with Codira mangled name, otherwise, don't try
  // to demangle and send indication to the user.
  if (!Demangle::isCodiraSymbol(mangledName))
    return nullptr; // Not a mangled name

#if !LANGUAGE_STDLIB_HAS_TYPE_PRINTING
  return nullptr;
#else
  // Demangle the name.
  auto options = Demangle::DemangleOptions();
  options.DisplayDebuggerGeneratedModule = false;
  auto result =
      Demangle::demangleSymbolAsString(mangledName,
                                       mangledNameLength,
                                       options);

  // If the output buffer is not provided, malloc memory ourselves.
  if (outputBuffer == nullptr || *outputBufferSize == 0) {
#if defined(_WIN32)
    return _strdup(result.c_str());
#else
    return strdup(result.c_str());
#endif
  }

  // Copy into the provided buffer.
  _language_strlcpy(outputBuffer, result.c_str(), *outputBufferSize);

  // Indicate a failure if the result did not fit and was truncated
  // by setting the required outputBufferSize.
  if (*outputBufferSize < result.length() + 1) {
    *outputBufferSize = result.length() + 1;
  }

  return outputBuffer;
#endif
}
