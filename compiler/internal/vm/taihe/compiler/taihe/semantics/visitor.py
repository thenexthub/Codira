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

"""Implements the classic visitor pattern for core types.

In most cases, you need to call `node.accept(visitor)` to visit a node.
"""

from typing import TYPE_CHECKING, Generic, TypeVar

from typing_extensions import override

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        CallbackTypeRefDecl,
        Decl,
        DeclarationImportDecl,
        DeclarationRefDecl,
        EnumDecl,
        EnumItemDecl,
        ExplicitTypeRefDecl,
        GenericArgDecl,
        GenericTypeRefDecl,
        GlobFuncDecl,
        IfaceDecl,
        IfaceExtendDecl,
        IfaceMethodDecl,
        ImplicitTypeRefDecl,
        ImportDecl,
        LongTypeRefDecl,
        PackageDecl,
        PackageGroup,
        PackageImportDecl,
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
        ArrayType,
        BuiltinType,
        CallbackType,
        EnumType,
        GenericType,
        IfaceType,
        MapType,
        NonVoidType,
        OpaqueType,
        OptionalType,
        ScalarType,
        SetType,
        StringType,
        StructType,
        Type,
        UnionType,
        UnitType,
        UserType,
        VectorType,
        VoidType,
    )


_R = TypeVar("_R")


class UnitTypeVisitor(Generic[_R]):
    def visit_unit_type(self, t: "UnitType") -> _R:
        raise NotImplementedError


class ScalarTypeVisitor(Generic[_R]):
    def visit_scalar_type(self, t: "ScalarType") -> _R:
        raise NotImplementedError


class StringTypeVisitor(Generic[_R]):
    def visit_string_type(self, t: "StringType") -> _R:
        raise NotImplementedError


class OpaqueTypeVisitor(Generic[_R]):
    def visit_opaque_type(self, t: "OpaqueType") -> _R:
        raise NotImplementedError


class BuiltinTypeVisitor(
    Generic[_R],
    ScalarTypeVisitor[_R],
    StringTypeVisitor[_R],
    OpaqueTypeVisitor[_R],
    UnitTypeVisitor[_R],
):
    def visit_builtin_type(self, t: "BuiltinType") -> _R:
        raise NotImplementedError

    @override
    def visit_scalar_type(self, t: "ScalarType") -> _R:
        return self.visit_builtin_type(t)

    @override
    def visit_string_type(self, t: "StringType") -> _R:
        return self.visit_builtin_type(t)

    @override
    def visit_opaque_type(self, t: "OpaqueType") -> _R:
        return self.visit_builtin_type(t)

    @override
    def visit_unit_type(self, t: "UnitType") -> _R:
        return self.visit_builtin_type(t)


class OptionalTypeVisitor(Generic[_R]):
    def visit_optional_type(self, t: "OptionalType") -> _R:
        raise NotImplementedError


class ArrayTypeVisitor(Generic[_R]):
    def visit_array_type(self, t: "ArrayType") -> _R:
        raise NotImplementedError


class VectorTypeVisitor(Generic[_R]):
    def visit_vector_type(self, t: "VectorType") -> _R:
        raise NotImplementedError


class MapTypeVisitor(Generic[_R]):
    def visit_map_type(self, t: "MapType") -> _R:
        raise NotImplementedError


class SetTypeVisitor(Generic[_R]):
    def visit_set_type(self, t: "SetType") -> _R:
        raise NotImplementedError


class GenericTypeVisitor(
    Generic[_R],
    OptionalTypeVisitor[_R],
    ArrayTypeVisitor[_R],
    VectorTypeVisitor[_R],
    MapTypeVisitor[_R],
    SetTypeVisitor[_R],
):
    def visit_generic_type(self, t: "GenericType") -> _R:
        raise NotImplementedError

    @override
    def visit_optional_type(self, t: "OptionalType") -> _R:
        return self.visit_generic_type(t)

    @override
    def visit_array_type(self, t: "ArrayType") -> _R:
        return self.visit_generic_type(t)

    @override
    def visit_vector_type(self, t: "VectorType") -> _R:
        return self.visit_generic_type(t)

    @override
    def visit_map_type(self, t: "MapType") -> _R:
        return self.visit_generic_type(t)

    @override
    def visit_set_type(self, t: "SetType") -> _R:
        return self.visit_generic_type(t)


