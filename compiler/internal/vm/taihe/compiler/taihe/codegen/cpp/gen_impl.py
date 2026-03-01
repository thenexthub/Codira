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

import re

from taihe.codegen.abi.analyses import (
    GlobFuncAbiInfo,
    IfaceAbiInfo,
    IfaceMethodAbiInfo,
    PackageAbiInfo,
    TypeAbiInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.codegen.cpp.analyses import (
    GlobFuncCppImplInfo,
    IfaceCppImplInfo,
    IfaceCppInfo,
    IfaceMethodCppImplInfo,
    IfaceMethodCppInfo,
    PackageCppImplInfo,
    TypeCppInfo,
    from_abi,
    into_abi,
)
from taihe.semantics.declarations import (
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
)
from taihe.semantics.types import IfaceType, NonVoidType
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


class CppImplHeadersGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            CppMacroPackageGenerator(self.om, self.am, pkg).gen_package_file()


class CppMacroPackageGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_cpp_impl_info = PackageCppImplInfo.get(self.am, pkg)
        self.target = CHeaderWriter(
            self.om,
            f"include/{pkg_cpp_impl_info.header}",
            FileKind.CPP_HEADER,
        )

    def gen_package_file(self):
        pkg_abi_info = PackageAbiInfo.get(self.am, self.pkg)
        with self.target:
            self.target.add_include("taihe/common.hpp")
            self.target.add_include(pkg_abi_info.header)
            for func in self.pkg.functions:
                for param in func.params:
                    param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                    self.target.add_include(*param_ty_cpp_info.impl_headers)
                if isinstance(return_ty := func.return_ty, NonVoidType):
                    return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                    self.target.add_include(*return_ty_cpp_info.impl_headers)
                self.gen_func(func)

    def gen_func(self, func: GlobFuncDecl):
        func_abi_info = GlobFuncAbiInfo.get(self.am, func)
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        func_impl = "CPP_FUNC_IMPL"
        params_abi = []
        args_cpp = []
        for param in func.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params_abi.append(f"{param_ty_abi_info.as_param} {param.name}")
            args_cpp.append(from_abi(param_ty_cpp_info.as_param, param.name))
        params_abi_str = ", ".join(params_abi)
        args_cpp_str = ", ".join(args_cpp)
        result_cpp = f"{func_impl}({args_cpp_str})"
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
            result_abi = into_abi(return_ty_cpp_info.as_owner, result_cpp)
        else:
            return_ty_abi_name = "void"
            result_abi = result_cpp
        self.target.writelns(
            f"#define {func_cpp_impl_info.macro}({func_impl}) \\",
            f"    {return_ty_abi_name} {func_abi_info.impl_name}({params_abi_str}) {{ \\",
            f"        return {result_abi}; \\",
            f"    }}",
        )


class CppMacroIfaceGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_cpp_impl_info.header}",
            FileKind.CPP_HEADER,
        )

    def gen_iface_file(self):
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include("taihe/common.hpp")
            self.target.add_include(iface_cpp_info.impl_header)
            for method in self.iface.methods:
                for param in method.params:
                    param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                    self.target.add_include(*param_ty_cpp_info.impl_headers)
                if isinstance(return_ty := method.return_ty, NonVoidType):
                    return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                    self.target.add_include(*return_ty_cpp_info.impl_headers)
                self.gen_method(method)

    def gen_method(self, method: IfaceMethodDecl):
        method_abi_info = IfaceMethodAbiInfo.get(self.am, method)
        method_cpp_impl_info = IfaceMethodCppImplInfo.get(self.am, method)
        method_impl = "CPP_METHOD_IMPL"
        params_abi = []
        args_cpp = []
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        params_abi.append(f"{iface_abi_info.as_param} tobj")
        args_cpp.append(from_abi(iface_cpp_info.as_param, "tobj"))
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params_abi.append(f"{param_ty_abi_info.as_param} {param.name}")
            args_cpp.append(from_abi(param_ty_cpp_info.as_param, param.name))
        params_abi_str = ", ".join(params_abi)
        args_cpp_str = ", ".join(args_cpp)
        result_cpp = f"{method_impl}({args_cpp_str})"
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
            result_abi = into_abi(return_ty_cpp_info.as_owner, result_cpp)
        else:
            return_ty_abi_name = "void"
            result_abi = result_cpp
        self.target.writelns(
            f"#define {method_cpp_impl_info.macro}({method_impl}) \\",
            f"    {return_ty_abi_name} {method_abi_info.impl_name}({params_abi_str}) {{ \\",
            f"        return {result_abi}; \\",
            f"    }}",
        )


class CppImplSourcesGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am
        self.using_namespaces: list[str] = []

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            CppTemplatePackageGenerator(self.om, self.am, pkg).gen_package_file()
        for pkg in pg.packages:
            for iface in pkg.interfaces:
                CppTemplateClassHeaderGenerator(self.om, self.am, iface).gen_file()
                CppTemplateClassSourceGenerator(self.om, self.am, iface).gen_file()


class CppTemplateBaseWriterGenerator:
    def __init__(
        self,
        om: OutputManager,
        am: AnalysisManager,
        target: CSourceWriter,
        using_namespaces: list[str],
    ):
        self.om = om
        self.am = am
        self.target = target
        self.using_namespaces = using_namespaces

    @property
    def make_holder(self):
        return self.mask("taihe::make_holder")

    @property
    def runtime_error(self):
        return self.mask("std::runtime_error")

    def mask(self, cpp_type: str):
        pattern = r"(::)?([A-Za-z_][A-Za-z_0-9]*::)*[A-Za-z_][A-Za-z_0-9]*"

        def replace_ns(match):
            matched = match.group(0)
            for ns in self.using_namespaces:
                ns = ns + "::"
                if matched.startswith(ns):
                    return matched[len(ns) :]
                ns = "::" + ns
                if matched.startswith(ns):
                    return matched[len(ns) :]
            return matched

        return re.sub(pattern, replace_ns, cpp_type)

    def gen_using_namespaces(self):
        for namespace in self.using_namespaces:
            self.target.writelns(
                f"using namespace {namespace};",
            )


class CppTemplatePackageGenerator(CppTemplateBaseWriterGenerator):
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.pkg = pkg
        pkg_cpp_impl_info = PackageCppImplInfo.get(am, pkg)
        target = CSourceWriter(
            om,
            f"temp/{pkg_cpp_impl_info.source}",
            FileKind.TEMPLATE,
        )
        super().__init__(om, am, target, [])

    def gen_package_file(self):
        pkg_cpp_impl_info = PackageCppImplInfo.get(self.am, self.pkg)
        with self.target:
            self.target.add_include(pkg_cpp_impl_info.header)
            self.target.add_include("stdexcept")
            self.target.newline()
            with self.target.indented(
                f"namespace {{",
                f"}}  // namespace",
                indent="",
            ):
                self.gen_using_namespaces()
                for func in self.pkg.functions:
                    self.target.newline()
                    self.gen_func_impl(func)
            self.target.newline()
            self.target.writelns(
                "// Since these macros are auto-generate, lint will cause false positive.",
                "// NOLINTBEGIN",
            )
            for func in self.pkg.functions:
                self.gen_func_macro(func)
            self.target.writelns(
                "// NOLINTEND",
            )

    def gen_func_impl(self, func: GlobFuncDecl):
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        params_cpp = []
        for param in func.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with self.target.indented(
            f"{return_ty_cpp_name} {func_cpp_impl_info.function}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := func.return_ty, IfaceType):
                ret_cpp_impl_info = IfaceCppImplInfo.get(self.am, return_ty.decl)
                self.target.add_include(ret_cpp_impl_info.template_header)
                self.target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{ret_cpp_impl_info.template_class}, {return_ty_cpp_name}>();",
                )
            else:
                self.target.writelns(
                    f'TH_THROW({self.runtime_error}, "not implemented");',
                )

    def gen_func_macro(self, func: GlobFuncDecl):
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        self.target.writelns(
            f"{func_cpp_impl_info.macro}({func_cpp_impl_info.function});",
        )


