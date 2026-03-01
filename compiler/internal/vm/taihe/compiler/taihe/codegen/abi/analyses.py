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
from dataclasses import dataclass

from typing_extensions import override

from taihe.codegen.abi.mangle import DeclKind, encode
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


class PackageAbiInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.abi.h"
        self.source = f"{p.name}.abi.c"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageAbiInfo":
        return PackageAbiInfo(am, p)


class GlobFuncAbiInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        segments = [*f.parent_pkg.segments, f.name]
        self.impl_name = encode(segments, DeclKind.FUNC)

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncAbiInfo":
        return GlobFuncAbiInfo(am, f)


class IfaceMethodAbiInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        segments = [*f.parent_pkg.segments, f.parent_iface.name, f.name]
        self.impl_name = encode(segments, DeclKind.FUNC)
        self.wrap_name = encode(segments, DeclKind.METHOD)
        self.min_version = 0

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodAbiInfo":
        return IfaceMethodAbiInfo(am, f)


class EnumAbiInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        self.abi_type = "int"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: EnumDecl) -> "EnumAbiInfo":
        return EnumAbiInfo(am, d)


class UnionAbiInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.tag_type = "int"
        self.union_name = encode(segments, DeclKind.UNION)
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name} const*"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: UnionDecl) -> "UnionAbiInfo":
        return UnionAbiInfo(am, d)


class StructAbiInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name} const*"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: StructDecl) -> "StructAbiInfo":
        return StructAbiInfo(am, d)


@dataclass
class AncestorListItemInfo:
    iface: IfaceDecl
    ftbl_ptr: str


@dataclass
class AncestorDictItemInfo:
    offset: int
    static_cast: str
    ftbl_ptr: str


class IfaceAbiInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.version = 0
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name}"
        self.ftable = encode(segments, DeclKind.FTABLE)
        self.vtable = encode(segments, DeclKind.VTABLE)
        self.iid = encode(segments, DeclKind.IID)
        self.dynamic_cast = encode(segments, DeclKind.DYNAMIC_CAST)
        self.ancestor_list: list[AncestorListItemInfo] = []
        self.ancestor_dict: dict[IfaceDecl, AncestorDictItemInfo] = {}
        self.ancestors = [d]
        for extend in d.extends:
            extend_abi_info = IfaceAbiInfo.get(am, extend.ty.decl)
            self.ancestors.extend(extend_abi_info.ancestors)
        for i, ancestor in enumerate(self.ancestors):
            ftbl_ptr = f"ftbl_ptr_{i}"
            self.ancestor_list.append(
                AncestorListItemInfo(
                    iface=ancestor,
                    ftbl_ptr=ftbl_ptr,
                )
            )
            self.ancestor_dict.setdefault(
                ancestor,
                AncestorDictItemInfo(
                    offset=i,
                    static_cast=encode([*segments, str(i)], DeclKind.STATIC_CAST),
                    ftbl_ptr=ftbl_ptr,
                ),
            )

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceAbiInfo":
        return IfaceAbiInfo(am, d)


class TypeAbiInfo(AbstractAnalysis[NonVoidType], ABC):
    defn_headers: list[str]
    impl_headers: list[str]
    # type as struct field / union field / return value
    as_owner: str
    # type as parameter
    as_param: str

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, t: NonVoidType) -> "TypeAbiInfo":
        return t.accept(TypeAbiInfoDispatcher(am))


class EnumTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: EnumType) -> None:
        enum_abi_info = EnumAbiInfo.get(am, t.decl)
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = enum_abi_info.abi_type
        self.as_param = enum_abi_info.abi_type


class UnionTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        union_abi_info = UnionAbiInfo.get(am, t.decl)
        self.defn_headers = [union_abi_info.defn_header]
        self.impl_headers = [union_abi_info.impl_header]
        self.as_owner = union_abi_info.as_owner
        self.as_param = union_abi_info.as_param


class StructTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: StructType) -> None:
        struct_abi_info = StructAbiInfo.get(am, t.decl)
        self.defn_headers = [struct_abi_info.defn_header]
        self.impl_headers = [struct_abi_info.impl_header]
        self.as_owner = struct_abi_info.as_owner
        self.as_param = struct_abi_info.as_param


class IfaceTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType) -> None:
        iface_abi_info = IfaceAbiInfo.get(am, t.decl)
        self.defn_headers = [iface_abi_info.defn_header]
        self.impl_headers = [iface_abi_info.impl_header]
        self.as_owner = iface_abi_info.as_owner
        self.as_param = iface_abi_info.as_param


class UnitTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: UnitType) -> None:
        self.defn_headers = ["taihe/unit.abi.h"]
        self.impl_headers = ["taihe/unit.abi.h"]
        self.as_owner = "struct TUnit"
        self.as_param = "struct TUnit"


class ScalarTypeAbiInfo(TypeAbiInfo):
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
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = res
        self.as_owner = res


class OpaqueTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: StringType) -> None:
        self.defn_headers = ["taihe/string.abi.h"]
        self.impl_headers = ["taihe/string.abi.h"]
        self.as_owner = "struct TString"
        self.as_param = "struct TString"


class ArrayTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        self.defn_headers = ["taihe/array.abi.h"]
        self.impl_headers = ["taihe/array.abi.h"]
        self.as_owner = "struct TArray"
        self.as_param = "struct TArray"


class OptionalTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        self.defn_headers = ["taihe/optional.abi.h"]
        self.impl_headers = ["taihe/optional.abi.h"]
        self.as_owner = "struct TOptional"
        self.as_param = "struct TOptional"


class CallbackTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        self.defn_headers = ["taihe/callback.abi.h"]
        self.impl_headers = ["taihe/callback.abi.h"]
        self.as_owner = "struct TCallback"
        self.as_param = "struct TCallback"


class VectorTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        self.defn_headers = ["taihe/vector.abi.h"]
        self.impl_headers = ["taihe/vector.abi.h"]
        self.as_owner = "struct TVector"
        self.as_param = "struct TVector"


class MapTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        self.defn_headers = ["taihe/map.abi.h"]
        self.impl_headers = ["taihe/map.abi.h"]
        self.as_owner = "struct TMap"
        self.as_param = "struct TMap"


class SetTypeAbiInfo(TypeAbiInfo):
    def __init__(self, am: AnalysisManager, t: SetType) -> None:
        self.defn_headers = ["taihe/set.abi.h"]
        self.impl_headers = ["taihe/set.abi.h"]
        self.as_owner = "struct TSet"
        self.as_param = "struct TSet"


class TypeAbiInfoDispatcher(NonVoidTypeVisitor[TypeAbiInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @override
    def visit_enum_type(self, t: EnumType) -> TypeAbiInfo:
        return EnumTypeAbiInfo(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> TypeAbiInfo:
        return UnionTypeAbiInfo(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> TypeAbiInfo:
        return StructTypeAbiInfo(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> TypeAbiInfo:
        return IfaceTypeAbiInfo(self.am, t)

    @override
    def visit_unit_type(self, t: UnitType) -> TypeAbiInfo:
        return UnitTypeAbiInfo(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> TypeAbiInfo:
        return ScalarTypeAbiInfo(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> TypeAbiInfo:
        return OpaqueTypeAbiInfo(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> TypeAbiInfo:
        return StringTypeAbiInfo(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> TypeAbiInfo:
        return ArrayTypeAbiInfo(self.am, t)

    @override
    def visit_vector_type(self, t: VectorType) -> TypeAbiInfo:
        return VectorTypeAbiInfo(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> TypeAbiInfo:
        return OptionalTypeAbiInfo(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> TypeAbiInfo:
        return MapTypeAbiInfo(self.am, t)

    @override
    def visit_set_type(self, t: SetType) -> TypeAbiInfo:
        return SetTypeAbiInfo(self.am, t)

    @override
    def visit_callback_type(self, t: CallbackType) -> TypeAbiInfo:
        return CallbackTypeAbiInfo(self.am, t)


class PackageCImplInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.impl.h"
        self.source = f"{p.name}.impl.c"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageCImplInfo":
        return PackageCImplInfo(am, p)


class IfaceCImplInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        self.header = f"{d.parent_pkg.name}.{d.name}.default.h"
        self.source = f"{d.parent_pkg.name}.{d.name}.default.c"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceCImplInfo":
        return IfaceCImplInfo(am, d)


class GlobFuncCImplInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        self.macro = f"TH_EXPORT_C_API_{f.name}"
        self.function = f"{f.name}_impl"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncCImplInfo":
        return GlobFuncCImplInfo(am, f)


class IfaceMethodCImplInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        self.macro = f"TH_EXPORT_DEFAULT_C_API_{f.name}"
        self.function = f"{f.name}_default_impl"

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodCImplInfo":
        return IfaceMethodCImplInfo(am, f)