class CallbackTypeVisitor(Generic[_R]):
    def visit_callback_type(self, t: "CallbackType") -> _R:
        raise NotImplementedError


class EnumTypeVisitor(Generic[_R]):
    def visit_enum_type(self, t: "EnumType") -> _R:
        raise NotImplementedError


class StructTypeVisitor(Generic[_R]):
    def visit_struct_type(self, t: "StructType") -> _R:
        raise NotImplementedError


class UnionTypeVisitor(Generic[_R]):
    def visit_union_type(self, t: "UnionType") -> _R:
        raise NotImplementedError


class IfaceTypeVisitor(Generic[_R]):
    def visit_iface_type(self, t: "IfaceType") -> _R:
        raise NotImplementedError


class UserTypeVisitor(
    Generic[_R],
    EnumTypeVisitor[_R],
    StructTypeVisitor[_R],
    UnionTypeVisitor[_R],
    IfaceTypeVisitor[_R],
):
    def visit_user_type(self, t: "UserType") -> _R:
        raise NotImplementedError

    @override
    def visit_enum_type(self, t: "EnumType") -> _R:
        return self.visit_user_type(t)

    @override
    def visit_struct_type(self, t: "StructType") -> _R:
        return self.visit_user_type(t)

    @override
    def visit_union_type(self, t: "UnionType") -> _R:
        return self.visit_user_type(t)

    @override
    def visit_iface_type(self, t: "IfaceType") -> _R:
        return self.visit_user_type(t)


class NonVoidTypeVisitor(
    Generic[_R],
    BuiltinTypeVisitor[_R],
    GenericTypeVisitor[_R],
    CallbackTypeVisitor[_R],
    UserTypeVisitor[_R],
):
    def visit_non_void_type(self, t: "NonVoidType") -> _R:
        raise NotImplementedError

    @override
    def visit_opaque_type(self, t: "OpaqueType") -> _R:
        return self.visit_non_void_type(t)

    @override
    def visit_builtin_type(self, t: "BuiltinType") -> _R:
        return self.visit_non_void_type(t)

    @override
    def visit_generic_type(self, t: "GenericType") -> _R:
        return self.visit_non_void_type(t)

    @override
    def visit_callback_type(self, t: "CallbackType") -> _R:
        return self.visit_non_void_type(t)

    @override
    def visit_user_type(self, t: "UserType") -> _R:
        return self.visit_non_void_type(t)


class VoidTypeVisitor(Generic[_R]):
    def visit_void_type(self, t: "VoidType") -> _R:
        raise NotImplementedError


class TypeVisitor(
    Generic[_R],
    NonVoidTypeVisitor[_R],
    VoidTypeVisitor[_R],
):
    def visit_type(self, t: "Type") -> _R:
        raise NotImplementedError

    @override
    def visit_non_void_type(self, t: "NonVoidType") -> _R:
        return self.visit_type(t)

    @override
    def visit_void_type(self, t: "VoidType") -> _R:
        return self.visit_type(t)


class GenericArgVisitor(Generic[_R]):
    def visit_generic_arg(self, d: "GenericArgDecl") -> _R:
        raise NotImplementedError


class ParamVisitor(Generic[_R]):
    def visit_param(self, d: "ParamDecl") -> _R:
        raise NotImplementedError


class ShortTypeRefVisitor(Generic[_R]):
    def visit_short_type_ref(self, d: "ShortTypeRefDecl") -> _R:
        raise NotImplementedError


class LongTypeRefVisitor(Generic[_R]):
    def visit_long_type_ref(self, d: "LongTypeRefDecl") -> _R:
        raise NotImplementedError


