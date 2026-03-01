# coding=utf-8
#
#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

"""Defines the type system."""

from abc import ABC, abstractmethod
from collections.abc import Callable
from dataclasses import dataclass
from enum import Enum
from typing import TYPE_CHECKING, TypeVar

from typing_extensions import override

from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import GenericArgumentsError, TypeUsageError

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        CallbackTypeRefDecl,
        EnumDecl,
        GenericTypeRefDecl,
        IfaceDecl,
        StructDecl,
        TypeDecl,
        TypeRefDecl,
        UnionDecl,
    )
    from taihe.semantics.visitor import (
        ArrayTypeVisitor,
        BuiltinTypeVisitor,
        CallbackTypeVisitor,
        EnumTypeVisitor,
        GenericTypeVisitor,
        IfaceTypeVisitor,
        MapTypeVisitor,
        NonVoidTypeVisitor,
        OpaqueTypeVisitor,
        OptionalTypeVisitor,
        ScalarTypeVisitor,
        SetTypeVisitor,
        StringTypeVisitor,
        StructTypeVisitor,
        TypeVisitor,
        UnionTypeVisitor,
        UnitTypeVisitor,
        UserTypeVisitor,
        VectorTypeVisitor,
        VoidTypeVisitor,
    )


_R = TypeVar("_R")


############################
# Infrastructure for Types #
############################


@dataclass(frozen=True, repr=False)
class Type(ABC):
    """Base class for all types."""

    ref: "TypeRefDecl"

    def __repr__(self) -> str:
        return f"<{self.__class__.__qualname__} {self.signature}>"

    @property
    @abstractmethod
    def signature(self) -> str:
        """Return the representation of the type."""

    @abstractmethod
    def accept(self, v: "TypeVisitor[_R]") -> _R:
        ...


@dataclass(frozen=True, repr=False)
class VoidType(Type):
    """Represents the void type, which indicates no value."""

    @property
    @override
    def signature(self):
        return "void"

    @override
    def accept(self, v: "VoidTypeVisitor[_R]") -> _R:
        return v.visit_void_type(self)


@dataclass(frozen=True, repr=False)
class NonVoidType(Type, ABC):
    """Represents a valid type that can be used in the type system."""

    @override
    def accept(self, v: "NonVoidTypeVisitor[_R]") -> _R:
        ...


#################
# Builtin Types #
#################


@dataclass(frozen=True, repr=False)
class BuiltinType(NonVoidType, ABC):
    """Base class for literal types."""

    @abstractmethod
    def accept(self, v: "BuiltinTypeVisitor[_R]") -> _R:
        ...


@dataclass(frozen=True, repr=False)
class UnitType(BuiltinType):
    @property
    @override
    def signature(self):
        return "unit"

    @override
    def accept(self, v: "UnitTypeVisitor[_R]") -> _R:
        return v.visit_unit_type(self)


class ScalarKind(Enum):
    """Enumeration of scalar types."""

    BOOL = ("bool", 8, False, False)
    F32 = ("f32", 32, True, True)
    F64 = ("f64", 64, True, True)
    I8 = ("i8", 8, True, False)
    I16 = ("i16", 16, True, False)
    I32 = ("i32", 32, True, False)
    I64 = ("i64", 64, True, False)
    U8 = ("u8", 8, False, False)
    U16 = ("u16", 16, False, False)
    U32 = ("u32", 32, False, False)
    U64 = ("u64", 64, False, False)

    def __init__(self, symbol: str, width: int, is_signed: bool, is_float: bool):
        self.symbol = symbol
        self.width = width
        self.is_signed = is_signed
        self.is_float = is_float


@dataclass(frozen=True, repr=False)
class ScalarType(BuiltinType):
    kind: ScalarKind

    @property
    @override
    def signature(self):
        return self.kind.symbol

    @override
    def accept(self, v: "ScalarTypeVisitor[_R]") -> _R:
        return v.visit_scalar_type(self)


