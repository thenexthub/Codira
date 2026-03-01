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

from abc import ABC, abstractmethod
from collections.abc import Iterable
from dataclasses import dataclass
from dataclasses import field as datafield
from json import dumps
from typing import ClassVar

from typing_extensions import override

from taihe.codegen.abi.writer import CSourceWriter
from taihe.codegen.ani.attributes import (
    ArrayBufferAttr,
    AsyncAttribute,
    BigIntAttr,
    ClassAttr,
    ConstAttr,
    ConstructorAttribute,
    CtorAttr,
    ExportDefaultAttr,
    ExtendsAttr,
    FixedArrayAttr,
    GenAsyncAttr,
    GenPromiseAttr,
    GetAttr,
    LiteralAttr,
    NamespaceAttr,
    NullAttr,
    OnOffAttr,
    OptionalAttr,
    OverloadAttr,
    PromiseAttribute,
    RecordAttr,
    RenameAttr,
    SetAttr,
    StaticAttr,
    StaticOverloadAttribute,
    StsFillAttr,
    StsInjectAttr,
    StsInjectIntoClazzAttr,
    StsInjectIntoIfaceAttr,
    StsInjectIntoModuleAttr,
    StsLastAttr,
    StsThisAttr,
    StsTypeAttr,
    TypedArrayAttr,
    UndefinedAttr,
)
from taihe.codegen.ani.writer import DefaultNaming, KeepNaming, StsWriter
from taihe.codegen.cpp.analyses import (
    EnumCppInfo,
    IfaceCppInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.semantics.declarations import (
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceExtendDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
    ParamDecl,
    StructDecl,
    StructFieldDecl,
    UnionDecl,
    UnionFieldDecl,
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

# Ani Runtime Types


@dataclass
class AniRuntimeType(ABC):
    @property
    @abstractmethod
    def sig(self) -> str:
        ...

    @property
    @abstractmethod
    def boxed(self) -> "AniRuntimeNonPrimitiveType":
        ...


@dataclass
class AniRuntimePrimitiveType(AniRuntimeType):
    _boxed: "AniRuntimeNonPrimitiveType"
    _sig: str

    @property
    def sig(self) -> str:
        return self._sig

    @property
    def boxed(self) -> "AniRuntimeNonPrimitiveType":
        return self._boxed


@dataclass
class AniRuntimeNonPrimitiveType(AniRuntimeType, ABC):
    @property
    @abstractmethod
    def desc(self) -> str:
        ...

    @property
    def boxed(self) -> "AniRuntimeNonPrimitiveType":
        return self

    @abstractmethod
    def as_union_members(self) -> Iterable["AniRuntimeUnionMemberType"]:
        ...


@dataclass
class AniRuntimeUnionMemberType(AniRuntimeNonPrimitiveType, ABC):
    def as_union_members(self) -> Iterable["AniRuntimeUnionMemberType"]:
        yield self


@dataclass
class AniRuntimeUnionType(AniRuntimeNonPrimitiveType):
    members: list["AniRuntimeUnionMemberType"]

    @property
    def sig(self) -> str:
        signatures = [union_member.sig for union_member in self.members]
        signatures = sorted(set(signatures))
        signatures_str = "".join(signatures)
        return f"X{{{signatures_str}}}"

    @property
    def desc(self) -> str:
        return self.sig

    @staticmethod
    def union(*sig_types: AniRuntimeType) -> AniRuntimeNonPrimitiveType:
        members = [
            member
            for sig_type in sig_types
            for member in sig_type.boxed.as_union_members()
        ]
        if len(members) == 0:
            return AniRuntimeUndefinedType()
        if len(members) == 1:
            return members[0]
        return AniRuntimeUnionType(members=members)

    def as_union_members(self) -> Iterable["AniRuntimeUnionMemberType"]:
        yield from self.members


@dataclass
class AniRuntimeUndefinedType(AniRuntimeNonPrimitiveType):
    @property
    def sig(self) -> str:
        return "U"

    @property
    def desc(self) -> str:
        return "std.core.Object"

    def as_union_members(self) -> Iterable["AniRuntimeUnionMemberType"]:
        yield from ()


@dataclass
class AniRuntimeFixedArrayType(AniRuntimeUnionMemberType):
    element: AniRuntimeType

    @property
    def sig(self) -> str:
        return f"A{{{self.element.sig}}}"

    @property
    def desc(self) -> str:
        return self.sig


@dataclass
class AniRuntimeClassType(AniRuntimeUnionMemberType):
    name: str

    @property
    def sig(self) -> str:
        return f"C{{{self.name}}}"

    @property
    def desc(self) -> str:
        return self.name


@dataclass
class AniRuntimeEnumType(AniRuntimeUnionMemberType):
    name: str

    @property
    def sig(self) -> str:
        return f"E{{{self.name}}}"

    @property
    def desc(self) -> str:
        return self.name


# Ani Types


@dataclass(repr=False)
class AniType:
    hint: str
    base: "AniBaseType"

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.base.hint.capitalize()

    @property
    def fixedarray(self) -> "AniFixedArrayType":
        return self.base.fixedarray_hint


@dataclass(repr=False)
class AniFixedArrayType(AniType):
    item: "AniBaseType"

    def __init__(self, hint: str, item: "AniBaseType"):
        super().__init__(hint, ANI_REF)
        self.item = item
        self.item.fixedarray_hint = self


@dataclass(repr=False)
class AniBaseType(AniType):
    fixedarray_hint: "AniFixedArrayType"

    def __init__(self, hint: str):
        super().__init__(hint, self)


ANI_REF = AniBaseType(hint="ref")
ANI_FIXEDARRAY_REF = AniFixedArrayType(hint="fixedarray_ref", item=ANI_REF)

ANI_BOOLEAN = AniBaseType(hint="boolean")
ANI_FIXEDARRAY_BOOLEAN = AniFixedArrayType(hint="fixedarray_boolean", item=ANI_BOOLEAN)

ANI_FLOAT = AniBaseType(hint="float")
ANI_FIXEDARRAY_FLOAT = AniFixedArrayType(hint="fixedarray_float", item=ANI_FLOAT)

ANI_DOUBLE = AniBaseType(hint="double")
ANI_FIXEDARRAY_DOUBLE = AniFixedArrayType(hint="fixedarray_double", item=ANI_DOUBLE)

ANI_BYTE = AniBaseType(hint="byte")
ANI_FIXEDARRAY_BYTE = AniFixedArrayType(hint="fixedarray_byte", item=ANI_BYTE)

ANI_SHORT = AniBaseType(hint="short")
ANI_FIXEDARRAY_SHORT = AniFixedArrayType(hint="fixedarray_short", item=ANI_SHORT)

ANI_INT = AniBaseType(hint="int")
ANI_FIXEDARRAY_INT = AniFixedArrayType(hint="fixedarray_int", item=ANI_INT)

ANI_LONG = AniBaseType(hint="long")
ANI_FIXEDARRAY_LONG = AniFixedArrayType(hint="fixedarray_long", item=ANI_LONG)

ANI_OBJECT = AniType(hint="object", base=ANI_REF)
ANI_ARRAY = AniType(hint="array", base=ANI_REF)
ANI_FN_OBJECT = AniType(hint="fn_object", base=ANI_REF)
ANI_ENUM_ITEM = AniType(hint="enum_item", base=ANI_REF)
ANI_STRING = AniType(hint="string", base=ANI_REF)
ANI_ARRAYBUFFER = AniType(hint="arraybuffer", base=ANI_REF)


# Ani Function and Method


@dataclass(repr=False)
class AniFuncLike:
    hint: str

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.hint.capitalize()

    @property
    def upper(self) -> str:
        return self.hint.upper()


ANI_FUNCTION = AniFuncLike("function")
ANI_METHOD = AniFuncLike("method")


# Ani Scopes


@dataclass(repr=False)
class AniScope:
    hint: str
    member: AniFuncLike

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.hint.capitalize()

    @property
    def upper(self) -> str:
        return self.hint.upper()


ANI_CLASS = AniScope("class", ANI_METHOD)
ANI_MODULE = AniScope("module", ANI_FUNCTION)
ANI_NAMESPACE = AniScope("namespace", ANI_FUNCTION)


# ArkTs Module and Namespace


@dataclass
class ArkTsOutDir:
    bundle_str: str | None = None
    prefix_str: str | None = None

    @property
    def bundle_parts(self) -> list[str]:
        return [self.bundle_str] if self.bundle_str else []

    @property
    def prefix_parts(self) -> list[str]:
        return [] if self.prefix_str is None else self.prefix_str.split("/")


@dataclass
class ArkTsModuleOrNamespace(ABC):
    scope: ClassVar[AniScope]

    is_default: bool = datafield(default=False, init=False)
    injected_codes: list[str] = datafield(default_factory=list, init=False)
    injected_heads: list[str] = datafield(default_factory=list, init=False)
    packages: list[PackageDecl] = datafield(default_factory=list, init=False)
    children: dict[str, "ArkTsNamespace"] = datafield(default_factory=dict, init=False)

    @property
    @abstractmethod
    def mod(self) -> "ArkTsModule":
        pass

    @property
    @abstractmethod
    def impl_desc(self) -> str:
        pass

    @abstractmethod
    def get_member(self, name: str, is_default: bool, target: StsWriter) -> str:
        pass

    def add_path(
        self,
        ns_parts: list[str],
        pkg: PackageDecl,
        is_default: bool,
    ) -> "ArkTsModuleOrNamespace":
        if not ns_parts:
            self.packages.append(pkg)
            self.is_default |= is_default
            return self
        head, *tail = ns_parts
        child = self.children.setdefault(head, ArkTsNamespace(self, head))
        return child.add_path(tail, pkg, is_default)


@dataclass
class ArkTsModule(ArkTsModuleOrNamespace):
    parent: ArkTsOutDir
    module_name: str

    scope: ClassVar[AniScope] = ANI_MODULE

    obj_drop = "_taihe_objDrop"
    obj_dup = "_taihe_objDup"
    registry = "_taihe_registry"
    native_invoke = "_taihe_nativeInvoke"
    make_callback = "_taihe_makeCallback"
    bigint_to_arrbuf = "_taihe_fromBigIntToArrayBuffer"
    arrbuf_to_bigint = "_taihe_fromArrayBufferToBigInt"
    BEType = "_taihe_BusinessError"
    ACType = "_taihe_AsyncCallback"

    @property
    def mod(self) -> "ArkTsModule":
        return self

    @property
    def impl_desc(self) -> str:
        impl_desc_parts = [
            *self.parent.bundle_parts,
            *self.parent.prefix_parts,
            self.module_name,
        ]
        return ".".join(impl_desc_parts)

    def get_member(self, name: str, is_default: bool, target: StsWriter) -> str:
        filtered_name = "".join(c if c.isalnum() else "_" for c in self.module_name)
        import_name = f"_taihe_{filtered_name}_{name}"
        if is_default:
            target.add_import_default(f"./{self.module_name}.ets", import_name)
        else:
            target.add_import_decl(f"./{self.module_name}.ets", name, import_name)
        return import_name


@dataclass
class ArkTsNamespace(ArkTsModuleOrNamespace):
    parent: ArkTsModuleOrNamespace
    ns_name: str

    scope: ClassVar[AniScope] = ANI_NAMESPACE

    @property
    def mod(self) -> "ArkTsModule":
        return self.parent.mod

    @property
    def impl_desc(self) -> str:
        return f"{self.parent.impl_desc}.{self.ns_name}"

    def get_member(self, name: str, is_default: bool, target: StsWriter) -> str:
        return f"{self.parent.get_member(self.ns_name, self.is_default, target)}.{name}"


# ANI Analyses


class PackageGroupAniInfo(AbstractAnalysis[PackageGroup]):
    def __init__(self, am: AnalysisManager, pg: PackageGroup) -> None:
        self.am = am
        self.pg = pg

        self.mods: dict[str, ArkTsModule] = {}
        self.pkg_map: dict[PackageDecl, ArkTsModuleOrNamespace] = {}

        self.path = ArkTsOutDir(
            bundle_str=self.am.config.arkts_module_prefix,
            prefix_str=self.am.config.arkts_path_prefix,
        )

        for pkg in pg.packages:
            ns_parts = []
            if attr := NamespaceAttr.get(pkg):
                module_str = attr.module
                if ns_name := attr.namespace:
                    ns_parts = ns_name.split(".")
            else:
                module_str = pkg.name

            is_default = ExportDefaultAttr.get(pkg) is not None

            mod = self.mods.setdefault(module_str, ArkTsModule(self.path, module_str))
            ns_name = self.pkg_map[pkg] = mod.add_path(ns_parts, pkg, is_default)

            for attr in StsInjectIntoModuleAttr.get_all(pkg):
                mod.injected_heads.append(attr.sts_code)

            for attr in StsInjectAttr.get_all(pkg):
                ns_name.injected_codes.append(attr.sts_code)

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, pg: PackageGroup) -> "PackageGroupAniInfo":
        return PackageGroupAniInfo(am, pg)

    def get_namespace(self, pkg: PackageDecl) -> ArkTsModuleOrNamespace:
        return self.pkg_map[pkg]


class PackageAniInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.am = am
        self.p = p

        self.header = f"{p.name}.ani.hpp"
        self.source = f"{p.name}.ani.cpp"

        self.cpp_ns = "::".join(p.segments)

        pg_ani_info = PackageGroupAniInfo.get(am, p.parent_group)
        self.ns = pg_ani_info.get_namespace(p)

        if self.am.config.sts_keep_name:
            self.naming = KeepNaming()
        else:
            self.naming = DefaultNaming()

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageAniInfo":
        return PackageAniInfo(am, p)


class GlobFuncAniInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        self.am = am
        self.f = f

        self.native_prefix = "native function "
        self.native_name = f"_taihe_{f.name}_native"

        self.reverse_name = f"_taihe_{f.name}_reverse"

        naming = PackageAniInfo.get(am, f.parent_pkg).naming

        if rename_attr := RenameAttr.get(f):
            func_name = rename_attr.name
        elif rename_attr := OverloadAttr.get(f):
            func_name = rename_attr.func_name
        else:
            func_name = naming.as_func(f.name)

        self.static_scope = None
        self.ctor_scope = None

        if old_ctor_attr := CtorAttr.get(f):
            self.ctor_scope = old_ctor_attr.cls_name
            func_name = ""
        elif ctor_attr := ConstructorAttribute.get(f):
            self.ctor_scope = ctor_attr.cls_name
        elif static_attr := StaticAttr.get(f):
            self.static_scope = static_attr.cls_name

        self.get_name = None
        self.set_name = None
        self.promise_name = None
        self.async_name = None
        self.norm_name = None

        self.gen_async_name = None
        self.gen_promise_name = None

        self.overload_name = None
        self.on_off_pair = None

        if get_attr := GetAttr.get(f):
            if get_attr.member_name is not None:
                get_name = get_attr.member_name
            else:
                get_name = naming.as_field(get_attr.func_suffix)
            self.get_name = get_name
        elif set_attr := SetAttr.get(f):
            if set_attr.member_name is not None:
                set_name = set_attr.member_name
            else:
                set_name = naming.as_field(set_attr.func_suffix)
            self.set_name = set_name
        elif PromiseAttribute.get(f):
            self.promise_name = func_name
        elif AsyncAttribute.get(f):
            self.async_name = func_name
        else:
            self.norm_name = func_name
            if gen_async_attr := GenAsyncAttr.get(f):
                if gen_async_attr.func_name is not None:
                    self.gen_async_name = gen_async_attr.func_name
                else:
                    self.gen_async_name = naming.as_func(gen_async_attr.func_prefix)
            if gen_promise_attr := GenPromiseAttr.get(f):
                if gen_promise_attr.func_name is not None:
                    self.gen_promise_name = gen_promise_attr.func_name
                else:
                    self.gen_promise_name = naming.as_func(gen_promise_attr.func_prefix)

        if overload_attr := StaticOverloadAttribute.get(f):
            self.overload_name = overload_attr.name

        if on_off_attr := OnOffAttr.get(f):
            if on_off_attr.type is not None:
                on_off_type = on_off_attr.type
            else:
                on_off_type = naming.as_field(on_off_attr.func_suffix)
            self.on_off_pair = (on_off_attr.name, on_off_type)

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            self.sts_params.append(param)

        self.is_default = ExportDefaultAttr.get(f) is not None

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncAniInfo":
        return GlobFuncAniInfo(am, f)

    def call_native(self, name: str) -> str:
        return name

    def as_native_args(self, upper_args_sts: list[str]) -> list[str]:
        this_atg_sts = "this"
        last_arg_sts = this_atg_sts
        upper_arg_sts = iter(upper_args_sts)
        lower_args_sts: list[str] = []
        for param in self.f.params:
            if StsThisAttr.get(param):
                lower_args_sts.append(this_atg_sts)
            elif StsLastAttr.get(param):
                lower_args_sts.append(last_arg_sts)
            elif fill_attr := StsFillAttr.get(param):
                fill_arg_sts = fill_attr.content
                lower_args_sts.append(fill_arg_sts)
            else:
                lower_args_sts.append(last_arg_sts := next(upper_arg_sts))
        return lower_args_sts

    def as_normal_args(self, lower_args_sts: list[str]) -> list[str]:
        upper_args_sts: list[str] = []
        for param, lower_arg_sts in zip(self.f.params, lower_args_sts, strict=True):
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            upper_args_sts.append(lower_arg_sts)
        return upper_args_sts


class IfaceMethodAniInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        self.am = am
        self.f = f

        self.native_prefix = "native "
        self.native_name = f"_taihe_{f.name}_native"

        self.reverse_name = f"_taihe_{f.parent_iface.name}_{f.name}_reverse"

        naming = PackageAniInfo.get(am, f.parent_pkg).naming

        if rename_attr := RenameAttr.get(f):
            func_name = rename_attr.name
        elif rename_attr := OverloadAttr.get(f):
            func_name = rename_attr.func_name
        else:
            func_name = naming.as_func(f.name)

        self.get_name = None
        self.set_name = None
        self.promise_name = None
        self.async_name = None
        self.norm_name = None

        self.gen_async_name = None
        self.gen_promise_name = None

        self.overload_name = None
        self.on_off_pair = None

        if get_attr := GetAttr.get(f):
            if get_attr.member_name is not None:
                get_name = get_attr.member_name
            else:
                get_name = naming.as_field(get_attr.func_suffix)
            self.get_name = get_name
        elif set_attr := SetAttr.get(f):
            if set_attr.member_name is not None:
                set_name = set_attr.member_name
            else:
                set_name = naming.as_field(set_attr.func_suffix)
            self.set_name = set_name
        elif PromiseAttribute.get(f):
            self.promise_name = func_name
        elif AsyncAttribute.get(f):
            self.async_name = func_name
        else:
            self.norm_name = func_name
            if gen_async_attr := GenAsyncAttr.get(f):
                if gen_async_attr.func_name is not None:
                    self.gen_async_name = gen_async_attr.func_name
                else:
                    self.gen_async_name = naming.as_func(gen_async_attr.func_prefix)
            if gen_promise_attr := GenPromiseAttr.get(f):
                if gen_promise_attr.func_name is not None:
                    self.gen_promise_name = gen_promise_attr.func_name
                else:
                    self.gen_promise_name = naming.as_func(gen_promise_attr.func_prefix)

        if overload_attr := StaticOverloadAttribute.get(f):
            self.overload_name = overload_attr.name

        if on_off_attr := OnOffAttr.get(f):
            if on_off_attr.type is not None:
                on_off_type = on_off_attr.type
            else:
                on_off_type = naming.as_field(on_off_attr.func_suffix)
            self.on_off_pair = (on_off_attr.name, on_off_type)

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            self.sts_params.append(param)

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodAniInfo":
        return IfaceMethodAniInfo(am, f)

    def call_native(self, name: str) -> str:
        return f"this.{name}" if name else "this"

    def as_native_args(self, upper_args_sts: list[str]) -> list[str]:
        this_atg_sts = "this"
        last_arg_sts = this_atg_sts
        upper_arg_sts = iter(upper_args_sts)
        lower_args_sts: list[str] = []
        for param in self.f.params:
            if StsThisAttr.get(param):
                lower_args_sts.append(this_atg_sts)
            elif StsLastAttr.get(param):
                lower_args_sts.append(last_arg_sts)
            elif fill_attr := StsFillAttr.get(param):
                fill_arg_sts = fill_attr.content
                lower_args_sts.append(fill_arg_sts)
            else:
                lower_args_sts.append(last_arg_sts := next(upper_arg_sts))
        return lower_args_sts

    def as_normal_args(self, lower_args_sts: list[str]) -> list[str]:
        upper_args_sts: list[str] = []
        for param, lower_arg_sts in zip(self.f.params, lower_args_sts, strict=True):
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            upper_args_sts.append(lower_arg_sts)
        return upper_args_sts


class NamedFunctionLikeAniInfo:
    @staticmethod
    def get(
        am: AnalysisManager,
        f: GlobFuncDecl | IfaceMethodDecl,
    ) -> GlobFuncAniInfo | IfaceMethodAniInfo:
        if isinstance(f, GlobFuncDecl):
            return GlobFuncAniInfo.get(am, f)
        if isinstance(f, IfaceMethodDecl):
            return IfaceMethodAniInfo.get(am, f)


class EnumAniInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        self.parent_ns = PackageAniInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        self.type_desc = f"{self.parent_ns.impl_desc}.{self.sts_type_name}"

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: EnumDecl) -> "EnumAniInfo":
        return EnumAniInfo(am, d)

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(self.sts_type_name, self.is_default, target)


class UnionAniInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.hpp"

        self.parent_ns = PackageAniInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name

        self.sts_all_fields: list[list[UnionFieldDecl]] = []
        for field in d.fields:
            if isinstance(field_ty := field.ty, UnionType):
                inner_ani_info = UnionAniInfo.get(am, field_ty.decl)
                self.sts_all_fields.extend(
                    [field, *parts] for parts in inner_ani_info.sts_all_fields
                )
            else:
                self.sts_all_fields.append([field])

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: UnionDecl) -> "UnionAniInfo":
        return UnionAniInfo(am, d)

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(self.sts_type_name, self.is_default, target)


class StructAniInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.hpp"

        self.parent_ns = PackageAniInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        if ClassAttr.get(d):
            self.sts_impl_name = self.sts_type_name
        else:
            self.sts_impl_name = f"_taihe_{d.name}_inner"
        self.sts_ctor_name = f"_taihe_{d.name}_ctor"
        self.type_desc = f"{self.parent_ns.impl_desc}.{self.sts_type_name}"
        self.impl_desc = f"{self.parent_ns.impl_desc}.{self.sts_impl_name}"

        self.interface_injected_codes: list[str] = []
        for iface_injected in StsInjectIntoIfaceAttr.get_all(d):
            self.interface_injected_codes.append(iface_injected.sts_code)
        self.class_injected_codes: list[str] = []
        for class_injected in StsInjectIntoClazzAttr.get_all(d):
            self.class_injected_codes.append(class_injected.sts_code)

        self.sts_class_extends: list[StructFieldDecl] = []
        self.sts_iface_extends: list[StructFieldDecl] = []
        self.sts_all_fields: list[list[StructFieldDecl]] = []
        for field in d.fields:
            if extend := ExtendsAttr.get(field):
                extend_ani_info = StructAniInfo.get(am, extend.ty.decl)
                if extend_ani_info.is_class():
                    self.sts_class_extends.append(field)
                else:
                    self.sts_iface_extends.append(field)
                self.sts_all_fields.extend(
                    [field, *parts] for parts in extend_ani_info.sts_all_fields
                )
            else:
                self.sts_all_fields.append([field])

        self.sorted_sts_all_fields = sorted(
            self.sts_all_fields,
            key=lambda parts: OptionalAttr.get(parts[-1]) is not None,
        )

        self.sts_local_fields: list[StructFieldDecl] = []
        self.sts_class_extend_fields: list[StructFieldDecl] = []
        self.sts_iface_extend_fields: list[StructFieldDecl] = []
        for parts in self.sorted_sts_all_fields:
            origin = parts[0]
            final = parts[-1]
            if extend := ExtendsAttr.get(origin):
                extend_ani_info = StructAniInfo.get(am, extend.ty.decl)
                if extend_ani_info.is_class():
                    self.sts_class_extend_fields.append(final)
                else:
                    self.sts_iface_extend_fields.append(final)
            else:
                self.sts_local_fields.append(final)

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: StructDecl) -> "StructAniInfo":
        return StructAniInfo(am, d)

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(self.sts_type_name, self.is_default, target)


