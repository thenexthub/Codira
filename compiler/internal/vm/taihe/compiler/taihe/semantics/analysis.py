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

from collections.abc import Callable
from itertools import chain
from math import isfinite
from typing import Any, TypeGuard, TypeVar

from typing_extensions import override

from taihe.semantics.attributes import AttributeRegistry, UncheckedAttribute
from taihe.semantics.declarations import (
    CallbackTypeRefDecl,
    Decl,
    DeclarationRefDecl,
    EnumDecl,
    ExplicitTypeRefDecl,
    GenericArgDecl,
    GenericTypeRefDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceExtendDecl,
    IfaceMethodDecl,
    ImplicitTypeRefDecl,
    LongTypeRefDecl,
    PackageDecl,
    PackageGroup,
    PackageLevelDecl,
    PackageRefDecl,
    ParamDecl,
    ShortTypeRefDecl,
    StructDecl,
    StructFieldDecl,
    TypeDecl,
    TypeRefDecl,
    UnionDecl,
    UnionFieldDecl,
)
from taihe.semantics.types import (
    BUILTIN_GENERICS,
    BUILTIN_TYPES,
    CallbackType,
    IfaceType,
    NonVoidType,
    ScalarKind,
    ScalarType,
    StringType,
    Type,
    UnitType,
    UserType,
    VoidType,
)
from taihe.semantics.visitor import (
    ExplicitTypeRefVisitor,
    RecursiveDeclVisitor,
    TypeRefVisitor,
)
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import (
    DeclarationNotInScopeError,
    DeclNotExistError,
    DuplicateExtendsWarn,
    EmptyStructOrUnionError,
    EnumValueError,
    NotATypeError,
    PackageNotExistError,
    PackageNotInScopeError,
    RecursiveReferenceError,
    SymbolConflictWithNamespaceError,
    TypeUsageError,
)


def analyze_semantics(
    pg: PackageGroup,
    dm: DiagnosticsManager,
    am: AttributeRegistry,
):
    """Runs semantic analysis passes on the given package group."""
    _check_decl_confilct_with_namespace(pg, dm)
    pg.accept(_ResolveImportsPass(dm))
    pg.accept(_ResolveTypePass(dm))
    pg.accept(_CheckEnumTypePass(dm))
    pg.accept(_CheckStructAndUnionNotEmptyPass(dm))
    pg.accept(_CheckRecursiveInclusionPass(dm))

    if dm.has_error:
        return

    pg.accept(_ConvertAttrPass(dm, am))
    pg.accept(_CheckAttrPass(dm))


def _check_decl_confilct_with_namespace(
    pg: PackageGroup,
    dm: DiagnosticsManager,
):
    """Checks for declarations conflicts with namespaces."""
    namespaces: dict[str, list[PackageDecl]] = {}
    for pkg in pg.packages:
        pkg_name = pkg.name
        # package "a.b.c" -> namespaces ["a.b.c", "a.b", "a"]
        while True:
            namespaces.setdefault(pkg_name, []).append(pkg)
            splited = pkg_name.rsplit(".", maxsplit=1)
            if len(splited) == 2:
                pkg_name = splited[0]
            else:
                break

    for p in pg.packages:
        for d in p.declarations:
            name = p.name + "." + d.name
            if packages := namespaces.get(name, []):
                dm.emit(SymbolConflictWithNamespaceError(d, name, packages))


