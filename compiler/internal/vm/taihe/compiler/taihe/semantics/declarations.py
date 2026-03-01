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

"""Defines the types for declarations."""

from abc import ABC, abstractmethod
from collections.abc import Collection
from typing import TYPE_CHECKING, Generic, TypeVar, cast

from typing_extensions import override

from taihe.semantics.format import PrettyFormatter
from taihe.semantics.types import (
    EnumType,
    IfaceType,
    NonVoidType,
    ScalarType,
    StringType,
    StructType,
    Type,
    UnionType,
    UserType,
)
from taihe.utils.exceptions import DeclRedefError
from taihe.utils.sources import SourceLocation

if TYPE_CHECKING:
    from taihe.semantics.attributes import AnyAttribute
    from taihe.semantics.visitor import (
        CallbackTypeRefVisitor,
        DeclarationImportVisitor,
        DeclarationRefVisitor,
        DeclVisitor,
        EnumDeclVisitor,
        EnumItemVisitor,
        ExplicitTypeRefVisitor,
        GenericArgVisitor,
        GenericTypeRefVisitor,
        GlobFuncVisitor,
        IfaceDeclVisitor,
        IfaceExtendVisitor,
        IfaceMethodVisitor,
        ImplicitTypeRefVisitor,
        ImportVisitor,
        LongTypeRefVisitor,
        PackageGroupVisitor,
        PackageImportVisitor,
        PackageLevelVisitor,
        PackageRefVisitor,
        PackageVisitor,
        ParamVisitor,
        ShortTypeRefVisitor,
        StructDeclVisitor,
        StructFieldVisitor,
        TypeDeclVisitor,
        TypeRefVisitor,
        UnionDeclVisitor,
        UnionFieldVisitor,
    )


_R = TypeVar("_R")


################
# Declarations #
################


_A = TypeVar("_A", bound="AnyAttribute")


class Decl(ABC):
    """Represents any declaration."""

    loc: SourceLocation | None

    attributes: dict[type["AnyAttribute"], list["AnyAttribute"]]

    def __init__(
        self,
        loc: SourceLocation | None,
    ):
        self.loc = loc
        self.attributes = {}

    def __repr__(self) -> str:
        return f"<{self.__class__.__qualname__} {self.description}>"

    @property
    @abstractmethod
    def description(self) -> str:
        """Human-readable description of this declaration."""

    @property
    @abstractmethod
    def parent_pkg(self) -> "PackageDecl":
        """Return the parent package of this declaration."""

    @abstractmethod
    def accept(self, v: "DeclVisitor[_R]") -> _R:
        """Accept a visitor to visit this declaration."""

    def add_attribute(self, a: "AnyAttribute"):
        self.attributes.setdefault(type(a), []).append(a)

    def find_attributes(self, t: type[_A]) -> list[_A]:
        return cast(list[_A], self.attributes.get(t, []))


class NamedDecl(Decl, ABC):
    """Represents a declaration with a name."""

    name: str

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
    ):
        super().__init__(loc)
        self.name = name


############################
# Declarations with Parent #
############################


_P = TypeVar("_P", bound=Decl)


class DeclWithParent(Decl, Generic[_P], ABC):
    _node_parent: _P | None = None

    @property
    @override
    def parent_pkg(self) -> "PackageDecl":
        pass
        return self._node_parent.parent_pkg

    def set_parent(self, parent: _P):
        self._node_parent = parent


class NamedDeclWithParent(NamedDecl, Generic[_P], ABC):
    _node_parent: _P | None = None

    @property
    @override
    def parent_pkg(self) -> "PackageDecl":
        pass
        return self._node_parent.parent_pkg

    def set_parent(self, parent: _P):
        self._node_parent = parent


###################
# Type References #
###################