class IfaceAniInfo(AbstractAnalysis[IfaceDecl]):
    scope: ClassVar[AniScope] = ANI_CLASS

    data_ptr = "_taihe_dataPtr"
    vtbl_ptr = "_taihe_vtblPtr"
    register = "_taihe_register"

    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.hpp"

        self.parent_ns = PackageAniInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        if ClassAttr.get(d):
            self.sts_impl_name = self.sts_type_name
        else:
            self.sts_impl_name = f"_taihe_{d.name}_inner"
        self.sts_ctor_name = f"_taihe_{d.name}_ctor"
        self.type_desc = f"{self.parent_ns.impl_desc}.{self.sts_type_name}"
        self.impl_desc = f"{self.parent_ns.impl_desc}.{self.sts_impl_name}"

        self.interface_injected_codes: list[str] = []
        for iface_injected in StsInjectIntoIfaceAttr.get_all(d):
            self.interface_injected_codes.append(iface_injected.sts_code)
        self.class_injected_codes: list[str] = []
        for class_injected in StsInjectIntoClazzAttr.get_all(d):
            self.class_injected_codes.append(class_injected.sts_code)

        self.sts_class_extends: list[IfaceExtendDecl] = []
        self.sts_iface_extends: list[IfaceExtendDecl] = []
        for extend in d.extends:
            extend_ani_info = IfaceAniInfo.get(am, extend.ty.decl)
            if extend_ani_info.is_class():
                self.sts_class_extends.append(extend)
            else:
                self.sts_iface_extends.append(extend)

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceAniInfo":
        return IfaceAniInfo(am, d)

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(self.sts_type_name, self.is_default, target)