class _ResolveImportsPass(RecursiveDeclVisitor):
    """Resolves imports and type references within a package group."""

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm
        self._curr_pg = None

    @property
    def curr_pg(self) -> PackageGroup:
        pass
        return self._curr_pg

    @override
    def visit_package_group(self, g: PackageGroup) -> None:
        self._curr_pg = g
        try:
            super().visit_package_group(g)
        finally:
            self._curr_pg = None

    @override
    def visit_package_ref(self, d: PackageRefDecl) -> None:
        super().visit_package_ref(d)
        if d.is_resolved:
            return
        d.is_resolved = True
        d.resolved_pkg_or_none = self.resolve_package_ref(d)

    @override
    def visit_declaration_ref(self, d: DeclarationRefDecl) -> None:
        super().visit_declaration_ref(d)
        if d.is_resolved:
            return
        d.is_resolved = True
        d.resolved_decl_or_none = self.resolve_declaration_ref(d)

    def resolve_package_ref(self, d: PackageRefDecl) -> PackageDecl | None:
        pkg = self.curr_pg.lookup(d.symbol)

        if pkg is not None:
            return pkg

        self.dm.emit(PackageNotExistError(d.symbol, loc=d.loc))
        return None

    def resolve_declaration_ref(self, d: DeclarationRefDecl) -> PackageLevelDecl | None:
        pkg = d.pkg_ref.resolved_pkg_or_none

        if pkg is not None:
            decl = pkg.lookup(d.symbol)

            if decl is not None:
                return decl

            self.dm.emit(DeclNotExistError(d.symbol, loc=d.loc))
            return None

        # No need to repeatedly throw exceptions
        return None


class _ExplicitTypeRefResolver(ExplicitTypeRefVisitor[Type | None]):
    """A visitor that resolves types in declarations."""

    def __init__(
        self,
        dm: DiagnosticsManager,
        curr_pkg: PackageDecl,
    ):
        self.dm = dm
        self.curr_pkg = curr_pkg

    @override
    def visit_long_type_ref(self, d: LongTypeRefDecl) -> Type | None:
        # Look for imported package declarations
        pkg_import = self.curr_pkg.lookup_pkg_import(d.pkname)

        if pkg_import is not None:
            pkg = pkg_import.pkg_ref.resolved_pkg_or_none

            if pkg is not None:
                decl = pkg.lookup(d.symbol)

                if decl is not None:
                    if isinstance(decl, TypeDecl):
                        return decl.as_type(d)

                    self.dm.emit(NotATypeError(d.symbol, loc=d.loc))
                    return None

                self.dm.emit(DeclNotExistError(d.symbol, loc=d.loc))
                return None

            # No need to repeatedly throw exceptions
            return None

        self.dm.emit(PackageNotInScopeError(d.pkname, loc=d.loc))
        return None

    @override
    def visit_short_type_ref(self, d: ShortTypeRefDecl) -> Type | None:
        # Find Builtin Types
        builtin_type = BUILTIN_TYPES.get(d.symbol)

        if builtin_type is not None:
            return builtin_type(d)

        # Find types declared in the current package
        decl = self.curr_pkg.lookup(d.symbol)

        if decl is not None:
            if isinstance(decl, TypeDecl):
                return decl.as_type(d)

            self.dm.emit(NotATypeError(d.symbol, loc=d.loc))
            return None

        # Look for imported type declarations
        decl_import = self.curr_pkg.lookup_decl_import(d.symbol)

        if decl_import is not None:
            decl = decl_import.decl_ref.resolved_decl_or_none

            if decl is not None:
                if isinstance(decl, TypeDecl):
                    return decl.as_type(d)

                self.dm.emit(NotATypeError(d.symbol, loc=d.loc))
                return None

            # No need to repeatedly throw exceptions
            return None

        self.dm.emit(DeclarationNotInScopeError(d.symbol, loc=d.loc))
        return None

    @override
    def visit_generic_type_ref(self, d: GenericTypeRefDecl) -> Type | None:
        # Find Builtin Generics
        builtin_generic = BUILTIN_GENERICS.get(d.symbol)

        if builtin_generic is not None:
            args: list[GenericArgDecl] = []
            for arg in d.args:
                if arg.ty_or_none is None:
                    return None
                args.append(arg)

            return builtin_generic.try_construct(d, dm=self.dm)

        self.dm.emit(DeclarationNotInScopeError(d.symbol, loc=d.loc))
        return None

    @override
    def visit_callback_type_ref(self, d: CallbackTypeRefDecl) -> Type | None:
        for param in d.params:
            if param.ty_or_none is None:
                return None
        if d.return_ty_or_none is None:
            return None
        return CallbackType(d)