class GenericTypeRefVisitor(Generic[_R]):
    def visit_generic_type_ref(self, d: "GenericTypeRefDecl") -> _R:
        raise NotImplementedError


class CallbackTypeRefVisitor(Generic[_R]):
    def visit_callback_type_ref(self, d: "CallbackTypeRefDecl") -> _R:
        raise NotImplementedError


class ExplicitTypeRefVisitor(
    Generic[_R],
    ShortTypeRefVisitor[_R],
    LongTypeRefVisitor[_R],
    GenericTypeRefVisitor[_R],
    CallbackTypeRefVisitor[_R],
):
    def visit_explicit_type_ref(self, d: "ExplicitTypeRefDecl") -> _R:
        raise NotImplementedError

    @override
    def visit_short_type_ref(self, d: "ShortTypeRefDecl") -> _R:
        return self.visit_explicit_type_ref(d)

    @override
    def visit_long_type_ref(self, d: "LongTypeRefDecl") -> _R:
        return self.visit_explicit_type_ref(d)

    @override
    def visit_generic_type_ref(self, d: "GenericTypeRefDecl") -> _R:
        return self.visit_explicit_type_ref(d)

    @override
    def visit_callback_type_ref(self, d: "CallbackTypeRefDecl") -> _R:
        return self.visit_explicit_type_ref(d)


class ImplicitTypeRefVisitor(Generic[_R]):
    def visit_implicit_type_ref(self, d: "ImplicitTypeRefDecl") -> _R:
        raise NotImplementedError


class TypeRefVisitor(
    Generic[_R],
    ExplicitTypeRefVisitor[_R],
    ImplicitTypeRefVisitor[_R],
):
    def visit_type_ref(self, d: "TypeRefDecl") -> _R:
        raise NotImplementedError

    @override
    def visit_explicit_type_ref(self, d: "ExplicitTypeRefDecl") -> _R:
        return self.visit_type_ref(d)

    @override
    def visit_implicit_type_ref(self, d: "ImplicitTypeRefDecl") -> _R:
        return self.visit_type_ref(d)


class PackageRefVisitor(Generic[_R]):
    def visit_package_ref(self, d: "PackageRefDecl") -> _R:
        raise NotImplementedError


class DeclarationRefVisitor(Generic[_R]):
    def visit_declaration_ref(self, d: "DeclarationRefDecl") -> _R:
        raise NotImplementedError


class PackageImportVisitor(Generic[_R]):
    def visit_package_import(self, d: "PackageImportDecl") -> _R:
        raise NotImplementedError


class DeclarationImportVisitor(Generic[_R]):
    def visit_declaration_import(self, d: "DeclarationImportDecl") -> _R:
        raise NotImplementedError


class ImportVisitor(
    Generic[_R],
    PackageImportVisitor[_R],
    DeclarationImportVisitor[_R],
):
    def visit_import(self, d: "ImportDecl") -> _R:
        raise NotImplementedError

    @override
    def visit_package_import(self, d: "PackageImportDecl") -> _R:
        return self.visit_import(d)

    @override
    def visit_declaration_import(self, d: "DeclarationImportDecl") -> _R:
        return self.visit_import(d)


class EnumItemVisitor(Generic[_R]):
    def visit_enum_item(self, d: "EnumItemDecl") -> _R:
        raise NotImplementedError


class StructFieldVisitor(Generic[_R]):
    def visit_struct_field(self, d: "StructFieldDecl") -> _R:
        raise NotImplementedError


class UnionFieldVisitor(Generic[_R]):
    def visit_union_field(self, d: "UnionFieldDecl") -> _R:
        raise NotImplementedError


class IfaceExtendVisitor(Generic[_R]):
    def visit_iface_extend(self, d: "IfaceExtendDecl") -> _R:
        raise NotImplementedError


class IfaceMethodVisitor(Generic[_R]):
    def visit_iface_method(self, d: "IfaceMethodDecl") -> _R:
        raise NotImplementedError