class TypeAniInfo(AbstractAnalysis[NonVoidType], ABC):
    ani_type: AniType
    sig_type: AniRuntimeType

    def __init__(self, am: AnalysisManager, t: NonVoidType):
        self.cpp_info = TypeCppInfo.get(am, t)

    @property
    def type_sig(self) -> str:
        return self.sig_type.sig

    @property
    def type_sig_boxed(self) -> str:
        return self.sig_type.boxed.sig

    @property
    def type_desc(self) -> str:
        return self.sig_type.boxed.desc

    @classmethod
    @override
    def _create(cls, am: AnalysisManager, t: NonVoidType) -> "TypeAniInfo":
        return t.accept(TypeAniInfoDispatcher(am))

    @abstractmethod
    def sts_type_in(self, target: StsWriter) -> str:
        pass

    @abstractmethod
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        pass

    @abstractmethod
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        pass

    def check_type(
        self,
        target: CSourceWriter,
        env: str,
        is_field_ani: str,
    ):
        target.writelns(
            f'{env}->Object_InstanceOf(static_cast<ani_object>(ani_value), TH_ANI_FIND_CLASS(env, "{self.type_desc}"), &{is_field_ani});',
        )

    def into_ani_boxed(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_boxed: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.into_ani(target, env, cpp_value, ani_boxed)
        else:
            ani_after = f"{ani_boxed}_ani_after"
            target.writelns(
                f"ani_object {ani_boxed} = {{}};",
            )
            self.into_ani(target, env, cpp_value, ani_after)
            target.writelns(
                f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "<ctor>", "{self.type_sig}:"), &{ani_boxed}, {ani_after});',
            )

    def from_ani_boxed(
        self,
        target: CSourceWriter,
        env: str,
        ani_boxed: str,
        cpp_after: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_value = f"static_cast<{self.ani_type}>({ani_boxed})"
            self.from_ani(target, env, ani_value, cpp_after)
        else:
            ani_value = f"{cpp_after}_ani_value"
            target.writelns(
                f"{self.ani_type} {ani_value} = {{}};",
                f'{env}->Object_CallMethod_{self.ani_type.suffix}(static_cast<ani_object>({ani_boxed}), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "to{self.ani_type.suffix}", ":{self.type_sig}"), &{ani_value});',
            )
            self.from_ani(target, env, ani_value, cpp_after)

    def from_ani_fixedarray(
        self,
        target: CSourceWriter,
        env: str,
        ani_size: str,
        ani_fixedarray_value: str,
        cpp_fixedarray_buffer: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_item = f"{cpp_fixedarray_buffer}_ani_item"
            cpp_item = f"{cpp_fixedarray_buffer}_cpp_item"
            iterator = f"{cpp_fixedarray_buffer}_iterator"
            with target.indented(
                f"for (size_t {iterator} = 0; {iterator} < {ani_size}; {iterator}++) {{",
                f"}}",
            ):
                target.writelns(
                    f"{self.ani_type} {ani_item} = {{}};",
                    f"{env}->FixedArray_Get_Ref({ani_fixedarray_value}, {iterator}, reinterpret_cast<ani_ref*>(&{ani_item}));",
                )
                self.from_ani(target, env, ani_item, cpp_item)
                target.writelns(
                    f"new (&{cpp_fixedarray_buffer}[{iterator}]) {self.cpp_info.as_owner}(std::move({cpp_item}));",
                )
        else:
            target.writelns(
                f"{env}->FixedArray_GetRegion_{self.ani_type.suffix}({ani_fixedarray_value}, 0, {ani_size}, reinterpret_cast<{self.ani_type}*>({cpp_fixedarray_buffer}));",
            )

    def into_ani_fixedarray(
        self,
        target: CSourceWriter,
        env: str,
        cpp_size: str,
        cpp_fixedarray_value: str,
        ani_fixedarray_after: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_item = f"{ani_fixedarray_after}_ani_item"
            ani_none = f"{ani_fixedarray_after}_ani_none"
            iterator = f"{ani_fixedarray_after}_itorator"
            target.writelns(
                f"ani_fixedarray_ref {ani_fixedarray_after} = {{}};",
                f"ani_ref {ani_none} = {{}};",
                f"{env}->GetUndefined(&{ani_none});",
                f'{env}->FixedArray_New_Ref(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), {cpp_size}, {ani_none}, &{ani_fixedarray_after});',
            )
            with target.indented(
                f"for (size_t {iterator} = 0; {iterator} < {cpp_size}; {iterator}++) {{",
                f"}}",
            ):
                self.into_ani(
                    target,
                    env,
                    f"{cpp_fixedarray_value}[{iterator}]",
                    ani_item,
                )
                target.writelns(
                    f"{env}->FixedArray_Set_Ref({ani_fixedarray_after}, {iterator}, {ani_item});",
                )
        else:
            target.writelns(
                f"{self.ani_type.fixedarray} {ani_fixedarray_after} = {{}};",
                f"{env}->FixedArray_New_{self.ani_type.suffix}({cpp_size}, &{ani_fixedarray_after});",
                f"{env}->FixedArray_SetRegion_{self.ani_type.suffix}({ani_fixedarray_after}, 0, {cpp_size}, reinterpret_cast<{self.ani_type} const*>({cpp_fixedarray_value}));",
            )


class EnumTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: EnumType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        enum_ani_info = EnumAniInfo.get(self.am, self.t.decl)
        self.ani_type = ANI_ENUM_ITEM
        self.sig_type = AniRuntimeEnumType(enum_ani_info.type_desc)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        enum_ani_info = EnumAniInfo.get(self.am, self.t.decl)
        return enum_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        ani_index = f"{cpp_after}_ani_index"
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.decl)
        target.writelns(
            f"ani_size {ani_index} = {{}};",
            f"{env}->EnumItem_GetIndex({ani_value}, &{ani_index});",
            f"{enum_cpp_info.full_name} {cpp_after}(static_cast<{enum_cpp_info.full_name}::key_t>({ani_index}));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        target.writelns(
            f"ani_enum_item {ani_after} = {{}};",
            f'{env}->Enum_GetEnumItemByIndex(TH_ANI_FIND_ENUM({env}, "{self.type_desc}"), static_cast<ani_size>({cpp_value}.get_key()), &{ani_after});',
        )


class ConstEnumTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: EnumType, const_attr: ConstAttr):
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.const_attr = const_attr
        enum_ty_ani_info = TypeAniInfo.get(self.am, self.t.decl.ty)
        self.ani_type = enum_ty_ani_info.ani_type
        self.sig_type = enum_ty_ani_info.sig_type

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        enum_ty_ani_info = TypeAniInfo.get(self.am, self.t.decl.ty)
        return enum_ty_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        cpp_temp = f"{cpp_after}_cpp_temp"
        enum_ty_ani_info = TypeAniInfo.get(self.am, self.t.decl.ty)
        enum_ty_ani_info.from_ani(target, env, ani_value, cpp_temp)
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.decl)
        target.writelns(
            f"{enum_cpp_info.full_name} {cpp_after} = {enum_cpp_info.full_name}::from_value({cpp_temp});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        cpp_temp = f"{ani_after}_cpp_temp"
        enum_ty_cpp_info = TypeCppInfo.get(self.am, self.t.decl.ty)
        enum_ty_ani_info = TypeAniInfo.get(self.am, self.t.decl.ty)
        target.writelns(
            f"{enum_ty_cpp_info.as_owner} {cpp_temp} = {cpp_value}.get_value();",
        )
        enum_ty_ani_info.into_ani(target, env, cpp_temp, ani_after)


class StructTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: StructType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        struct_ani_info = StructAniInfo.get(self.am, self.t.decl)
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType(struct_ani_info.type_desc)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        struct_ani_info = StructAniInfo.get(self.am, self.t.decl)
        return struct_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        struct_ani_info = StructAniInfo.get(self.am, self.t.decl)
        struct_cpp_info = StructCppInfo.get(self.am, self.t.decl)
        target.add_include(struct_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = ::taihe::from_ani<{struct_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        struct_ani_info = StructAniInfo.get(self.am, self.t.decl)
        struct_cpp_info = StructCppInfo.get(self.am, self.t.decl)
        target.add_include(struct_ani_info.impl_header)
        target.writelns(
            f"ani_object {ani_after} = ::taihe::into_ani<{struct_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class UnionTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_REF
        sig_types: list[AniRuntimeType] = []
        for field in t.decl.fields:
            field_ani_info = TypeAniInfo.get(self.am, field.ty)
            sig_types.append(field_ani_info.sig_type)
        self.sig_type = AniRuntimeUnionType.union(*sig_types)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        union_ani_info = UnionAniInfo.get(self.am, self.t.decl)
        return union_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        union_ani_info = UnionAniInfo.get(self.am, self.t.decl)
        union_cpp_info = UnionCppInfo.get(self.am, self.t.decl)
        target.add_include(union_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = ::taihe::from_ani<{union_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        union_ani_info = UnionAniInfo.get(self.am, self.t.decl)
        union_cpp_info = UnionCppInfo.get(self.am, self.t.decl)
        target.add_include(union_ani_info.impl_header)
        target.writelns(
            f"ani_ref {ani_after} = ::taihe::into_ani<{union_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class IfaceTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        iface_ani_info = IfaceAniInfo.get(self.am, self.t.decl)
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType(iface_ani_info.type_desc)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        iface_ani_info = IfaceAniInfo.get(self.am, self.t.decl)
        return iface_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        iface_ani_info = IfaceAniInfo.get(self.am, self.t.decl)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.t.decl)
        target.add_include(iface_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = ::taihe::from_ani<{iface_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        iface_ani_info = IfaceAniInfo.get(self.am, self.t.decl)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.t.decl)
        target.add_include(iface_ani_info.impl_header)
        target.writelns(
            f"ani_object {ani_after} = ::taihe::into_ani<{iface_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class NullTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: UnitType):
        super().__init__(am, t)
        self.ani_type = ANI_REF
        self.sig_type = AniRuntimeClassType("std.core.Null")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "null"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = {{}};",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        target.writelns(
            f"ani_ref {ani_after} = {{}};",
            f"{env}->GetNull(&{ani_after});",
        )

    @override
    def check_type(
        self,
        target: CSourceWriter,
        env: str,
        is_field_ani: str,
    ):
        target.writelns(
            f"{env}->Reference_IsNull(ani_value, &{is_field_ani});",
        )


class UndefinedTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: UnitType):
        super().__init__(am, t)
        self.ani_type = ANI_REF
        self.sig_type = AniRuntimeUndefinedType()

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "undefined"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = {{}};",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        target.writelns(
            f"ani_ref {ani_after} = {{}};",
            f"{env}->GetUndefined(&{ani_after});",
        )

    @override
    def check_type(
        self,
        target: CSourceWriter,
        env: str,
        is_field_ani: str,
    ):
        target.writelns(
            f"{env}->Reference_IsUndefined(ani_value, &{is_field_ani});",
        )


class StringLiteralTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: UnitType, literal_attr: LiteralAttr):
        super().__init__(am, t)
        self.ani_type = ANI_STRING
        self.sig_type = AniRuntimeClassType("std.core.String")
        self.value = literal_attr.value

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return dumps(self.value)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = {{}};",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        cpp_strv = f"{ani_after}_cpp_strv"
        target.writelns(
            f"std::string_view {cpp_strv} = {dumps(self.value)};",
            f"ani_string {ani_after} = {{}};",
            f"{env}->String_NewUTF8({cpp_strv}.data(), {cpp_strv}.size(), &{ani_after});",
        )

    @override
    def check_type(
        self,
        target: CSourceWriter,
        env: str,
        is_field_ani: str,
    ):
        super().check_type(target, env, is_field_ani)
        cpp_strv = f"{is_field_ani}_cpp_strv"
        ani_size = f"{is_field_ani}_size"
        cpp_buff = f"{is_field_ani}_buff"
        target.writelns(
            f"std::string_view {cpp_strv} = {dumps(self.value)};",
            f"ani_size {ani_size} = {{}};",
            f"{env}->String_GetUTF8Size(static_cast<ani_string>(ani_value), &{ani_size});",
            f"char {cpp_buff}[{ani_size} + 1];",
            f"{env}->String_GetUTF8(static_cast<ani_string>(ani_value), {cpp_buff}, {ani_size} + 1, &{ani_size});",
            f"{cpp_buff}[{ani_size}] = '\\0';",
            f"{is_field_ani} &= {cpp_strv} == {cpp_buff};",
        )


class ScalarTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        super().__init__(am, t)
        sts_info = {
            ScalarKind.BOOL: (ANI_BOOLEAN, "boolean", "z"),
            ScalarKind.F32: (ANI_FLOAT, "float", "f"),
            ScalarKind.F64: (ANI_DOUBLE, "double", "d"),
            ScalarKind.I8: (ANI_BYTE, "byte", "b"),
            ScalarKind.I16: (ANI_SHORT, "short", "s"),
            ScalarKind.I32: (ANI_INT, "int", "i"),
            ScalarKind.I64: (ANI_LONG, "long", "l"),
            ScalarKind.U8: (ANI_BYTE, "byte", "b"),
            ScalarKind.U16: (ANI_SHORT, "short", "s"),
            ScalarKind.U32: (ANI_INT, "int", "i"),
            ScalarKind.U64: (ANI_LONG, "long", "l"),
        }[t.kind]
        ani_type, sts_type, sig = sts_info
        sig_type_boxed = AniRuntimeClassType(f"std.core.{sts_type.capitalize()}")
        sig_type = AniRuntimePrimitiveType(sig_type_boxed, sig)
        self.sig_type = sig_type
        self.ani_type = ani_type
        self.sts_type = sts_type

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = static_cast<{self.cpp_info.as_owner}>({ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        target.writelns(
            f"{self.ani_type} {ani_after} = static_cast<{self.ani_type}>({cpp_value});",
        )


class StringTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.ani_type = ANI_STRING
        self.sig_type = AniRuntimeClassType("std.core.String")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "string"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        ani_size = f"{cpp_after}_ani_size"
        cpp_tstr = f"{cpp_after}_cpp_tstr"
        cpp_buff = f"{cpp_after}_cpp_buff"
        target.writelns(
            f"ani_size {ani_size} = {{}};",
            f"{env}->String_GetUTF8Size({ani_value}, &{ani_size});",
            f"TString {cpp_tstr};",
            f"char* {cpp_buff} = tstr_initialize(&{cpp_tstr}, {ani_size} + 1);",
            f"{env}->String_GetUTF8({ani_value}, {cpp_buff}, {ani_size} + 1, &{ani_size});",
            f"{cpp_buff}[{ani_size}] = '\\0';",
            f"{cpp_tstr}.length = {ani_size};",
            f"::taihe::string {cpp_after} = ::taihe::string({cpp_tstr});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        target.writelns(
            f"ani_string {ani_after} = {{}};",
            f"{env}->String_NewUTF8({cpp_value}.c_str(), {cpp_value}.size(), &{ani_after});",
        )


class OpaqueTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_OBJECT
        if sts_type_attr := StsTypeAttr.get(self.t.ref):
            self.sts_type = sts_type_attr.type_name
        else:
            self.sts_type = "Object"
        self.sig_type = AniRuntimeClassType("std.core.Object")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = reinterpret_cast<{self.cpp_info.as_owner}>({ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        target.writelns(
            f"{self.ani_type} {ani_after} = reinterpret_cast<{self.ani_type}>({cpp_value});",
        )


class OptionalTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        self.ani_type = ANI_REF
        self.sig_type = item_ty_ani_info.sig_type.boxed

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"({sts_type} | undefined)"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        ani_flag = f"{cpp_after}_ani_flag"
        cpp_temp = f"{cpp_after}_cpp_temp"
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after};",
            f"ani_boolean {ani_flag} = {{}};",
            f"{env}->Reference_IsUndefined({ani_value}, &{ani_flag});",
        )
        with target.indented(
            f"if (!{ani_flag}) {{",
            f"}};",
        ):
            item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
            item_ty_ani_info.from_ani_boxed(target, env, ani_value, cpp_temp)
            target.writelns(
                f"{cpp_after}.emplace(std::move({cpp_temp}));",
            )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        ani_temp = f"{ani_after}_ani_temp"
        target.writelns(
            f"ani_ref {ani_after} = {{}};",
        )
        with target.indented(
            f"if (!{cpp_value}) {{",
            f"}}",
        ):
            target.writelns(
                f"{env}->GetUndefined(&{ani_after});",
            )
        with target.indented(
            f"else {{",
            f"}}",
        ):
            item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
            item_ty_ani_info.into_ani_boxed(target, env, f"(*{cpp_value})", ani_temp)
            target.writelns(
                f"{ani_after} = {ani_temp};",
            )


class FixedArrayTypeAniInfo(TypeAniInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        fixedarray_attr: FixedArrayAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        self.ani_type = item_ty_ani_info.ani_type.fixedarray
        self.sig_type = AniRuntimeFixedArrayType(item_ty_ani_info.sig_type)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"FixedArray<{sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_after}_ani_size"
        cpp_buffer = f"{cpp_after}_cpp_buffer"
        target.writelns(
            f"ani_size {ani_size} = {{}};",
            f"{env}->FixedArray_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_fixedarray(
            target,
            env,
            ani_size,
            ani_value,
            cpp_buffer,
        )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after}({cpp_buffer}, {ani_size});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_after}_cpp_size"
        target.writelns(
            f"size_t {cpp_size} = {cpp_value}.size();",
        )
        item_ty_ani_info.into_ani_fixedarray(
            target,
            env,
            cpp_size,
            f"{cpp_value}.data()",
            ani_after,
        )


class ArrayTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_ARRAY
        self.sig_type = AniRuntimeClassType("std.core.Array")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"Array<{sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_after}_ani_size"
        cpp_buffer = f"{cpp_after}_cpp_buffer"
        ani_item = f"{cpp_buffer}_ani_item"
        cpp_item = f"{cpp_buffer}_cpp_item"
        iterator = f"{cpp_buffer}_iterator"
        target.writelns(
            f"ani_size {ani_size} = {{}};",
            f"{env}->Array_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        with target.indented(
            f"for (size_t {iterator} = 0; {iterator} < {ani_size}; {iterator}++) {{",
            f"}}",
        ):
            target.writelns(
                f"ani_ref {ani_item} = {{}};",
                f"{env}->Array_Get({ani_value}, {iterator}, &{ani_item});",
            )
            item_ty_ani_info.from_ani_boxed(target, env, ani_item, cpp_item)
            target.writelns(
                f"new (&{cpp_buffer}[{iterator}]) {item_ty_ani_info.cpp_info.as_owner}(std::move({cpp_item}));",
            )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after}({cpp_buffer}, {ani_size});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        item_ty_ani_info = TypeAniInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_after}_ani_size"
        ani_item = f"{ani_after}_ani_item"
        ani_none = f"{ani_after}_ani_none"
        iterator = f"{ani_after}_iterator"
        target.writelns(
            f"size_t {cpp_size} = {cpp_value}.size();",
            f"ani_array {ani_after} = {{}};",
            f"ani_ref {ani_none} = {{}};",
            f"{env}->GetUndefined(&{ani_none});",
            f"{env}->Array_New({cpp_size}, {ani_none}, &{ani_after});",
        )
        with target.indented(
            f"for (size_t {iterator} = 0; {iterator} < {cpp_size}; {iterator}++) {{",
            f"}}",
        ):
            item_ty_ani_info.into_ani_boxed(
                target,
                env,
                f"{cpp_value}[{iterator}]",
                ani_item,
            )
            target.writelns(
                f"{env}->Array_Set({ani_after}, {iterator}, {ani_item});",
            )


class ArrayBufferTypeAniInfo(TypeAniInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        arraybuffer_attr: ArrayBufferAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.arraybuffer_attr = arraybuffer_attr
        self.ani_type = ANI_ARRAYBUFFER
        self.sig_type = AniRuntimeClassType("std.core.ArrayBuffer")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "ArrayBuffer"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{cpp_after}_ani_data"
        ani_length = f"{cpp_after}_ani_length"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_size {ani_length} = {{}};",
            f"{env}->ArrayBuffer_GetInfo({ani_value}, &{ani_data}, &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_after}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{ani_after}_ani_data"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_arraybuffer {ani_after} = {{}};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), &{ani_data}, &{ani_after});",
            f"std::copy({cpp_value}.begin(), {cpp_value}.end(), reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}));",
        )


class TypedArrayTypeAniInfo(TypeAniInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        typedarray_attr: TypedArrayAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.typedarray_attr = typedarray_attr
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType(f"escompat.{self.typedarray_attr.sts_type}")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.typedarray_attr.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_byte_length = f"{cpp_after}_ani_byte_length"
        ani_byte_offset = f"{cpp_after}_ani_byte_offset"
        ani_arrbuf = f"{cpp_after}_ani_arrbuf"
        ani_data = f"{cpp_after}_ani_data"
        ani_length = f"{cpp_after}_ani_length"
        target.writelns(
            f"ani_int {ani_byte_length} = {{}};",
            f"ani_int {ani_byte_offset} = {{}};",
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
        )
        pass
        if self.t.item_ty.kind.is_signed:
            target.writelns(
                f'{env}->Object_GetField_Int({ani_value}, TH_ANI_FIND_CLASS_FIELD({env}, "{self.type_desc}", "byteLength"), &{ani_byte_length});',
                f'{env}->Object_GetField_Int({ani_value}, TH_ANI_FIND_CLASS_FIELD({env}, "{self.type_desc}", "byteOffset"), &{ani_byte_offset});',
            )
        else:
            target.writelns(
                f'{env}->Object_CallMethod_Int({ani_value}, TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "%%get-byteLength", ":i"), &{ani_byte_length});',
                f'{env}->Object_CallMethod_Int({ani_value}, TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "%%get-byteOffset", ":i"), &{ani_byte_offset});',
            )
        target.writelns(
            f'{env}->Object_GetField_Ref({ani_value}, TH_ANI_FIND_CLASS_FIELD({env}, "{self.type_desc}", "buffer"), reinterpret_cast<ani_ref*>(&{ani_arrbuf}));',
            f"void* {ani_data} = {{}};",
            f"ani_size {ani_length} = {{}};",
            f"{env}->ArrayBuffer_GetInfo({ani_arrbuf}, &{ani_data}, &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_after}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}) + {ani_byte_offset}, {ani_byte_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{ani_after}_ani_data"
        ani_arrbuf = f"{ani_after}_ani_arrbuf"
        ani_byte_length = f"{ani_after}_ani_byte_length"
        ani_byte_offset = f"{ani_after}_ani_byte_offset"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), &{ani_data}, &{ani_arrbuf});",
            f"std::copy({cpp_value}.begin(), {cpp_value}.end(), reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}));",
            f"ani_ref {ani_byte_length} = {{}};",
            f"{env}->GetUndefined(&{ani_byte_length});",
            f"ani_ref {ani_byte_offset} = {{}};",
            f"{env}->GetUndefined(&{ani_byte_offset});",
            f"ani_object {ani_after} = {{}};",
            f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "<ctor>", "C{{std.core.ArrayBuffer}}C{{std.core.Double}}C{{std.core.Double}}:"), &{ani_after}, {ani_arrbuf}, {ani_byte_length}, {ani_byte_offset});',
        )