@dataclass(frozen=True, repr=False)
class StringType(BuiltinType):
    @property
    @override
    def signature(self):
        return "String"

    @override
    def accept(self, v: "StringTypeVisitor[_R]") -> _R:
        return v.visit_string_type(self)


@dataclass(frozen=True, repr=False)
class OpaqueType(BuiltinType):
    @property
    @override
    def signature(self):
        return "Opaque"

    @override
    def accept(self, v: "OpaqueTypeVisitor[_R]") -> _R:
        return v.visit_opaque_type(self)


# Builtin Types Map
BUILTIN_TYPES: dict[str, Callable[["TypeRefDecl"], Type]] = {
    "void": VoidType,
    "unit": UnitType,
    "bool": lambda ty_ref: ScalarType(ty_ref, ScalarKind.BOOL),
    "f32": lambda ty_ref: ScalarType(ty_ref, ScalarKind.F32),
    "f64": lambda ty_ref: ScalarType(ty_ref, ScalarKind.F64),
    "i8": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I8),
    "i16": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I16),
    "i32": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I32),
    "i64": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I64),
    "u8": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U8),
    "u16": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U16),
    "u32": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U32),
    "u64": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U64),
    "String": StringType,
    "Opaque": OpaqueType,
}


#################
# Callback Type #
#################


@dataclass(frozen=True, repr=False)
class CallbackType(NonVoidType):
    ref: "CallbackTypeRefDecl"

    @property
    @override
    def signature(self):
        params_fmt = ", ".join(
            f"{param.name}: {param.ty.signature}" for param in self.ref.params
        )
        return_fmt = self.ref.return_ty.signature
        return f"({params_fmt}) => {return_fmt}"

    @override
    def accept(self, v: "CallbackTypeVisitor[_R]") -> _R:
        return v.visit_callback_type(self)


####################
# Builtin Generics #
####################


class GenericType(NonVoidType, ABC):
    @classmethod
    @abstractmethod
    def try_construct(
        cls,
        ref: "GenericTypeRefDecl",
        dm: DiagnosticsManager,
    ) -> "GenericType | None":
        ...

    @abstractmethod
    def accept(self, v: "GenericTypeVisitor[_R]") -> _R:
        ...


@dataclass(frozen=True, repr=False)
class ArrayType(GenericType):
    item_ty: NonVoidType

    @property
    @override
    def signature(self):
        return f"Array<{self.item_ty.signature}>"

    @classmethod
    def try_construct(
        cls,
        ref: "GenericTypeRefDecl",
        dm: DiagnosticsManager,
    ) -> "ArrayType | None":
        if len(ref.args) != 1:
            dm.emit(GenericArgumentsError(ref, 1, len(ref.args)))
            return None
        item_ty = ref.args[0].ty
        if not isinstance(item_ty, NonVoidType):
            dm.emit(TypeUsageError(ref.args[0].ty_ref, item_ty))
            return None
        return cls(ref, item_ty)

    @override
    def accept(self, v: "ArrayTypeVisitor[_R]") -> _R:
        return v.visit_array_type(self)


@dataclass(frozen=True, repr=False)
class OptionalType(GenericType):
    item_ty: NonVoidType

    @property
    @override
    def signature(self):
        return f"Optional<{self.item_ty.signature}>"

    @classmethod
    def try_construct(
        cls,
        ref: "GenericTypeRefDecl",
        dm: DiagnosticsManager,
    ) -> "OptionalType | None":
        if len(ref.args) != 1:
            dm.emit(GenericArgumentsError(ref, 1, len(ref.args)))
            return None
        item_ty = ref.args[0].ty
        if not isinstance(item_ty, NonVoidType):
            dm.emit(TypeUsageError(ref.args[0].ty_ref, item_ty))
            return None
        return cls(ref, item_ty)

    @override
    def accept(self, v: "OptionalTypeVisitor[_R]") -> _R:
        return v.visit_optional_type(self)