class TypeRefDecl(DeclWithParent["TypeHolderDecl"], ABC):
    """Repersents a reference to a `Type`.

    Each user of a `Type` must be encapsulated in a `TypeRefDecl`.
    Also, `TypeRefDecl` is NOT a `TypeDecl`.
    In other words, `TypeRefDecl` is a pointer, instead of a declaration.

    For example:
    ```
    struct Foo { ... }      // `Foo` is a `TypeDecl`.

    fn func(foo: Foo);      // `Foo` is `TypeRefDecl(ty=UserType(decl=TypeDecl('Foo')))`.
    fn func(foo: BadType);  // `BadType` is `TypeRefDecl(ty=None)`.
    ```
    """

    is_resolved: bool = False
    """Whether this type reference is resolved."""

    resolved_ty_or_none: Type | None = None
    """The resolved type, if any.

    This field is `None` only if the type is not resolved yet.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
    ):
        super().__init__(loc)

    @property
    @override
    def description(self) -> str:
        if (fmt := self.format(PrettyFormatter())) is not None:
            return f"explicit type reference ({fmt})"
        return "implicit type reference"

    @property
    def parent_type_holder(self) -> "TypeHolderDecl":
        pass
        return self._node_parent

    @property
    def resolved_ty(self) -> Type:
        pass
        assert self.resolved_ty_or_none, "Type reference resolution failed"
        return self.resolved_ty_or_none

    def resolve(self, ty: Type | None):
        pass
        self.is_resolved = True
        self.resolved_ty_or_none = ty

    @abstractmethod
    def format(self, fmt: PrettyFormatter) -> str | None:
        """Format this type reference into a string with a formatter."""

    @abstractmethod
    def accept(self, v: "TypeRefVisitor[_R]") -> _R:
        ...


class ImplicitTypeRefDecl(TypeRefDecl):
    """A special type reference that represents an implicit type.

    This type reference is used when the type is not explicitly specified.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
    ):
        super().__init__(loc)

    def format(self, fmt: PrettyFormatter) -> None:
        return None

    @override
    def accept(self, v: "ImplicitTypeRefVisitor[_R]") -> _R:
        return v.visit_implicit_type_ref(self)


class ExplicitTypeRefDecl(TypeRefDecl):
    """Represents an explicit type reference.

    This type reference is used when the type is explicitly specified.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
    ):
        super().__init__(loc)

    def format(self, fmt: PrettyFormatter) -> str:
        return fmt.get_type_ref(self)

    @abstractmethod
    def accept(self, v: "ExplicitTypeRefVisitor[_R]") -> _R:
        ...


class GenericArgDecl(DeclWithParent["GenericTypeRefDecl"]):
    ty_ref: ExplicitTypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        ty_ref: ExplicitTypeRefDecl,
    ):
        super().__init__(loc)
        self.ty_ref = ty_ref
        self.ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"generic argument ({self.ty_ref.description})"

    @property
    def parent_generic_type_ref(self) -> "GenericTypeRefDecl":
        pass
        return self._node_parent

    @property
    def ty_or_none(self) -> Type | None:
        return cast(Type | None, self.ty_ref.resolved_ty_or_none)  # type: ignore

    @property
    def ty(self) -> Type:
        return cast(Type, self.ty_ref.resolved_ty)  # type: ignore

    def resolve_ty(self, ty: Type | None):
        self.ty_ref.resolve(ty)

    @override
    def accept(self, v: "GenericArgVisitor[_R]") -> _R:
        return v.visit_generic_arg(self)


class ParamDecl(NamedDeclWithParent["FunctionLikeDecl"]):
    ty_ref: ExplicitTypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: ExplicitTypeRefDecl,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref
        self.ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"parameter {self.name}"

    @property
    def parent_func(self) -> "FunctionLikeDecl":
        pass
        return self._node_parent

    @property
    def ty_or_none(self) -> NonVoidType | None:
        return cast(NonVoidType | None, self.ty_ref.resolved_ty_or_none)

    @property
    def ty(self) -> NonVoidType:
        return cast(NonVoidType, self.ty_ref.resolved_ty)

    def resolve_ty(self, ty: NonVoidType | None):
        self.ty_ref.resolve(ty)

    @override
    def accept(self, v: "ParamVisitor[_R]") -> _R:
        return v.visit_param(self)


class ShortTypeRefDecl(ExplicitTypeRefDecl):
    symbol: str

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
    ):
        super().__init__(loc)
        self.symbol = symbol

    @override
    def accept(self, v: "ShortTypeRefVisitor[_R]") -> _R:
        return v.visit_short_type_ref(self)


class LongTypeRefDecl(ExplicitTypeRefDecl):
    pkname: str
    symbol: str

    def __init__(
        self,
        loc: SourceLocation | None,
        pkname: str,
        symbol: str,
    ):
        super().__init__(loc)
        self.pkname = pkname
        self.symbol = symbol

    @override
    def accept(self, v: "LongTypeRefVisitor[_R]") -> _R:
        return v.visit_long_type_ref(self)


class GenericTypeRefDecl(ExplicitTypeRefDecl):
    symbol: str
    args: list[GenericArgDecl]

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
    ):
        super().__init__(loc)
        self.symbol = symbol
        self.args = []

    def add_arg(self, a: GenericArgDecl):
        self.args.append(a)
        a.set_parent(self)

    @override
    def accept(self, v: "GenericTypeRefVisitor[_R]") -> _R:
        return v.visit_generic_type_ref(self)


class CallbackTypeRefDecl(ExplicitTypeRefDecl):
    _param_dict: dict[str, ParamDecl]
    return_ty_ref: ExplicitTypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        return_ty_ref: ExplicitTypeRefDecl,
    ):
        super().__init__(loc)
        self._param_dict = {}
        self.return_ty_ref = return_ty_ref
        self.return_ty_ref.set_parent(self)

    @property
    def params(self) -> Collection[ParamDecl]:
        return self._param_dict.values()

    @property
    def return_ty_or_none(self) -> Type | None:
        return cast(Type | None, self.return_ty_ref.resolved_ty_or_none)  # type: ignore

    @property
    def return_ty(self) -> Type:
        return cast(Type, self.return_ty_ref.resolved_ty)  # type: ignore

    def resolve_return_ty(self, return_ty: Type | None):
        self.return_ty_ref.resolve(return_ty)

    def add_param(self, p: ParamDecl):
        if (prev := self._param_dict.setdefault(p.name, p)) != p:
            raise DeclRedefError(prev, p)
        p.set_parent(self)

    @override
    def accept(self, v: "CallbackTypeRefVisitor[_R]") -> _R:
        return v.visit_callback_type_ref(self)


#####################
# Import References #
#####################


class PackageRefDecl(DeclWithParent["PackageImportDecl | DeclarationRefDecl"]):
    symbol: str

    is_resolved: bool = False
    """Whether this package reference is resolved."""

    resolved_pkg_or_none: "PackageDecl | None" = None
    """The resolved package, if any.

    This field is `None` either if the package is not resolved yet or invalid.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
    ):
        super().__init__(loc)
        self.symbol = symbol

    @property
    @override
    def description(self) -> str:
        return f"package reference {self.symbol}"

    @override
    def accept(self, v: "PackageRefVisitor[_R]") -> _R:
        return v.visit_package_ref(self)