class BigIntTypeAniInfo(TypeAniInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        bigint_attr: BigIntAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.bigint_attr = bigint_attr
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType("std.core.BigInt")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "BigInt"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageAniInfo.get(self.am, self.t.ref.parent_pkg)
        ani_arrbuf = f"{cpp_after}_ani_arrbuf"
        ani_data = f"{cpp_after}_ani_data"
        ani_length = f"{cpp_after}_ani_length"
        target.writelns(
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.mod.impl_desc}", "{pkg_ani_info.ns.mod.bigint_to_arrbuf}", "C{{std.core.BigInt}}i:C{{std.core.ArrayBuffer}}"), reinterpret_cast<ani_ref*>(&{ani_arrbuf}), {ani_value}, static_cast<ani_int>(sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));'
            f"void* {ani_data} = {{}};",
            f"ani_size {ani_length} = {{}};",
            f"{env}->ArrayBuffer_GetInfo({ani_arrbuf}, &{ani_data}, &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_after}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageAniInfo.get(self.am, self.t.ref.parent_pkg)
        ani_data = f"{ani_after}_ani_data"
        ani_arrbuf = f"{ani_after}_ani_arrbuf"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), &{ani_data}, &{ani_arrbuf});",
            f"std::copy({cpp_value}.begin(), {cpp_value}.end(), reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}));",
            f"ani_object {ani_after} = {{}};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.mod.impl_desc}", "{pkg_ani_info.ns.mod.arrbuf_to_bigint}", "C{{std.core.ArrayBuffer}}:C{{std.core.BigInt}}"), reinterpret_cast<ani_ref*>(&{ani_after}), {ani_arrbuf});',
        )