class EnumDeclVisitor(Generic[_R]):
    def visit_enum_decl(self, d: "EnumDecl") -> _R:
        raise NotImplementedError


class StructDeclVisitor(Generic[_R]):
    def visit_struct_decl(self, d: "StructDecl") -> _R:
        raise NotImplementedError


class UnionDeclVisitor(Generic[_R]):
    def visit_union_decl(self, d: "UnionDecl") -> _R:
        raise NotImplementedError


class IfaceDeclVisitor(Generic[_R]):
    def visit_iface_decl(self, d: "IfaceDecl") -> _R:
        raise NotImplementedError


class TypeDeclVisitor(
    Generic[_R],
    EnumDeclVisitor[_R],
    StructDeclVisitor[_R],
    UnionDeclVisitor[_R],
    IfaceDeclVisitor[_R],
):
    def visit_type_decl(self, d: "TypeDecl") -> _R:
        raise NotImplementedError

    @override
    def visit_enum_decl(self, d: "EnumDecl") -> _R:
        return self.visit_type_decl(d)

    @override
    def visit_struct_decl(self, d: "StructDecl") -> _R:
        return self.visit_type_decl(d)

    @override
    def visit_union_decl(self, d: "UnionDecl") -> _R:
        return self.visit_type_decl(d)

    @override
    def visit_iface_decl(self, d: "IfaceDecl") -> _R:
        return self.visit_type_decl(d)


class GlobFuncVisitor(Generic[_R]):
    def visit_glob_func(self, d: "GlobFuncDecl") -> _R:
        raise NotImplementedError


class PackageLevelVisitor(
    Generic[_R],
    GlobFuncVisitor[_R],
    TypeDeclVisitor[_R],
):
    def visit_package_level(self, d: "PackageLevelDecl") -> _R:
        raise NotImplementedError

    @override
    def visit_glob_func(self, d: "GlobFuncDecl") -> _R:
        return self.visit_package_level(d)

    @override
    def visit_type_decl(self, d: "TypeDecl") -> _R:
        return self.visit_package_level(d)


class PackageVisitor(Generic[_R]):
    def visit_package(self, p: "PackageDecl") -> _R:
        raise NotImplementedError


class PackageGroupVisitor(Generic[_R]):
    def visit_package_group(self, g: "PackageGroup") -> _R:
        raise NotImplementedError