class _TypeRefResolver(_ExplicitTypeRefResolver, TypeRefVisitor[Type | None]):
    """A visitor that resolves type references in declarations."""

    def __init__(
        self,
        dm: DiagnosticsManager,
        curr_pkg: PackageDecl,
        default_ctor: Callable[[ImplicitTypeRefDecl], Type],
    ):
        super().__init__(dm, curr_pkg)
        self.default_type = default_ctor

    @override
    def visit_implicit_type_ref(self, d: ImplicitTypeRefDecl) -> Type | None:
        return self.default_type(d)


_T = TypeVar("_T", bound=Type)


class _ResolveTypePass(RecursiveDeclVisitor):
    """Resolves type references within a package group."""

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm
        self._curr_pkg = None

    @property
    def curr_pkg(self) -> PackageDecl:
        pass
        return self._curr_pkg

    @override
    def visit_package(self, p: PackageDecl) -> None:
        self._curr_pkg = p
        try:
            super().visit_package(p)
        finally:
            self._curr_pkg = None

    @override
    def visit_param(self, d: ParamDecl) -> None:
        super().visit_param(d)
        ty = self.resolve_explicit_type_ref(d.ty_ref, NonVoidType)
        d.resolve_ty(ty)

    @override
    def visit_generic_arg(self, d: GenericArgDecl) -> None:
        super().visit_generic_arg(d)
        ty = self.resolve_explicit_type_ref(d.ty_ref, Type)
        d.resolve_ty(ty)

    @override
    def visit_callback_type_ref(self, d: CallbackTypeRefDecl) -> None:
        super().visit_callback_type_ref(d)
        ty = self.resolve_explicit_type_ref(d.return_ty_ref, Type)
        d.resolve_return_ty(ty)

    @override
    def visit_glob_func(self, d: GlobFuncDecl) -> None:
        super().visit_glob_func(d)
        ty = self.resolve_type_ref(d.return_ty_ref, Type, default_type=VoidType)
        d.resolve_return_ty(ty)

    @override
    def visit_iface_method(self, d: IfaceMethodDecl) -> None:
        super().visit_iface_method(d)
        ty = self.resolve_type_ref(d.return_ty_ref, Type, default_type=VoidType)
        d.resolve_return_ty(ty)

    @override
    def visit_union_field(self, d: UnionFieldDecl) -> None:
        super().visit_union_field(d)
        ty = self.resolve_type_ref(d.ty_ref, NonVoidType, default_type=UnitType)
        d.resolve_ty(ty)

    @override
    def visit_struct_field(self, d: StructFieldDecl) -> None:
        super().visit_struct_field(d)
        ty = self.resolve_explicit_type_ref(d.ty_ref, NonVoidType)
        d.resolve_ty(ty)

    @override
    def visit_iface_extend(self, d: IfaceExtendDecl) -> None:
        super().visit_iface_extend(d)
        ty = self.resolve_explicit_type_ref(d.ty_ref, IfaceType)
        d.resolve_ty(ty)

    @override
    def visit_enum_decl(self, d: EnumDecl) -> None:
        super().visit_enum_decl(d)
        ty = self.resolve_explicit_type_ref(d.ty_ref, ScalarType, StringType)
        d.resolve_ty(ty)

    def resolve_explicit_type_ref(
        self,
        ty_ref: ExplicitTypeRefDecl,
        *target: type[_T],
    ) -> _T | None:
        ty = ty_ref.accept(_ExplicitTypeRefResolver(self.dm, self.curr_pkg))
        if ty is None:
            return None
        if isinstance(ty, target):
            return ty
        self.dm.emit(TypeUsageError(ty_ref, ty))
        return None

    def resolve_type_ref(
        self,
        ty_ref: TypeRefDecl,
        *target: type[_T],
        default_type: Callable[[ImplicitTypeRefDecl], _T],
    ) -> _T | None:
        ty = ty_ref.accept(_TypeRefResolver(self.dm, self.curr_pkg, default_type))
        if ty is None:
            return None
        if isinstance(ty, target):
            return ty
        self.dm.emit(TypeUsageError(ty_ref, ty))
        return None