class RecordTypeAniInfo(TypeAniInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: MapType,
        record_attr: RecordAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.record_attr = record_attr
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType("std.core.Record")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        key_ty_ani_info = TypeAniInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeAniInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(target)
        val_sts_type = val_ty_ani_info.sts_type_in(target)
        return f"Record<{key_sts_type}, {val_sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        ani_iter = f"{cpp_after}_ani_iter"
        ani_item = f"{cpp_after}_ani_item"
        ani_next = f"{cpp_after}_ani_next"
        ani_done = f"{cpp_after}_ani_done"
        ani_key = f"{cpp_after}_ani_key"
        ani_val = f"{cpp_after}_ani_val"
        cpp_key = f"{cpp_after}_cpp_key"
        cpp_val = f"{cpp_after}_cpp_val"
        key_ty_ani_info = TypeAniInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeAniInfo.get(self.am, self.t.val_ty)
        target.writelns(
            f"ani_object {ani_iter} = {{}};",
            f'{env}->Object_CallMethod_Ref({ani_value}, TH_ANI_FIND_CLASS_METHOD({env}, "std.core.Record", "entries", ":C{{std.core.IterableIterator}}"), reinterpret_cast<ani_ref*>(&{ani_iter}));',
            f"{self.cpp_info.as_owner} {cpp_after};",
        )
        with target.indented(
            f"while (true) {{",
            f"}}",
        ):
            target.writelns(
                f"ani_object {ani_next} = {{}};",
                f"ani_boolean {ani_done} = {{}};",
                f'{env}->Object_CallMethod_Ref({ani_iter}, TH_ANI_FIND_CLASS_METHOD({env}, "std.core.Iterator", "next", ":C{{std.core.IteratorResult}}"), reinterpret_cast<ani_ref*>(&{ani_next}));',
                f'{env}->Object_GetField_Boolean({ani_next}, TH_ANI_FIND_CLASS_FIELD({env}, "std.core.IteratorResult", "done"), &{ani_done});',
            )
            with target.indented(
                f"if ({ani_done}) {{;",
                f"}};",
            ):
                target.writelns(
                    f"break;",
                )
            target.writelns(
                f"ani_tuple_value {ani_item} = {{}};",
                f'{env}->Object_GetField_Ref({ani_next},  TH_ANI_FIND_CLASS_FIELD({env}, "std.core.IteratorResult", "value"), reinterpret_cast<ani_ref*>(&{ani_item}));',
                f"ani_ref {ani_key} = {{}};",
                f'{env}->Object_GetField_Ref({ani_item}, TH_ANI_FIND_CLASS_FIELD({env}, "std.core.Tuple2", "$0"), &{ani_key});',
                f"ani_ref {ani_val} = {{}};",
                f'{env}->Object_GetField_Ref({ani_item}, TH_ANI_FIND_CLASS_FIELD({env}, "std.core.Tuple2", "$1"), &{ani_val});',
            )
            key_ty_ani_info.from_ani_boxed(target, env, ani_key, cpp_key)
            val_ty_ani_info.from_ani_boxed(target, env, ani_val, cpp_val)
            target.writelns(
                f"{cpp_after}.emplace({cpp_key}, {cpp_val});",
            )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        key_ty_ani_info = TypeAniInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeAniInfo.get(self.am, self.t.val_ty)
        cpp_key = f"{ani_after}_cpp_key"
        cpp_val = f"{ani_after}_cpp_val"
        ani_key = f"{ani_after}_ani_key"
        ani_val = f"{ani_after}_ani_val"
        target.writelns(
            f"ani_object {ani_after} = {{}};",
            f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "std.core.Record"), TH_ANI_FIND_CLASS_METHOD({env}, "std.core.Record", "<ctor>", ":"), &{ani_after});',
        )
        with target.indented(
            f"for (const auto& [{cpp_key}, {cpp_val}] : {cpp_value}) {{",
            f"}}",
        ):
            key_ty_ani_info.into_ani_boxed(target, env, cpp_key, ani_key)
            val_ty_ani_info.into_ani_boxed(target, env, cpp_val, ani_val)
            target.writelns(
                f'{env}->Object_CallMethod_Void({ani_after}, TH_ANI_FIND_CLASS_METHOD({env}, "std.core.Record", "$_set", "X{{C{{std.core.BaseEnum}}C{{std.core.Numeric}}C{{std.core.String}}}}Y:"), {ani_key}, {ani_val});',
            )