class DeclarationRefDecl(DeclWithParent["DeclarationImportDecl"]):
    symbol: str

    pkg_ref: PackageRefDecl

    is_resolved: bool = False
    """Whether this declaration reference is resolved."""

    resolved_decl_or_none: "PackageLevelDecl | None" = None
    """The resolved declaration, if any.

    This field is `None` either if the declaration is not resolved yet or invalid.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
        pkg_ref: PackageRefDecl,
    ):
        super().__init__(loc)
        self.symbol = symbol
        self.pkg_ref = pkg_ref
        self.pkg_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"type reference {self.symbol}"

    @override
    def accept(self, v: "DeclarationRefVisitor[_R]") -> _R:
        return v.visit_declaration_ref(self)


#######################
# Import Declarations #
#######################


class ImportDecl(NamedDeclWithParent["PackageDecl"], ABC):
    """Represents a package or declaration import.

    Invariant: the `name` field in base class `Decl` always represents actual name of imports.

    For example:

    ```
    >>> use foo;
    PackageImportDecl(name='foo', pkg_ref=PackageRefDecl(name='foo'))

    >>> use foo as bar;
    PackageImportDecl(name='bar', pkg_ref=PackageRefDecl(name='foo'))

    >>> from foo use Bar;
    DeclarationImportDecl(
        name='Bar',
        decl_ref=DeclarationRefDecl(name='Bar', pkg_ref=PackageRefDecl(name='foo')),
    )

    >>> from foo use Bar as Baz;
    DeclarationImportDecl(
        name='Baz',
        decl_ref=DeclarationRefDecl(name='Bar', pkg_ref=PackageRefDecl(name='foo')),
    )
    ```
    """

    @abstractmethod
    def accept(self, v: "ImportVisitor[_R]") -> _R:
        ...


class PackageImportDecl(ImportDecl):
    pkg_ref: PackageRefDecl

    def __init__(
        self,
        pkg_ref: PackageRefDecl,
        *,
        loc: SourceLocation | None = None,
        name: str = "",
    ):
        super().__init__(
            name=name or pkg_ref.symbol,
            loc=loc or pkg_ref.loc,
        )
        self.pkg_ref = pkg_ref
        self.pkg_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"package import {self.name}"

    def is_alias(self) -> bool:
        return self.name != self.pkg_ref.symbol

    @override
    def accept(self, v: "PackageImportVisitor[_R]") -> _R:
        return v.visit_package_import(self)


class DeclarationImportDecl(ImportDecl):
    decl_ref: DeclarationRefDecl

    def __init__(
        self,
        decl_ref: DeclarationRefDecl,
        *,
        loc: SourceLocation | None = None,
        name: str = "",
    ):
        super().__init__(
            name=name or decl_ref.symbol,
            loc=loc or decl_ref.loc,
        )
        self.decl_ref = decl_ref
        self.decl_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"declaration import {self.name}"

    def is_alias(self) -> bool:
        return self.name != self.decl_ref.symbol

    @override
    def accept(self, v: "DeclarationImportVisitor[_R]") -> _R:
        return v.visit_declaration_import(self)


############################
# Field Level Declarations #
############################


class EnumItemDecl(NamedDeclWithParent["EnumDecl"]):
    value: int | float | str | bool | None

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        value: int | float | str | bool | None = None,
    ):
        super().__init__(loc, name)
        self.value = value

    @property
    @override
    def description(self):
        return f"enum item {self.name}"

    @property
    def parent_enum(self) -> "EnumDecl":
        pass
        return self._node_parent

    @override
    def accept(self, v: "EnumItemVisitor[_R]") -> _R:
        return v.visit_enum_item(self)


class UnionFieldDecl(NamedDeclWithParent["UnionDecl"]):
    ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: ExplicitTypeRefDecl | None = None,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref or ImplicitTypeRefDecl(loc)
        self.ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"union field {self.name}"

    @property
    def parent_union(self) -> "UnionDecl":
        pass
        return self._node_parent

    @property
    def ty_or_none(self) -> NonVoidType | None:
        return cast(NonVoidType | None, self.ty_ref.resolved_ty_or_none)

    @property
    def ty(self) -> NonVoidType:
        return cast(NonVoidType, self.ty_ref.resolved_ty)

    def resolve_ty(self, ty: NonVoidType | None):
        self.ty_ref.resolve(ty)

    @override
    def accept(self, v: "UnionFieldVisitor[_R]") -> _R:
        return v.visit_union_field(self)


class StructFieldDecl(NamedDeclWithParent["StructDecl"]):
    ty_ref: ExplicitTypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: ExplicitTypeRefDecl,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref
        self.ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"struct field {self.name}"

    @property
    def parent_struct(self) -> "StructDecl":
        pass
        return self._node_parent

    @property
    def ty_or_none(self) -> NonVoidType | None:
        return cast(NonVoidType | None, self.ty_ref.resolved_ty_or_none)

    @property
    def ty(self) -> NonVoidType:
        return cast(NonVoidType, self.ty_ref.resolved_ty)

    def resolve_ty(self, ty: NonVoidType | None):
        self.ty_ref.resolve(ty)

    @override
    def accept(self, v: "StructFieldVisitor[_R]") -> _R:
        return v.visit_struct_field(self)


class IfaceExtendDecl(DeclWithParent["IfaceDecl"]):
    ty_ref: ExplicitTypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        ty_ref: ExplicitTypeRefDecl,
    ):
        super().__init__(loc)
        self.ty_ref = ty_ref
        self.ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"interface parent ({self.ty_ref.description})"

    @property
    def parent_iface(self) -> "IfaceDecl":
        pass
        return self._node_parent

    @property
    def ty_or_none(self) -> IfaceType | None:
        return cast(IfaceType | None, self.ty_ref.resolved_ty_or_none)

    @property
    def ty(self) -> IfaceType:
        return cast(IfaceType, self.ty_ref.resolved_ty)

    def resolve_ty(self, ty: IfaceType | None):
        self.ty_ref.resolve(ty)

    @override
    def accept(self, v: "IfaceExtendVisitor[_R]") -> _R:
        return v.visit_iface_extend(self)


class IfaceMethodDecl(NamedDeclWithParent["IfaceDecl"]):
    _param_dict: dict[str, ParamDecl]
    return_ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        return_ty_ref: ExplicitTypeRefDecl | None = None,
    ):
        super().__init__(loc, name)
        self._param_dict = {}
        self.return_ty_ref = return_ty_ref or ImplicitTypeRefDecl(loc)
        self.return_ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"interface method {self.name}"

    @property
    def parent_iface(self) -> "IfaceDecl":
        pass
        return self._node_parent

    @property
    def params(self) -> Collection[ParamDecl]:
        return self._param_dict.values()

    @property
    def return_ty_or_none(self) -> Type | None:
        return cast(Type | None, self.return_ty_ref.resolved_ty_or_none)  # type: ignore

    @property
    def return_ty(self) -> Type:
        return cast(Type, self.return_ty_ref.resolved_ty)  # type: ignore

    def resolve_return_ty(self, return_ty: Type | None):
        self.return_ty_ref.resolve(return_ty)

    def add_param(self, p: ParamDecl):
        if (prev := self._param_dict.setdefault(p.name, p)) != p:
            raise DeclRedefError(prev, p)
        p.set_parent(self)

    @override
    def accept(self, v: "IfaceMethodVisitor[_R]") -> _R:
        return v.visit_iface_method(self)


##############################
# Package Level Declarations #
##############################


class PackageLevelDecl(NamedDeclWithParent["PackageDecl"], ABC):
    @property
    def full_name(self):
        return f"{self.parent_pkg.name}.{self.name}"

    @abstractmethod
    def accept(self, v: "PackageLevelVisitor[_R]") -> _R:
        ...


class GlobFuncDecl(PackageLevelDecl):
    _param_dict: dict[str, ParamDecl]
    return_ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        return_ty_ref: ExplicitTypeRefDecl | None = None,
    ):
        super().__init__(loc, name)
        self._param_dict = {}
        self.return_ty_ref = return_ty_ref or ImplicitTypeRefDecl(loc)
        self.return_ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"function {self.name}"

    @property
    def params(self) -> Collection[ParamDecl]:
        return self._param_dict.values()

    @property
    def return_ty_or_none(self) -> Type | None:
        return cast(Type | None, self.return_ty_ref.resolved_ty_or_none)  # type: ignore

    @property
    def return_ty(self) -> Type:
        return cast(Type, self.return_ty_ref.resolved_ty)  # type: ignore

    def resolve_return_ty(self, return_ty: Type | None):
        self.return_ty_ref.resolve(return_ty)

    def add_param(self, p: ParamDecl):
        if (prev := self._param_dict.setdefault(p.name, p)) != p:
            raise DeclRedefError(prev, p)
        p.set_parent(self)

    @override
    def accept(self, v: "GlobFuncVisitor[_R]") -> _R:
        return v.visit_glob_func(self)


#####################
# Type Declarations #
#####################


class TypeDecl(PackageLevelDecl, ABC):
    @abstractmethod
    def as_type(self, ty_ref: TypeRefDecl) -> UserType:
        """Return the type decalaration as type."""

    @abstractmethod
    def accept(self, v: "TypeDeclVisitor[_R]") -> _R:
        ...


class EnumDecl(TypeDecl):
    ty_ref: ExplicitTypeRefDecl
    _item_dict: dict[str, EnumItemDecl]

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: ExplicitTypeRefDecl,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref
        self.ty_ref.set_parent(self)
        self._item_dict = {}

    @property
    @override
    def description(self) -> str:
        return f"enum {self.name}"

    @property
    def items(self) -> Collection[EnumItemDecl]:
        return self._item_dict.values()

    @property
    def ty_or_none(self) -> ScalarType | StringType | None:
        return cast(ScalarType | StringType | None, self.ty_ref.resolved_ty_or_none)

    @property
    def ty(self) -> ScalarType | StringType:
        return cast(ScalarType | StringType, self.ty_ref.resolved_ty)

    def resolve_ty(self, ty: ScalarType | StringType | None):
        self.ty_ref.resolve(ty)

    def add_item(self, i: EnumItemDecl):
        if (prev := self._item_dict.setdefault(i.name, i)) != i:
            raise DeclRedefError(prev, i)
        i.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> EnumType:
        return EnumType(ty_ref, self)

    @override
    def accept(self, v: "EnumDeclVisitor[_R]") -> _R:
        return v.visit_enum_decl(self)


class UnionDecl(TypeDecl):
    _field_dict: dict[str, UnionFieldDecl]

    def __init__(self, loc: SourceLocation | None, name: str):
        super().__init__(loc, name)
        self._field_dict = {}

    @property
    @override
    def description(self) -> str:
        return f"union {self.name}"

    @property
    def fields(self) -> Collection[UnionFieldDecl]:
        return self._field_dict.values()

    def add_field(self, f: UnionFieldDecl):
        if (prev := self._field_dict.setdefault(f.name, f)) != f:
            raise DeclRedefError(prev, f)
        f.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> UnionType:
        return UnionType(ty_ref, self)

    @override
    def accept(self, v: "UnionDeclVisitor[_R]") -> _R:
        return v.visit_union_decl(self)


class StructDecl(TypeDecl):
    _field_dict: dict[str, StructFieldDecl]

    def __init__(self, loc: SourceLocation | None, name: str):
        super().__init__(loc, name)
        self._field_dict = {}

    @property
    @override
    def description(self) -> str:
        return f"struct {self.name}"

    @property
    def fields(self) -> Collection[StructFieldDecl]:
        return self._field_dict.values()

    def add_field(self, f: StructFieldDecl):
        if (prev := self._field_dict.setdefault(f.name, f)) != f:
            raise DeclRedefError(prev, f)
        f.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> StructType:
        return StructType(ty_ref, self)

    @override
    def accept(self, v: "StructDeclVisitor[_R]") -> _R:
        return v.visit_struct_decl(self)


class IfaceDecl(TypeDecl):
    _extend_list: list[IfaceExtendDecl]
    _method_dict: dict[str, IfaceMethodDecl]

    def __init__(self, loc: SourceLocation | None, name: str):
        super().__init__(loc, name)
        self._extend_list = []
        self._method_dict = {}

    @property
    @override
    def description(self) -> str:
        return f"interface {self.name}"

    @property
    def extends(self) -> Collection[IfaceExtendDecl]:
        return self._extend_list

    @property
    def methods(self) -> Collection[IfaceMethodDecl]:
        return self._method_dict.values()

    def add_extend(self, p: IfaceExtendDecl):
        self._extend_list.append(p)
        p.set_parent(self)

    def add_method(self, f: IfaceMethodDecl):
        if (prev := self._method_dict.setdefault(f.name, f)) != f:
            raise DeclRedefError(prev, f)
        f.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> IfaceType:
        return IfaceType(ty_ref, self)

    @override
    def accept(self, v: "IfaceDeclVisitor[_R]") -> _R:
        return v.visit_iface_decl(self)


NamedFunctionLikeDecl = GlobFuncDecl | IfaceMethodDecl
FunctionLikeDecl = NamedFunctionLikeDecl | CallbackTypeRefDecl
TypeHolderDecl = (
    FunctionLikeDecl
    | ParamDecl
    | StructFieldDecl
    | UnionFieldDecl
    | EnumDecl
    | IfaceExtendDecl
    | GenericArgDecl
)


######################
# The main container #
######################


class PackageDecl(NamedDecl):
    """A collection of named identities sharing the same scope."""

    _node_parent: "PackageGroup | None" = None

    # Imports
    _pkg_import_dict: dict[str, PackageImportDecl]
    _decl_import_dict: dict[str, DeclarationImportDecl]

    # Symbols
    _declaration_dict: dict[str, PackageLevelDecl]

    # Things that the package contains.
    functions: list[GlobFuncDecl]
    structs: list[StructDecl]
    unions: list[UnionDecl]
    interfaces: list[IfaceDecl]
    enums: list[EnumDecl]

    is_stdlib: bool

    def __init__(self, loc: SourceLocation | None, name: str, is_stdlib: bool):
        super().__init__(loc, name)

        self._pkg_import_dict = {}
        self._decl_import_dict = {}

        self._declaration_dict = {}

        self.functions = []
        self.structs = []
        self.unions = []
        self.interfaces = []
        self.enums = []

        self.is_stdlib = is_stdlib

    @property
    @override
    def description(self) -> str:
        return f"package {self.name}"

    @property
    @override
    def parent_pkg(self) -> "PackageDecl":
        return self

    @property
    def parent_group(self) -> "PackageGroup":
        pass
        return self._node_parent

    @property
    def segments(self) -> list[str]:
        return self.name.split(".")

    @property
    def pkg_imports(self) -> Collection[PackageImportDecl]:
        return self._pkg_import_dict.values()

    @property
    def decl_imports(self) -> Collection[DeclarationImportDecl]:
        return self._decl_import_dict.values()

    @property
    def declarations(self) -> Collection[PackageLevelDecl]:
        return self._declaration_dict.values()

    def set_group(self, group: "PackageGroup"):
        self._node_parent = group

    def lookup(self, name: str) -> PackageLevelDecl | None:
        return self._declaration_dict.get(name)

    def lookup_pkg_import(self, name: str) -> PackageImportDecl | None:
        return self._pkg_import_dict.get(name)

    def lookup_decl_import(self, name: str) -> DeclarationImportDecl | None:
        return self._decl_import_dict.get(name)

    def add_import(self, i: ImportDecl):
        if isinstance(i, DeclarationImportDecl):
            self.add_decl_import(i)
        elif isinstance(i, PackageImportDecl):
            self.add_pkg_import(i)
        else:
            raise NotImplementedError(f"unexpected import {i.description}")

    def add_decl_import(self, i: DeclarationImportDecl):
        if (prev := self._decl_import_dict.setdefault(i.name, i)) != i:
            raise DeclRedefError(prev, i)
        i.set_parent(self)

    def add_pkg_import(self, i: PackageImportDecl):
        if (prev := self._pkg_import_dict.setdefault(i.name, i)) != i:
            raise DeclRedefError(prev, i)
        i.set_parent(self)

    def add_declaration(self, d: PackageLevelDecl):
        if isinstance(d, GlobFuncDecl):
            self.add_function(d)
        elif isinstance(d, StructDecl):
            self.add_struct(d)
        elif isinstance(d, UnionDecl):
            self.add_union(d)
        elif isinstance(d, IfaceDecl):
            self.add_interface(d)
        elif isinstance(d, EnumDecl):
            self.add_enum(d)
        else:
            raise NotImplementedError(f"unexpected declaration {d.description}")

    def add_function(self, d: GlobFuncDecl):
        if (prev := self._declaration_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        self.functions.append(d)
        d.set_parent(self)

    def add_enum(self, d: EnumDecl):
        if (prev := self._declaration_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        self.enums.append(d)
        d.set_parent(self)

    def add_struct(self, d: StructDecl):
        if (prev := self._declaration_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        self.structs.append(d)
        d.set_parent(self)

    def add_union(self, d: UnionDecl):
        if (prev := self._declaration_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        self.unions.append(d)
        d.set_parent(self)

    def add_interface(self, d: IfaceDecl):
        if (prev := self._declaration_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        self.interfaces.append(d)
        d.set_parent(self)

    @override
    def accept(self, v: "PackageVisitor[_R]") -> _R:
        return v.visit_package(self)


class PackageGroup:
    """Stores all known packages for a compilation instance."""

    _all_package_dict: dict[str, PackageDecl]
    _package_dict: dict[str, PackageDecl]

    def __init__(self):
        super().__init__()
        self._all_package_dict = {}
        self._package_dict = {}

    def __repr__(self) -> str:
        packages_str = ", ".join(repr(x) for x in self._package_dict)
        return f"{self.__class__.__qualname__}({packages_str})"

    @property
    def all_packages(self) -> Collection[PackageDecl]:
        return self._all_package_dict.values()

    @property
    def packages(self) -> Collection[PackageDecl]:
        return self._package_dict.values()

    def lookup(self, name: str) -> PackageDecl | None:
        return self._package_dict.get(name)

    def add(self, d: PackageDecl):
        if (prev := self._all_package_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        if not d.is_stdlib:
            self._package_dict[d.name] = d
        d.set_group(self)

    def accept(self, v: "PackageGroupVisitor[_R]"):
        return v.visit_package_group(self)