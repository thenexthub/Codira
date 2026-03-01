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

from taihe.codegen.abi.analyses import (
    GlobFuncAbiInfo,
    IfaceAbiInfo,
    IfaceMethodAbiInfo,
    PackageAbiInfo,
    StructAbiInfo,
    TypeAbiInfo,
    UnionAbiInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.semantics.declarations import (
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
    StructDecl,
    UnionDecl,
)
from taihe.semantics.types import NonVoidType
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


class AbiHeadersGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.all_packages:
            for struct in pkg.structs:
                AbiStructDeclGenerator(self.om, self.am, struct).gen_struct_decl_file()
                AbiStructDefnGenerator(self.om, self.am, struct).gen_struct_defn_file()
                AbiStructImplGenerator(self.om, self.am, struct).gen_struct_impl_file()
            for union in pkg.unions:
                AbiUnionDeclGenerator(self.om, self.am, union).gen_union_decl_file()
                AbiUnionDefnGenerator(self.om, self.am, union).gen_union_defn_file()
                AbiUnionImplGenerator(self.om, self.am, union).gen_union_impl_file()
            for iface in pkg.interfaces:
                AbiIfaceDeclGenerator(self.om, self.am, iface).gen_iface_decl_file()
                AbiIfaceDefnGenerator(self.om, self.am, iface).gen_iface_defn_file()
                AbiIfaceImplGenerator(self.om, self.am, iface).gen_iface_impl_file()
            AbiPackageHeaderGenerator(self.om, self.am, pkg).gen_package_file()


class AbiStructDeclGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, struct: StructDecl):
        self.om = om
        self.am = am
        self.struct = struct
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        self.target = CHeaderWriter(
            self.om,
            f"include/{struct_abi_info.decl_header}",
            FileKind.C_HEADER,
        )

    def gen_struct_decl_file(self):
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        with self.target:
            self.target.add_include("taihe/common.h")
            self.target.writelns(
                f"struct {struct_abi_info.mangled_name};",
            )


class AbiStructDefnGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, struct: StructDecl):
        self.om = om
        self.am = am
        self.struct = struct
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        self.target = CHeaderWriter(
            self.om,
            f"include/{struct_abi_info.defn_header}",
            FileKind.C_HEADER,
        )

    def gen_struct_defn_file(self):
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        with self.target:
            self.target.add_include(struct_abi_info.decl_header)
            for field in self.struct.fields:
                field_ty_abi_info = TypeAbiInfo.get(self.am, field.ty)
                self.target.add_include(*field_ty_abi_info.defn_headers)
            self.gen_struct_defn()

    def gen_struct_defn(self):
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        with self.target.indented(
            f"struct {struct_abi_info.mangled_name} {{",
            f"}};",
        ):
            for field in self.struct.fields:
                field_ty_abi_info = TypeAbiInfo.get(self.am, field.ty)
                self.target.writelns(
                    f"{field_ty_abi_info.as_owner} {field.name};",
                )


class AbiStructImplGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, struct: StructDecl):
        self.om = om
        self.am = am
        self.struct = struct
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        self.target = CHeaderWriter(
            self.om,
            f"include/{struct_abi_info.impl_header}",
            FileKind.C_HEADER,
        )

    def gen_struct_impl_file(self):
        struct_abi_info = StructAbiInfo.get(self.am, self.struct)
        with self.target:
            self.target.add_include(struct_abi_info.defn_header)
            for field in self.struct.fields:
                field_ty_abi_info = TypeAbiInfo.get(self.am, field.ty)
                self.target.add_include(*field_ty_abi_info.impl_headers)


class AbiUnionDeclGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, union: UnionDecl):
        self.om = om
        self.am = am
        self.union = union
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        self.target = CHeaderWriter(
            self.om,
            f"include/{union_abi_info.decl_header}",
            FileKind.C_HEADER,
        )

    def gen_union_decl_file(self):
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        with self.target:
            self.target.add_include("taihe/common.h")
            self.target.writelns(
                f"struct {union_abi_info.mangled_name};",
            )


class AbiUnionDefnGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, union: UnionDecl):
        self.om = om
        self.am = am
        self.union = union
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        self.target = CHeaderWriter(
            self.om,
            f"include/{union_abi_info.defn_header}",
            FileKind.C_HEADER,
        )

    def gen_union_defn_file(self):
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        with self.target:
            self.target.add_include(union_abi_info.decl_header)
            self.gen_union_defn()
            for field in self.union.fields:
                field_ty_abi_info = TypeAbiInfo.get(self.am, field.ty)
                self.target.add_include(*field_ty_abi_info.defn_headers)

    def gen_union_defn(self):
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        with self.target.indented(
            f"union {union_abi_info.union_name} {{",
            f"}};",
        ):
            for field in self.union.fields:
                field_ty_abi_info = TypeAbiInfo.get(self.am, field.ty)
                self.target.writelns(
                    f"{field_ty_abi_info.as_owner} {field.name};",
                )
        with self.target.indented(
            f"struct {union_abi_info.mangled_name} {{",
            f"}};",
        ):
            self.target.writelns(
                f"{union_abi_info.tag_type} m_tag;",
                f"union {union_abi_info.union_name} m_data;",
            )


class AbiUnionImplGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, union: UnionDecl):
        self.om = om
        self.am = am
        self.union = union
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        self.target = CHeaderWriter(
            self.om,
            f"include/{union_abi_info.impl_header}",
            FileKind.C_HEADER,
        )

    def gen_union_impl_file(self):
        union_abi_info = UnionAbiInfo.get(self.am, self.union)
        with self.target:
            self.target.add_include(union_abi_info.defn_header)
            for field in self.union.fields:
                field_ty_abi_info = TypeAbiInfo.get(self.am, field.ty)
                self.target.add_include(*field_ty_abi_info.impl_headers)


class AbiIfaceDeclGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_abi_info.decl_header}",
            FileKind.C_HEADER,
        )

    def gen_iface_decl_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include("taihe/object.abi.h")
            self.target.writelns(
                f"struct {iface_abi_info.mangled_name};",
            )


class AbiIfaceDefnGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_abi_info.defn_header}",
            FileKind.C_HEADER,
        )

    def gen_iface_defn_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include(iface_abi_info.decl_header)
            for ancestor, info in iface_abi_info.ancestor_dict.items():
                if ancestor is self.iface:
                    continue
                ancestor_abi_info = IfaceAbiInfo.get(self.am, ancestor)
                self.target.add_include(ancestor_abi_info.defn_header)
            self.gen_iface_vtable()
            self.gen_iface_defn()
            self.gen_iface_static_cast()
            self.gen_iface_dynamic_cast()

    def gen_iface_vtable(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        self.target.writelns(
            f"struct {iface_abi_info.ftable};",
        )
        with self.target.indented(
            f"struct {iface_abi_info.vtable} {{",
            f"}};",
        ):
            for ancestor_item_info in iface_abi_info.ancestor_list:
                ancestor_abi_info = IfaceAbiInfo.get(self.am, ancestor_item_info.iface)
                self.target.writelns(
                    f"struct {ancestor_abi_info.ftable} const* {ancestor_item_info.ftbl_ptr};",
                )

    def gen_iface_defn(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        self.target.writelns(
            f"TH_EXPORT void const* const {iface_abi_info.iid};",
        )
        with self.target.indented(
            f"struct {iface_abi_info.mangled_name} {{",
            f"}};",
        ):
            self.target.writelns(
                f"struct {iface_abi_info.vtable} const* vtbl_ptr;",
                f"struct DataBlockHead* data_ptr;",
            )

    def gen_iface_static_cast(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if ancestor is self.iface:
                continue
            ancestor_abi_info = IfaceAbiInfo.get(self.am, ancestor)
            with self.target.indented(
                f"TH_INLINE struct {ancestor_abi_info.vtable} const* {info.static_cast}(struct {iface_abi_info.vtable} const* vtbl_ptr) {{",
                f"}}",
            ):
                self.target.writelns(
                    f"return vtbl_ptr ? (struct {ancestor_abi_info.vtable} const*)((void* const*)vtbl_ptr + {info.offset}) : NULL;",
                )

    def gen_iface_dynamic_cast(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target.indented(
            f"TH_INLINE struct {iface_abi_info.vtable} const* {iface_abi_info.dynamic_cast}(struct TypeInfo const* rtti_ptr) {{",
            f"}}",
        ):
            with self.target.indented(
                f"for (size_t i = 0; i < rtti_ptr->len; i++) {{",
                f"}}",
            ):
                with self.target.indented(
                    f"if (rtti_ptr->idmap[i].id == {iface_abi_info.iid}) {{",
                    f"}}",
                ):
                    self.target.writelns(
                        f"return (struct {iface_abi_info.vtable} const*)rtti_ptr->idmap[i].vtbl_ptr;",
                    )
            self.target.writelns(
                f"return NULL;",
            )


class AbiIfaceImplGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_abi_info.impl_header}",
            FileKind.C_HEADER,
        )

    def gen_iface_impl_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include(iface_abi_info.defn_header)
            for method in self.iface.methods:
                for param in method.params:
                    param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
                    self.target.add_include(*param_ty_abi_info.defn_headers)
                if isinstance(return_ty := method.return_ty, NonVoidType):
                    param_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
                    self.target.add_include(*param_ty_abi_info.defn_headers)
            self.gen_iface_ftable()
            for method in self.iface.methods:
                self.gen_method(method)
                self.gen_method_call(method)
            for ancestor, info in iface_abi_info.ancestor_dict.items():
                if ancestor is self.iface:
                    continue
                ancestor_abi_info = IfaceAbiInfo.get(self.am, ancestor)
                self.target.add_include(ancestor_abi_info.impl_header)
            for method in self.iface.methods:
                for param in method.params:
                    return_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
                    self.target.add_include(*return_ty_abi_info.impl_headers)
                if isinstance(return_ty := method.return_ty, NonVoidType):
                    return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
                    self.target.add_include(*return_ty_abi_info.impl_headers)

    def gen_iface_ftable(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target.indented(
            f"struct {iface_abi_info.ftable} {{",
            f"}};",
        ):
            self.target.writelns(
                f"uint64_t version;",
            )
            with self.target.indented(
                f"struct {{",
                f"}} methods;",
            ):
                for method in self.iface.methods:
                    self.gen_iface_ftable_method(method)

    def gen_iface_ftable_method(self, method: IfaceMethodDecl):
        params = []
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        params.append(f"{iface_abi_info.as_param} tobj")
        for param in method.params:
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params.append(f"{param_ty_abi_info.as_param} {param.name}")
        params_str = ", ".join(params)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
        else:
            return_ty_abi_name = "void"
        self.target.writelns(
            f"{return_ty_abi_name} (*{method.name})({params_str});",
        )

    def gen_method_call(self, method: IfaceMethodDecl):
        method_abi_info = IfaceMethodAbiInfo.get(self.am, method)
        params = []
        args = []
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        params.append(f"{iface_abi_info.as_param} tobj")
        args.append("tobj")
        for param in method.params:
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params.append(f"{param_ty_abi_info.as_param} {param.name}")
            args.append(param.name)
        params_str = ", ".join(params)
        args_str = ", ".join(args)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
        else:
            return_ty_abi_name = "void"
        with self.target.indented(
            f"TH_INLINE {return_ty_abi_name} {method_abi_info.wrap_name}({params_str}) {{",
            f"}}",
        ):
            info = iface_abi_info.ancestor_dict[self.iface]
            with self.target.indented(
                f"if (0 >= {method_abi_info.min_version} || tobj.vtbl_ptr->{info.ftbl_ptr}->version >= {method_abi_info.min_version}) {{",
                f"}}",
            ):
                self.target.writelns(
                    f"return tobj.vtbl_ptr->{info.ftbl_ptr}->methods.{method.name}({args_str});",
                )
            with self.target.indented(
                f"else {{",
                f"}}",
            ):
                self.target.writelns(
                    f"return {method_abi_info.impl_name}({args_str});",
                )

    def gen_method(self, method: IfaceMethodDecl):
        method_abi_info = IfaceMethodAbiInfo.get(self.am, method)
        params = []
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        params.append(f"{iface_abi_info.as_param} tobj")
        for param in method.params:
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params.append(f"{param_ty_abi_info.as_param} {param.name}")
        params_str = ", ".join(params)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
        else:
            return_ty_abi_name = "void"
        self.target.writelns(
            f"TH_EXPORT {return_ty_abi_name} {method_abi_info.impl_name}({params_str});",
        )


class AbiPackageHeaderGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_abi_info = PackageAbiInfo.get(self.am, self.pkg)
        self.target = CHeaderWriter(
            self.om,
            f"include/{pkg_abi_info.header}",
            FileKind.C_HEADER,
        )

    def gen_package_file(self):
        with self.target:
            for struct in self.pkg.structs:
                struct_abi_info = StructAbiInfo.get(self.am, struct)
                self.target.add_include(struct_abi_info.impl_header)
            for union in self.pkg.unions:
                union_abi_info = UnionAbiInfo.get(self.am, union)
                self.target.add_include(union_abi_info.impl_header)
            for iface in self.pkg.interfaces:
                iface_abi_info = IfaceAbiInfo.get(self.am, iface)
                self.target.add_include(iface_abi_info.impl_header)
            self.target.add_include("taihe/common.h")
            for func in self.pkg.functions:
                for param in func.params:
                    param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
                    self.target.add_include(*param_ty_abi_info.impl_headers)
                if isinstance(return_ty := func.return_ty, NonVoidType):
                    return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
                    self.target.add_include(*return_ty_abi_info.impl_headers)
                self.gen_func(func)

    def gen_func(self, func: GlobFuncDecl):
        func_abi_info = GlobFuncAbiInfo.get(self.am, func)
        params = []
        for param in func.params:
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params.append(f"{param_ty_abi_info.as_param} {param.name}")
        params_str = ", ".join(params)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
        else:
            return_ty_abi_name = "void"
        self.target.writelns(
            f"TH_EXPORT {return_ty_abi_name} {func_abi_info.impl_name}({params_str});",
        )


class AbiSourcesGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.all_packages:
            AbiPackageSourceGenerator(self.om, self.am, pkg).gen_package_file()


class AbiPackageSourceGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_abi_info = PackageAbiInfo.get(self.am, self.pkg)
        self.target = CSourceWriter(
            self.om,
            f"src/{pkg_abi_info.source}",
            FileKind.C_SOURCE,
        )

    def gen_package_file(self):
        with self.target:
            for iface in self.pkg.interfaces:
                iface_abi_info = IfaceAbiInfo.get(self.am, iface)
                self.target.add_include(iface_abi_info.defn_header)
                self.gen_iface_iid(iface)

    def gen_iface_iid(self, iface: IfaceDecl):
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        self.target.writelns(
            f"void const* const {iface_abi_info.iid} = &{iface_abi_info.iid};",
        )