class CallbackTypeAniInfo(TypeAniInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_FN_OBJECT
        self.sig_type = AniRuntimeClassType(f"std.core.Function{len(t.ref.params)}")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        params = []
        for param in self.t.ref.params:
            opt = "?" if OptionalAttr.get(param) else ""
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            params.append(f"{param.name}{opt}: {param_ty_ani_info.sts_type_in(target)}")
        params_str = ", ".join(params)
        if isinstance(return_ty := self.t.ref.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_sts_name = return_ty_ani_info.sts_type_in(target)
        else:
            return_ty_sts_name = "void"
        return f"(({params_str}) => {return_ty_sts_name})"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_after: str,
    ):
        cpp_impl_t = f"{cpp_after}_cpp_impl_t"
        with target.indented(
            f"struct {cpp_impl_t} : ::taihe::dref_guard {{",
            f"}};",
        ):
            target.writelns(
                f"{cpp_impl_t}(ani_env* env, ani_ref val) : ::taihe::dref_guard(env, val) {{}}",
            )
            self.gen_invoke_operator(target)
            with target.indented(
                f"uintptr_t getGlobalReference() const {{",
                f"}}",
            ):
                target.writelns(
                    f"return reinterpret_cast<uintptr_t>(this->ref);",
                )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_after} = ::taihe::make_holder<{cpp_impl_t}, {self.cpp_info.as_owner}, ::taihe::platform::ani::AniObject>({env}, {ani_value});",
        )

    def gen_invoke_operator(
        self,
        target: CSourceWriter,
    ):
        params_cpp = []
        args_ani = []
        args_cpp = []
        for param in self.t.ref.params:
            arg_cpp = f"cpp_arg_{param.name}"
            arg_ani = f"ani_arg_{param.name}"
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{param_ty_cpp_info.as_param} {arg_cpp}")
            args_cpp.append(arg_cpp)
            args_ani.append(arg_ani)
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := self.t.ref.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = return_ty_cpp_info.as_owner
        else:
            return_ty_cpp_name = "void"
        with target.indented(
            f"{return_ty_cpp_name} operator()({params_cpp_str}) {{",
            f"}}",
        ):
            target.writelns(
                f"::taihe::env_guard guard;",
                f"ani_env *env = guard.get_env();",
            )
            for param, arg_cpp, arg_ani in zip(
                self.t.ref.params, args_cpp, args_ani, strict=True
            ):
                param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
                param_ty_ani_info.into_ani_boxed(
                    target,
                    "env",
                    arg_cpp,
                    arg_ani,
                )
            args_ani_str = ", ".join(args_ani)
            result_ani = "ani_result"
            target.writelns(
                f"ani_ref ani_argv[] = {{{args_ani_str}}};",
                f"ani_ref {result_ani} = {{}};",
                f"env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.ref.params)}, ani_argv, &{result_ani});",
            )
            if isinstance(return_ty := self.t.ref.return_ty, NonVoidType):
                result_cpp = "cpp_result"
                return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
                return_ty_ani_info.from_ani_boxed(
                    target,
                    "env",
                    result_ani,
                    result_cpp,
                )
                target.writelns(
                    f"return {result_cpp};",
                )
            else:
                target.writelns(
                    f"return;",
                )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_after: str,
    ):
        cpp_copy = f"{ani_after}_cpp_copy"
        cpp_scope = f"{ani_after}_cpp_scope"
        invoke_name = "invoke"
        ani_cast_ptr = f"{ani_after}_ani_cast_ptr"
        ani_func_ptr = f"{ani_after}_ani_func_ptr"
        ani_data_ptr = f"{ani_after}_ani_data_ptr"
        pkg_ani_info = PackageAniInfo.get(self.am, self.t.ref.parent_pkg)
        with target.indented(
            f"struct {cpp_scope} {{",
            f"}};",
        ):
            self.gen_native_invoke(target, invoke_name)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_copy} = {cpp_value};",
            f"ani_long {ani_cast_ptr} = reinterpret_cast<ani_long>(&{cpp_scope}::{invoke_name});",
            f"ani_long {ani_func_ptr} = reinterpret_cast<ani_long>({cpp_copy}.m_handle.vtbl_ptr);",
            f"ani_long {ani_data_ptr} = reinterpret_cast<ani_long>({cpp_copy}.m_handle.data_ptr);",
            f"{cpp_copy}.m_handle.data_ptr = nullptr;",
            f"ani_fn_object {ani_after} = {{}};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.mod.impl_desc}", "{pkg_ani_info.ns.mod.make_callback}", "lll:C{{std.core.Function0}}"), reinterpret_cast<ani_ref*>(&{ani_after}), {ani_cast_ptr}, {ani_func_ptr}, {ani_data_ptr});',
        )

    def gen_native_invoke(
        self,
        target: CSourceWriter,
        cpp_cast_ptr: str,
    ):
        params_ani = []
        args_ani = []
        params_ani.append("[[maybe_unused]] ani_env* env")
        params_ani.append("[[maybe_unused]] ani_long ani_func_ptr")
        params_ani.append("[[maybe_unused]] ani_long ani_data_ptr")
        for i in range(16):
            arg_ani = f"ani_arg_{i}"
            params_ani.append(f"[[maybe_unused]] ani_ref {arg_ani}")
            args_ani.append(arg_ani)
        params_ani_str = ", ".join(params_ani)
        vals_cpp = []
        for param in self.t.ref.params:
            val_cpp = f"cpp_arg_{param.name}"
            vals_cpp.append(val_cpp)
        return_ty_ani_name = "ani_ref"
        with target.indented(
            f"static {return_ty_ani_name} {cpp_cast_ptr}({params_ani_str}) {{",
            f"}};",
        ):
            target.writelns(
                f"{self.cpp_info.as_param}::vtable_type* cpp_vtbl_ptr = reinterpret_cast<{self.cpp_info.as_param}::vtable_type*>(ani_func_ptr);",
                f"DataBlockHead* cpp_data_ptr = reinterpret_cast<DataBlockHead*>(ani_data_ptr);",
                f"{self.cpp_info.as_param} cpp_func = {self.cpp_info.as_param}({{cpp_vtbl_ptr, cpp_data_ptr}});",
            )
            args_cpp = []
            for param, arg_ani, val_cpp in zip(
                self.t.ref.params, args_ani, vals_cpp, strict=False
            ):
                param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
                param_ty_ani_info.from_ani_boxed(
                    target,
                    "env",
                    arg_ani,
                    val_cpp,
                )
                param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                args_cpp.append(
                    f"std::forward<{param_ty_cpp_info.as_param}>({val_cpp})"
                )
            args_cpp_str = ", ".join(args_cpp)
            lambda_invoke = f"cpp_func({args_cpp_str})"
            if isinstance(return_ty := self.t.ref.return_ty, NonVoidType):
                return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
                result_cpp = "cpp_result"
                result_ani = "ani_result"
                target.writelns(
                    f"{return_ty_cpp_info.as_owner} {result_cpp} = {lambda_invoke};",
                    f"if (::taihe::has_error()) {{ return ani_ref{{}}; }}",
                )
                return_ty_ani_info.into_ani_boxed(
                    target,
                    "env",
                    result_cpp,
                    result_ani,
                )
                target.writelns(
                    f"return {result_ani};",
                )
            else:
                target.writelns(
                    f"{lambda_invoke};",
                    f"return ani_ref{{}};",
                )