class _CheckEnumTypePass(RecursiveDeclVisitor):
    """Validated enum item types."""

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm

    @override
    def visit_enum_decl(self, d: EnumDecl) -> None:
        match d.ty_or_none:
            case None:
                pass

            case ScalarType():
                succ: Callable[[Any | None], Any]
                check: Callable[[Any], bool]

                def check_int(val: Any, min: int, max: int) -> TypeGuard[int]:
                    return (
                        not isinstance(val, bool)
                        and isinstance(val, int)
                        and min <= val < max
                    )

                def succ_int(pred: int | None, min: int, max: int) -> int:
                    if pred is None:
                        return 0
                    return succ if (succ := pred + 1) < max else min

                match d.ty_or_none.kind:
                    case ScalarKind.I8:
                        succ = lambda pred: succ_int(pred, -(2**7), 2**7)
                        check = lambda val: check_int(val, -(2**7), 2**7)
                    case ScalarKind.I16:
                        succ = lambda pred: succ_int(pred, -(2**15), 2**15)
                        check = lambda val: check_int(val, -(2**15), 2**15)
                    case ScalarKind.I32:
                        succ = lambda pred: succ_int(pred, -(2**31), 2**31)
                        check = lambda val: check_int(val, -(2**31), 2**31)
                    case ScalarKind.I64:
                        succ = lambda pred: succ_int(pred, -(2**63), 2**63)
                        check = lambda val: check_int(val, -(2**63), 2**63)
                    case ScalarKind.U8:
                        succ = lambda pred: succ_int(pred, 0, 2**8)
                        check = lambda val: check_int(val, 0, 2**8)
                    case ScalarKind.U16:
                        succ = lambda pred: succ_int(pred, 0, 2**16)
                        check = lambda val: check_int(val, 0, 2**16)
                    case ScalarKind.U32:
                        succ = lambda pred: succ_int(pred, 0, 2**32)
                        check = lambda val: check_int(val, 0, 2**32)
                    case ScalarKind.U64:
                        succ = lambda pred: succ_int(pred, 0, 2**64)
                        check = lambda val: check_int(val, 0, 2**64)
                    case ScalarKind.BOOL:
                        succ = lambda pred: False
                        check = lambda val: isinstance(val, bool)
                    case ScalarKind.F32:
                        succ = lambda pred: 0.0
                        check = lambda val: isinstance(val, float) and isfinite(val)
                    case ScalarKind.F64:
                        succ = lambda pred: 0.0
                        check = lambda val: isinstance(val, float) and isfinite(val)

                pred = None
                for item in d.items:
                    if item.value is None:
                        item.value = succ(pred)
                    elif not check(item.value):
                        self.dm.emit(EnumValueError(item, d))
                        item.value = succ(None)
                    pred = item.value

            case StringType():
                for item in d.items:
                    if item.value is None:
                        item.value = item.name
                    elif not isinstance(item.value, str):
                        self.dm.emit(EnumValueError(item, d))
                        item.value = item.name


class _CheckStructAndUnionNotEmptyPass(RecursiveDeclVisitor):
    """Checks that structs and unions are not empty."""

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm

    @override
    def visit_struct_decl(self, d: StructDecl) -> None:
        super().visit_struct_decl(d)
        if not d.fields:
            self.dm.emit(EmptyStructOrUnionError(d))

    @override
    def visit_union_decl(self, d: UnionDecl) -> None:
        super().visit_union_decl(d)
        if not d.fields:
            self.dm.emit(EmptyStructOrUnionError(d))


_V = TypeVar("_V")
_E = TypeVar("_E")
Graph = dict[_V, list[tuple[_E, _V]]]
Cycle = list[_E]


