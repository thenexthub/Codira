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

from abc import ABC

from typing_extensions import override

from taihe.semantics.declarations import (
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    StructDecl,
    UnionDecl,
)
from taihe.semantics.types import (
    ArrayType,
    CallbackType,
    EnumType,
    IfaceType,
    MapType,
    NonVoidType,
    OpaqueType,
    OptionalType,
    ScalarKind,
    ScalarType,
    SetType,
    StringType,
    StructType,
    UnionType,
    UnitType,
    VectorType,
)
from taihe.semantics.visitor import NonVoidTypeVisitor
from taihe.utils.analyses import AbstractAnalysis, AnalysisManager


class PackageCppInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.proj.hpp"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageCppInfo":
        return PackageCppInfo(am, p)


class IfaceMethodCppInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        self.call_name = f.name
        self.impl_name = f.name

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodCppInfo":
        return IfaceMethodCppInfo(am, f)


class EnumCppInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: EnumDecl) -> "EnumCppInfo":
        return EnumCppInfo(am, d)


class StructCppInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.2.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name + " const&"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: StructDecl) -> "StructCppInfo":
        return StructCppInfo(am, d)


class UnionCppInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.2.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name + " const&"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: UnionDecl) -> "UnionCppInfo":
        return UnionCppInfo(am, d)


class IfaceCppInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.2.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.norm_name = d.name
        self.full_norm_name = "::" + self.namespace + "::" + self.norm_name

        self.weakspace = "::".join(d.parent_pkg.segments) + "::weak"
        self.weak_name = d.name
        self.full_weak_name = "::" + self.weakspace + "::" + self.weak_name

        self.as_owner = self.full_norm_name
        self.as_param = self.full_weak_name

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceCppInfo":
        return IfaceCppInfo(am, d)


class TypeCppInfo(AbstractAnalysis[NonVoidType], ABC):
    decl_headers: list[str]
    defn_headers: list[str]
    impl_headers: list[str]
    as_owner: str
    as_param: str

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, t: NonVoidType) -> "TypeCppInfo":
        return t.accept(TypeCppInfoDispatcher(am))


def into_abi(ty: str, val: str) -> str:
    return f"::taihe::into_abi<{ty}>({val})"


def from_abi(ty: str, val: str) -> str:
    return f"::taihe::from_abi<{ty}>({val})"


class EnumTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: EnumType):
        enum_cpp_info = EnumCppInfo.get(am, t.decl)
        self.decl_headers = [enum_cpp_info.decl_header]
        self.defn_headers = [enum_cpp_info.defn_header]
        self.impl_headers = [enum_cpp_info.defn_header]
        self.as_owner = enum_cpp_info.as_owner
        self.as_param = enum_cpp_info.as_param


class UnionTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        union_cpp_info = UnionCppInfo.get(am, t.decl)
        self.decl_headers = [union_cpp_info.decl_header]
        self.defn_headers = [union_cpp_info.defn_header]
        self.impl_headers = [union_cpp_info.impl_header]
        self.as_owner = union_cpp_info.as_owner
        self.as_param = union_cpp_info.as_param


class StructTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: StructType):
        struct_cpp_info = StructCppInfo.get(am, t.decl)
        self.decl_headers = [struct_cpp_info.decl_header]
        self.defn_headers = [struct_cpp_info.defn_header]
        self.impl_headers = [struct_cpp_info.impl_header]
        self.as_owner = struct_cpp_info.as_owner
        self.as_param = struct_cpp_info.as_param


class IfaceTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        iface_cpp_info = IfaceCppInfo.get(am, t.decl)
        self.decl_headers = [iface_cpp_info.decl_header]
        self.defn_headers = [iface_cpp_info.defn_header]
        self.impl_headers = [iface_cpp_info.impl_header]
        self.as_owner = iface_cpp_info.as_owner
        self.as_param = iface_cpp_info.as_param


class UnitTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: UnitType) -> None:
        self.decl_headers = ["taihe/unit.hpp"]
        self.defn_headers = ["taihe/unit.hpp"]
        self.impl_headers = ["taihe/unit.hpp"]
        self.as_owner = "::taihe::unit"
        self.as_param = "::taihe::unit"


class ScalarTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        res = {
            ScalarKind.BOOL: "bool",
            ScalarKind.F32: "float",
            ScalarKind.F64: "double",
            ScalarKind.I8: "int8_t",
            ScalarKind.I16: "int16_t",
            ScalarKind.I32: "int32_t",
            ScalarKind.I64: "int64_t",
            ScalarKind.U8: "uint8_t",
            ScalarKind.U16: "uint16_t",
            ScalarKind.U32: "uint32_t",
            ScalarKind.U64: "uint64_t",
        }[t.kind]
        self.decl_headers = []
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = res
        self.as_owner = res


class OpaqueTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        self.decl_headers = []
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: StringType):
        self.decl_headers = ["taihe/string.hpp"]
        self.defn_headers = ["taihe/string.hpp"]
        self.impl_headers = ["taihe/string.hpp"]
        self.as_owner = "::taihe::string"
        self.as_param = "::taihe::string_view"


class ArrayTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        arg_ty_cpp_info = TypeCppInfo.get(am, t.item_ty)
        self.decl_headers = ["taihe/array.hpp", *arg_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/array.hpp", *arg_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/array.hpp", *arg_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::array<{arg_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::array_view<{arg_ty_cpp_info.as_owner}>"


class OptionalTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        arg_ty_cpp_info = TypeCppInfo.get(am, t.item_ty)
        self.decl_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::optional<{arg_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::optional_view<{arg_ty_cpp_info.as_owner}>"


class VectorTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        val_ty_cpp_info = TypeCppInfo.get(am, t.val_ty)
        self.decl_headers = ["taihe/vector.hpp", *val_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/vector.hpp", *val_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/vector.hpp", *val_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::vector<{val_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::vector_view<{val_ty_cpp_info.as_owner}>"


class MapTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        key_ty_cpp_info = TypeCppInfo.get(am, t.key_ty)
        val_ty_cpp_info = TypeCppInfo.get(am, t.val_ty)
        self.decl_headers = [
            "taihe/map.hpp",
            *key_ty_cpp_info.decl_headers,
            *val_ty_cpp_info.decl_headers,
        ]
        self.defn_headers = [
            "taihe/map.hpp",
            *key_ty_cpp_info.decl_headers,
            *val_ty_cpp_info.decl_headers,
        ]
        self.impl_headers = [
            "taihe/map.hpp",
            *key_ty_cpp_info.impl_headers,
            *val_ty_cpp_info.impl_headers,
        ]
        self.as_owner = (
            f"::taihe::map<{key_ty_cpp_info.as_owner}, {val_ty_cpp_info.as_owner}>"
        )
        self.as_param = (
            f"::taihe::map_view<{key_ty_cpp_info.as_owner}, {val_ty_cpp_info.as_owner}>"
        )


class SetTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: SetType) -> None:
        key_ty_cpp_info = TypeCppInfo.get(am, t.key_ty)
        self.decl_headers = ["taihe/set.hpp", *key_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/set.hpp", *key_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/set.hpp", *key_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::set<{key_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::set_view<{key_ty_cpp_info.as_owner}>"


class CallbackTypeCppInfo(TypeCppInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        if isinstance(return_ty := t.ref.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(am, return_ty)
            return_ty_decl_headers = return_ty_cpp_info.decl_headers
            return_ty_impl_headers = return_ty_cpp_info.impl_headers
            return_ty_as_owner = return_ty_cpp_info.as_owner
        else:
            return_ty_decl_headers = []
            return_ty_impl_headers = []
            return_ty_as_owner = "void"
        params_ty_decl_headers = []
        params_ty_impl_headers = []
        params_ty_as_param = []
        for param in t.ref.params:
            param_ty_cpp_info = TypeCppInfo.get(am, param.ty)
            params_ty_decl_headers.extend(param_ty_cpp_info.decl_headers)
            params_ty_impl_headers.extend(param_ty_cpp_info.impl_headers)
            params_ty_as_param.append(f"{param_ty_cpp_info.as_param} {param.name}")
        params_fmt = ", ".join(params_ty_as_param)
        self.decl_headers = [
            "taihe/callback.hpp",
            *return_ty_decl_headers,
            *params_ty_decl_headers,
        ]
        self.defn_headers = [
            "taihe/callback.hpp",
            *return_ty_decl_headers,
            *params_ty_decl_headers,
        ]
        self.impl_headers = [
            "taihe/callback.hpp",
            *return_ty_impl_headers,
            *params_ty_impl_headers,
        ]
        self.as_owner = f"::taihe::callback<{return_ty_as_owner}({params_fmt})>"
        self.as_param = f"::taihe::callback_view<{return_ty_as_owner}({params_fmt})>"


class TypeCppInfoDispatcher(NonVoidTypeVisitor[TypeCppInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @override
    def visit_enum_type(self, t: EnumType) -> TypeCppInfo:
        return EnumTypeCppInfo(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> TypeCppInfo:
        return UnionTypeCppInfo(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> TypeCppInfo:
        return StructTypeCppInfo(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> TypeCppInfo:
        return IfaceTypeCppInfo(self.am, t)

    @override
    def visit_unit_type(self, t: UnitType) -> TypeCppInfo:
        return UnitTypeCppInfo(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> TypeCppInfo:
        return ScalarTypeCppInfo(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> TypeCppInfo:
        return OpaqueTypeCppInfo(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> TypeCppInfo:
        return StringTypeCppInfo(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> TypeCppInfo:
        return ArrayTypeCppInfo(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> TypeCppInfo:
        return OptionalTypeCppInfo(self.am, t)

    @override
    def visit_vector_type(self, t: VectorType) -> TypeCppInfo:
        return VectorTypeCppInfo(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> TypeCppInfo:
        return MapTypeCppInfo(self.am, t)

    @override
    def visit_set_type(self, t: SetType) -> TypeCppInfo:
        return SetTypeCppInfo(self.am, t)

    @override
    def visit_callback_type(self, t: CallbackType) -> TypeCppInfo:
        return CallbackTypeCppInfo(self.am, t)


class PackageCppUserInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.user.hpp"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageCppUserInfo":
        return PackageCppUserInfo(am, p)


class GlobFuncCppUserInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        self.namespace = "::".join(f.parent_pkg.segments)
        self.call_name = f.name
        self.full_name = "::" + self.namespace + "::" + self.call_name

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncCppUserInfo":
        return GlobFuncCppUserInfo(am, f)


class PackageCppImplInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.impl.hpp"
        self.source = f"{p.name}.impl.cpp"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageCppImplInfo":
        return PackageCppImplInfo(am, p)


class IfaceCppImplInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        self.header = f"{d.parent_pkg.name}.{d.name}.default.hpp"
        self.source = f"{d.parent_pkg.name}.{d.name}.default.cpp"
        self.template_header = f"{d.parent_pkg.name}.{d.name}.template.hpp"
        self.template_source = f"{d.parent_pkg.name}.{d.name}.template.cpp"
        self.template_class = f"{d.name}Impl"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceCppImplInfo":
        return IfaceCppImplInfo(am, d)


class GlobFuncCppImplInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        self.macro = f"TH_EXPORT_CPP_API_{f.name}"
        self.function = f.name

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncCppImplInfo":
        return GlobFuncCppImplInfo(am, f)


class IfaceMethodCppImplInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        self.macro = f"TH_EXPORT_DEFAULT_CPP_API_{f.name}"
        self.function = f.name

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodCppImplInfo":  # fmt: skip
        return IfaceMethodCppImplInfo(am, f)