class TypeAniInfoDispatcher(NonVoidTypeVisitor[TypeAniInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @override
    def visit_enum_type(self, t: EnumType) -> TypeAniInfo:
        if const_attr := ConstAttr.get(t.decl):
            return ConstEnumTypeAniInfo(self.am, t, const_attr)
        return EnumTypeAniInfo(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> TypeAniInfo:
        return UnionTypeAniInfo(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> TypeAniInfo:
        return StructTypeAniInfo(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> TypeAniInfo:
        return IfaceTypeAniInfo(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> TypeAniInfo:
        return OpaqueTypeAniInfo(self.am, t)

    @override
    def visit_unit_type(self, t: UnitType) -> TypeAniInfo:
        if literal_attr := LiteralAttr.get(t.ref):
            return StringLiteralTypeAniInfo(self.am, t, literal_attr)
        if UndefinedAttr.get(t.ref):
            return UndefinedTypeAniInfo(self.am, t)
        if NullAttr.get(t.ref):
            return NullTypeAniInfo(self.am, t)
        # TODO: compatible with older version, to be removed in future
        if isinstance(t.ref.parent_type_holder, StructFieldDecl | UnionFieldDecl):
            if UndefinedAttr.get(t.ref.parent_type_holder):
                return UndefinedTypeAniInfo(self.am, t)
            if NullAttr.get(t.ref.parent_type_holder):
                return NullTypeAniInfo(self.am, t)
        return NullTypeAniInfo(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> TypeAniInfo:
        return ScalarTypeAniInfo(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> TypeAniInfo:
        return StringTypeAniInfo(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> TypeAniInfo:
        if bigint_attr := BigIntAttr.get(t.ref):
            return BigIntTypeAniInfo(self.am, t, bigint_attr)
        if typedarray_attr := TypedArrayAttr.get(t.ref):
            return TypedArrayTypeAniInfo(self.am, t, typedarray_attr)
        if arraybuffer_attr := ArrayBufferAttr.get(t.ref):
            return ArrayBufferTypeAniInfo(self.am, t, arraybuffer_attr)
        if fixedarray_attr := FixedArrayAttr.get(t.ref):
            return FixedArrayTypeAniInfo(self.am, t, fixedarray_attr)
        return ArrayTypeAniInfo(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> TypeAniInfo:
        return OptionalTypeAniInfo(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> TypeAniInfo:
        if record_attr := RecordAttr.get(t.ref):
            return RecordTypeAniInfo(self.am, t, record_attr)
        raise NotImplementedError("MapType is not supported in ANI yet.")

    @override
    def visit_set_type(self, t: SetType) -> TypeAniInfo:
        raise NotImplementedError("SetType is not supported in ANI yet.")

    @override
    def visit_vector_type(self, t: VectorType) -> TypeAniInfo:
        raise NotImplementedError("VectorType is not supported in ANI yet.")

    @override
    def visit_callback_type(self, t: CallbackType) -> TypeAniInfo:
        return CallbackTypeAniInfo(self.am, t)