class DeclVisitor(
    Generic[_R],
    GenericArgVisitor[_R],
    ParamVisitor[_R],
    TypeRefVisitor[_R],
    PackageRefVisitor[_R],
    DeclarationRefVisitor[_R],
    ImportVisitor[_R],
    EnumItemVisitor[_R],
    StructFieldVisitor[_R],
    UnionFieldVisitor[_R],
    IfaceExtendVisitor[_R],
    IfaceMethodVisitor[_R],
    PackageLevelVisitor[_R],
    PackageVisitor[_R],
    PackageGroupVisitor[_R],
):
    def visit_decl(self, d: "Decl") -> _R:
        raise NotImplementedError

    @override
    def visit_generic_arg(self, d: "GenericArgDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_param(self, d: "ParamDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_type_ref(self, d: "TypeRefDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_package_ref(self, d: "PackageRefDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_declaration_ref(self, d: "DeclarationRefDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_import(self, d: "ImportDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_enum_item(self, d: "EnumItemDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_struct_field(self, d: "StructFieldDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_union_field(self, d: "UnionFieldDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_iface_extend(self, d: "IfaceExtendDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_iface_method(self, d: "IfaceMethodDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_package_level(self, d: "PackageLevelDecl") -> _R:
        return self.visit_decl(d)

    @override
    def visit_package(self, p: "PackageDecl") -> _R:
        return self.visit_decl(p)

    @override
    def visit_package_group(self, g: "PackageGroup") -> _R:
        raise NotImplementedError


class RecursiveDeclVisitor(DeclVisitor[None]):
    """A visitor that recursively traverses all declarations and their sub-declarations.

    This class is useful for full-tree traversal scenarios.
    """

    @override
    def visit_decl(self, d: "Decl") -> None:
        pass

    @override
    def visit_generic_arg(self, d: "GenericArgDecl") -> None:
        d.ty_ref.accept(self)
        super().visit_generic_arg(d)

    @override
    def visit_param(self, d: "ParamDecl") -> None:
        d.ty_ref.accept(self)
        super().visit_param(d)

    @override
    def visit_implicit_type_ref(self, d: "ImplicitTypeRefDecl") -> None:
        super().visit_implicit_type_ref(d)

    @override
    def visit_short_type_ref(self, d: "ShortTypeRefDecl") -> None:
        super().visit_short_type_ref(d)

    @override
    def visit_long_type_ref(self, d: "LongTypeRefDecl") -> None:
        super().visit_long_type_ref(d)

    @override
    def visit_generic_type_ref(self, d: "GenericTypeRefDecl") -> None:
        for i in d.args:
            i.accept(self)
        super().visit_generic_type_ref(d)

    @override
    def visit_callback_type_ref(self, d: "CallbackTypeRefDecl") -> None:
        for i in d.params:
            i.accept(self)
        d.return_ty_ref.accept(self)
        super().visit_callback_type_ref(d)

    @override
    def visit_package_ref(self, d: "PackageRefDecl") -> None:
        super().visit_package_ref(d)

    @override
    def visit_declaration_ref(self, d: "DeclarationRefDecl") -> None:
        d.pkg_ref.accept(self)
        super().visit_declaration_ref(d)

    @override
    def visit_package_import(self, d: "PackageImportDecl") -> None:
        d.pkg_ref.accept(self)
        super().visit_package_import(d)

    @override
    def visit_declaration_import(self, d: "DeclarationImportDecl") -> None:
        d.decl_ref.accept(self)
        super().visit_declaration_import(d)

    @override
    def visit_enum_item(self, d: "EnumItemDecl") -> None:
        super().visit_enum_item(d)

    @override
    def visit_struct_field(self, d: "StructFieldDecl") -> None:
        d.ty_ref.accept(self)
        super().visit_struct_field(d)

    @override
    def visit_union_field(self, d: "UnionFieldDecl") -> None:
        d.ty_ref.accept(self)
        super().visit_union_field(d)

    @override
    def visit_iface_extend(self, d: "IfaceExtendDecl") -> None:
        d.ty_ref.accept(self)
        super().visit_iface_extend(d)

    @override
    def visit_iface_method(self, d: "IfaceMethodDecl") -> None:
        for i in d.params:
            i.accept(self)
        d.return_ty_ref.accept(self)
        super().visit_iface_method(d)

    @override
    def visit_enum_decl(self, d: "EnumDecl") -> None:
        d.ty_ref.accept(self)
        for i in d.items:
            i.accept(self)
        super().visit_enum_decl(d)

    @override
    def visit_union_decl(self, d: "UnionDecl") -> None:
        for i in d.fields:
            i.accept(self)
        super().visit_union_decl(d)

    @override
    def visit_struct_decl(self, d: "StructDecl") -> None:
        for i in d.fields:
            i.accept(self)
        super().visit_struct_decl(d)

    @override
    def visit_iface_decl(self, d: "IfaceDecl") -> None:
        for i in d.extends:
            i.accept(self)
        for i in d.methods:
            i.accept(self)
        super().visit_iface_decl(d)

    @override
    def visit_glob_func(self, d: "GlobFuncDecl") -> None:
        for i in d.params:
            i.accept(self)
        d.return_ty_ref.accept(self)
        super().visit_glob_func(d)

    @override
    def visit_package(self, p: "PackageDecl") -> None:
        for i in p.pkg_imports:
            i.accept(self)
        for i in p.decl_imports:
            i.accept(self)
        for i in p.declarations:
            i.accept(self)
        super().visit_package(p)

    @override
    def visit_package_group(self, g: "PackageGroup") -> None:
        for i in g.all_packages:
            i.accept(self)