def _detect_cycles(graph: Graph[_V, _E]) -> list[Cycle[_E]]:
    """Detects and returns all cycles in a directed graph.

    Example:
    -------
    >>> graph = {
            "A": [("A.b_0", "B")],
            "B": [("B.c_0", "C")],
            "C": [("C.a_0", "A"), ("C.a_1", "A")],
        }
    >>> detect_cycles(graph)
    [["A.b_0", "B.c_0", "C.a_0"], ["A.b_0", "B.c_0", "C.a_1"]]
    """
    cycles: list[Cycle[_E]] = []

    order = {point: i for i, point in enumerate(graph)}
    glist = [
        [(edge, order[child]) for edge, child in children]
        for children in graph.values()
    ]
    visited = [False for _ in glist]
    cycle: Cycle[_E] = []

    def visit(i: int):
        if i < k:
            return
        if visited[i]:
            if i == k:
                cycles.append(cycle.copy())
            return
        visited[i] = True
        for edge, j in glist[i]:
            cycle.append(edge)
            visit(j)
            cycle.pop()
        visited[i] = False

    for k in range(len(glist)):
        visit(k)

    return cycles


class _CheckRecursiveInclusionPass(RecursiveDeclVisitor):
    """Validates struct fields for type correctness and cycles."""

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm
        self._type_table: Graph[TypeDecl, tuple[TypeDecl, TypeRefDecl]] | None = None

    @property
    def type_table(self) -> Graph[TypeDecl, tuple[TypeDecl, TypeRefDecl]]:
        """Returns the type table for the current package group."""
        pass
        return self._type_table

    @override
    def visit_package_group(self, g: PackageGroup) -> None:
        self._type_table = {}
        try:
            super().visit_package_group(g)
            cycles = _detect_cycles(self.type_table)
            for cycle in cycles:
                last, *other = cycle[::-1]
                self.dm.emit(RecursiveReferenceError(last, other))
        finally:
            self._type_table = None

    @override
    def visit_enum_decl(self, d: EnumDecl) -> None:
        self.type_table[d] = []

    @override
    def visit_iface_decl(self, d: IfaceDecl) -> None:
        extend_iface_list = self.type_table.setdefault(d, [])
        extend_iface_dict: dict[IfaceDecl, IfaceExtendDecl] = {}
        for extend in d.extends:
            if (extend_ty := extend.ty_or_none) is None:
                continue
            extend_iface = extend_ty.decl
            extend_iface_list.append(((d, extend.ty_ref), extend_iface))
            pred = extend_iface_dict.setdefault(extend_iface, extend)
            if pred != extend:
                self.dm.emit(DuplicateExtendsWarn(pred, extend, d, extend_iface))

    @override
    def visit_struct_decl(self, d: StructDecl) -> None:
        type_list = self.type_table.setdefault(d, [])
        for f in d.fields:
            if isinstance(ty := f.ty_or_none, UserType):
                type_list.append(((d, f.ty_ref), ty.decl))

    @override
    def visit_union_decl(self, d: UnionDecl) -> None:
        type_list = self.type_table.setdefault(d, [])
        for i in d.fields:
            if isinstance(ty := i.ty_or_none, UserType):
                type_list.append(((d, i.ty_ref), ty.decl))


class _ConvertAttrPass(RecursiveDeclVisitor):
    dm: DiagnosticsManager
    am: AttributeRegistry

    def __init__(self, dm: DiagnosticsManager, am: AttributeRegistry):
        self.dm = dm
        self.am = am

    @override
    def visit_decl(self, d: Decl) -> None:
        for unchecked_attr in UncheckedAttribute.consume(d):
            if (checked_attr := self.am.attach(unchecked_attr, self.dm)) is not None:
                d.add_attribute(checked_attr)


class _CheckAttrPass(RecursiveDeclVisitor):
    dm: DiagnosticsManager

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm

    @override
    def visit_decl(self, d: Decl) -> None:
        for attr in chain(*d.attributes.values()):
            attr.check_context(d, self.dm)