@dataclass(frozen=True, repr=False)
class VectorType(GenericType):
    val_ty: NonVoidType

    @property
    @override
    def signature(self):
        return f"Vector<{self.val_ty.signature}>"

    @classmethod
    def try_construct(
        cls,
        ref: "GenericTypeRefDecl",
        dm: DiagnosticsManager,
    ) -> "VectorType | None":
        if len(ref.args) != 1:
            dm.emit(GenericArgumentsError(ref, 1, len(ref.args)))
            return None
        key_ty = ref.args[0].ty
        if not isinstance(key_ty, NonVoidType):
            dm.emit(TypeUsageError(ref.args[0].ty_ref, key_ty))
            return None
        return cls(ref, key_ty)

    @override
    def accept(self, v: "VectorTypeVisitor[_R]") -> _R:
        return v.visit_vector_type(self)


@dataclass(frozen=True, repr=False)
class MapType(GenericType):
    key_ty: NonVoidType
    val_ty: NonVoidType

    @property
    @override
    def signature(self):
        return f"Map<{self.key_ty.signature}, {self.val_ty.signature}>"

    @classmethod
    def try_construct(
        cls,
        ref: "GenericTypeRefDecl",
        dm: DiagnosticsManager,
    ) -> "MapType | None":
        if len(ref.args) != 2:
            dm.emit(GenericArgumentsError(ref, 2, len(ref.args)))
            return None
        key_ty = ref.args[0].ty
        if not isinstance(key_ty, NonVoidType):
            dm.emit(TypeUsageError(ref.args[0].ty_ref, key_ty))
            return None
        val_ty = ref.args[1].ty
        if not isinstance(val_ty, NonVoidType):
            dm.emit(TypeUsageError(ref.args[1].ty_ref, val_ty))
            return None
        return cls(ref, key_ty, val_ty)

    @override
    def accept(self, v: "MapTypeVisitor[_R]") -> _R:
        return v.visit_map_type(self)


@dataclass(frozen=True, repr=False)
class SetType(GenericType):
    key_ty: NonVoidType

    @property
    @override
    def signature(self):
        return f"Set<{self.key_ty.signature}>"

    @classmethod
    def try_construct(
        cls,
        ref: "GenericTypeRefDecl",
        dm: DiagnosticsManager,
    ) -> "SetType | None":
        if len(ref.args) != 1:
            dm.emit(GenericArgumentsError(ref, 1, len(ref.args)))
            return None
        key_ty = ref.args[0].ty
        if not isinstance(key_ty, NonVoidType):
            dm.emit(TypeUsageError(ref.args[0].ty_ref, key_ty))
            return None
        return cls(ref, key_ty)

    @override
    def accept(self, v: "SetTypeVisitor[_R]") -> _R:
        return v.visit_set_type(self)


# Builtin Generics Map
BUILTIN_GENERICS: dict[str, type[GenericType]] = {
    "Array": ArrayType,
    "Optional": OptionalType,
    "Vector": VectorType,
    "Map": MapType,
    "Set": SetType,
}


##############
# User Types #
##############


@dataclass(frozen=True, repr=False)
class UserType(NonVoidType, ABC):
    decl: "TypeDecl"

    @property
    @override
    def signature(self):
        return self.decl.full_name

    @abstractmethod
    def accept(self, v: "UserTypeVisitor[_R]") -> _R:
        ...


@dataclass(frozen=True, repr=False)
class EnumType(UserType):
    decl: "EnumDecl"

    @override
    def accept(self, v: "EnumTypeVisitor[_R]") -> _R:
        return v.visit_enum_type(self)


@dataclass(frozen=True, repr=False)
class StructType(UserType):
    decl: "StructDecl"

    @override
    def accept(self, v: "StructTypeVisitor[_R]") -> _R:
        return v.visit_struct_type(self)


@dataclass(frozen=True, repr=False)
class UnionType(UserType):
    decl: "UnionDecl"

    @override
    def accept(self, v: "UnionTypeVisitor[_R]") -> _R:
        return v.visit_union_type(self)


@dataclass(frozen=True, repr=False)
class IfaceType(UserType):
    decl: "IfaceDecl"

    @override
    def accept(self, v: "IfaceTypeVisitor[_R]") -> _R:
        return v.visit_iface_type(self)