class CppTemplateIfaceGenerator(CppTemplateBaseWriterGenerator):
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.iface = iface
        iface_cpp_impl_info = IfaceCppImplInfo.get(am, iface)
        target = CSourceWriter(
            om,
            f"temp/{iface_cpp_impl_info.source}",
            FileKind.TEMPLATE,
        )
        super().__init__(om, am, target, [])

    def gen_iface_file(self):
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include(iface_cpp_impl_info.header)
            self.target.add_include("stdexcept")
            self.target.newline()
            with self.target.indented(
                f"namespace {{",
                f"}}  // namespace",
                indent="",
            ):
                self.gen_using_namespaces()
                for method in self.iface.methods:
                    self.target.newline()
                    self.gen_method_impl(method)
            self.target.newline()
            self.target.writelns(
                "// Since these macros are auto-generate, lint will cause false positive.",
                "// NOLINTBEGIN",
            )
            for method in self.iface.methods:
                self.gen_method_macro(method)
            self.target.writelns(
                "// NOLINTEND",
            )

    def gen_method_impl(self, method: IfaceMethodDecl):
        method_cpp_impl_info = IfaceMethodCppImplInfo.get(self.am, method)
        params_cpp = []
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        params_cpp.append(f"{self.mask(iface_cpp_info.as_param)} tobj")
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with self.target.indented(
            f"{return_ty_cpp_name} {method_cpp_impl_info.function}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := method.return_ty, IfaceType):
                ret_cpp_impl_info = IfaceCppImplInfo.get(self.am, return_ty.decl)
                self.target.add_include(ret_cpp_impl_info.template_header)
                self.target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{ret_cpp_impl_info.template_class}, {return_ty_cpp_name}>();",
                )
            else:
                self.target.writelns(
                    f'TH_THROW({self.runtime_error}, "not implemented");',
                )

    def gen_method_macro(self, method: IfaceMethodDecl):
        method_cpp_impl_info = IfaceMethodCppImplInfo.get(self.am, method)
        self.target.writelns(
            f"{method_cpp_impl_info.macro}({method_cpp_impl_info.function});",
        )


class CppTemplateClassHeaderGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        self.target = CHeaderWriter(
            self.om,
            f"temp/{iface_cpp_impl_info.template_header}",
            FileKind.TEMPLATE,
        )

    def gen_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include("taihe/common.hpp")
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    for param in method.params:
                        param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                        self.target.add_include(*param_ty_cpp_info.impl_headers)
                    if isinstance(return_ty := method.return_ty, NonVoidType):
                        return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                        self.target.add_include(*return_ty_cpp_info.impl_headers)
            self.target.newline()
            self.gen_iface_template_class()

    def gen_iface_template_class(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, self.iface)
        with self.target.indented(
            f"class {iface_cpp_impl_info.template_class} {{",
            f"}};",
        ):
            self.target.writelns(
                f"public:",
                f"// You can add member variables and constructor here.",
            )
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    self.gen_iface_method_decl(method)

    def gen_iface_method_decl(self, method: IfaceMethodDecl):
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        params_cpp = []
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{param_ty_cpp_info.as_param} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = return_ty_cpp_info.as_owner
        else:
            return_ty_cpp_name = "void"
        self.target.writelns(
            f"{return_ty_cpp_name} {method_cpp_info.call_name}({params_cpp_str});",
        )


class CppTemplateClassSourceGenerator(CppTemplateBaseWriterGenerator):
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.iface = iface
        iface_cpp_impl_info = IfaceCppImplInfo.get(am, iface)
        target = CSourceWriter(
            om,
            f"temp/{iface_cpp_impl_info.template_source}",
            FileKind.TEMPLATE,
        )
        super().__init__(om, am, target, [])

    def gen_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include(iface_cpp_impl_info.template_header)
            self.target.add_include("stdexcept")
            self.target.newline()
            self.gen_using_namespaces()
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    self.target.newline()
                    self.gen_iface_method_impl(method)

    def gen_iface_method_impl(self, method: IfaceMethodDecl):
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, self.iface)
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        params_cpp = []
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with self.target.indented(
            f"{return_ty_cpp_name} {iface_cpp_impl_info.template_class}::{method_cpp_info.impl_name}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := method.return_ty, IfaceType):
                ret_cpp_impl_info = IfaceCppImplInfo.get(self.am, return_ty.decl)
                self.target.add_include(ret_cpp_impl_info.template_header)
                self.target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{ret_cpp_impl_info.template_class}, {return_ty_cpp_name}>();",
                )
            else:
                self.target.writelns(
                    f'TH_THROW({self.runtime_error}, "not implemented");',
                )