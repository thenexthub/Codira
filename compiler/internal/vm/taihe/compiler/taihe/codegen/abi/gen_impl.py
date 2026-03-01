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
    GlobFuncCImplInfo,
    IfaceAbiInfo,
    IfaceCImplInfo,
    IfaceMethodAbiInfo,
    IfaceMethodCImplInfo,
    PackageAbiInfo,
    PackageCImplInfo,
    TypeAbiInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.semantics.declarations import (
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
)
from taihe.semantics.types import NonVoidType
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


class CImplHeadersGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            CMacroPackageGenerator(self.om, self.am, pkg).gen_package_file()
            for iface in pkg.interfaces:
                CMacroIfaceGenerator(self.om, self.am, iface).gen_iface_file()


class CMacroPackageGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_c_impl_info = PackageCImplInfo.get(self.am, self.pkg)
        self.target = CHeaderWriter(
            self.om,
            f"include/{pkg_c_impl_info.header}",
            FileKind.C_HEADER,
        )

    def gen_package_file(self):
        pkg_abi_info = PackageAbiInfo.get(self.am, self.pkg)
        with self.target:
            self.target.add_include("taihe/common.h", pkg_abi_info.header)
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
        func_c_impl_info = GlobFuncCImplInfo.get(self.am, func)
        func_impl = "C_FUNC_IMPL"
        params = []
        args = []
        for param in func.params:
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params.append(f"{param_ty_abi_info.as_param} {param.name}")
            args.append(param.name)
        params_str = ", ".join(params)
        args_str = ", ".join(args)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
        else:
            return_ty_abi_name = "void"
        self.target.writelns(
            f"#define {func_c_impl_info.macro}({func_impl}) \\",
            f"    {return_ty_abi_name} {func_abi_info.impl_name}({params_str}) {{ \\",
            f"        return {func_impl}({args_str}); \\",
            f"    }}",
        )


class CMacroIfaceGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_c_impl_info = IfaceCImplInfo.get(self.am, self.iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_c_impl_info.header}",
            FileKind.C_HEADER,
        )

    def gen_iface_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include("taihe/common.h", iface_abi_info.impl_header)
            for method in self.iface.methods:
                for param in method.params:
                    param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
                    self.target.add_include(*param_ty_abi_info.impl_headers)
                if isinstance(return_ty := method.return_ty, NonVoidType):
                    return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
                    self.target.add_include(*return_ty_abi_info.impl_headers)
                self.gen_method(method)

    def gen_method(self, method: IfaceMethodDecl):
        method_abi_info = IfaceMethodAbiInfo.get(self.am, method)
        method_c_impl_info = IfaceMethodCImplInfo.get(self.am, method)
        method_impl = "C_METHOD_IMPL"
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
        self.target.writelns(
            f"#define {method_c_impl_info.macro}({method_impl}) \\",
            f"    {return_ty_abi_name} {method_abi_info.impl_name}({params_str}) {{ \\",
            f"        return {method_impl}({args_str}); \\",
            f"    }}",
        )


class CImplSourcesGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            CTemplatePackageGenerator(self.om, self.am, pkg).gen_package_file()
            for iface in pkg.interfaces:
                CTemplateIfaceGenerator(self.om, self.am, iface).gen_iface_file()


class CTemplatePackageGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_c_impl_info = PackageCImplInfo.get(self.am, self.pkg)
        self.target = CSourceWriter(
            self.om,
            f"temp/{pkg_c_impl_info.source}",
            FileKind.TEMPLATE,
        )

    def gen_package_file(self):
        pkg_c_impl_info = PackageCImplInfo.get(self.am, self.pkg)
        with self.target:
            self.target.add_include(pkg_c_impl_info.header)
            for func in self.pkg.functions:
                self.gen_func(func)

    def gen_func(self, func: GlobFuncDecl):
        func_c_impl_info = GlobFuncCImplInfo.get(self.am, func)
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
        with self.target.indented(
            f"{return_ty_abi_name} {func_c_impl_info.function}({params_str}) {{",
            f"}}",
        ):
            self.target.writelns(
                f"// TODO",
            )
        self.target.writelns(
            f"{func_c_impl_info.macro}({func_c_impl_info.function});",
        )


class CTemplateIfaceGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_c_impl_info = IfaceCImplInfo.get(self.am, self.iface)
        self.target = CSourceWriter(
            self.om,
            f"temp/{iface_c_impl_info.source}",
            FileKind.TEMPLATE,
        )

    def gen_iface_file(self):
        iface_c_impl_info = IfaceCImplInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include(iface_c_impl_info.header)
            for method in self.iface.methods:
                self.gen_method(method)

    def gen_method(self, method: IfaceMethodDecl):
        method_c_impl_info = IfaceMethodCImplInfo.get(self.am, method)
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
        with self.target.indented(
            f"{return_ty_abi_name} {method_c_impl_info.function}({params_str}) {{",
            f"}}",
        ):
            self.target.writelns(
                f"// TODO",
            )
        self.target.writelns(
            f"{method_c_impl_info.macro}({method_c_impl_info.function});",
        )