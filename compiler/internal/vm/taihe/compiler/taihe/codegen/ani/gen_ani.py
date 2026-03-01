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
    IfaceAbiInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.codegen.ani.analyses import (
    AniScope,
    GlobFuncAniInfo,
    IfaceAniInfo,
    IfaceMethodAniInfo,
    PackageAniInfo,
    StructAniInfo,
    TypeAniInfo,
    UnionAniInfo,
)
from taihe.codegen.cpp.analyses import (
    GlobFuncCppUserInfo,
    IfaceCppInfo,
    IfaceMethodCppInfo,
    PackageCppUserInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
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


class AniCodeGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            for iface in pkg.interfaces:
                AniIfaceDeclGenerator(self.om, self.am, iface).gen_iface_decl_file()
                AniIfaceImplGenerator(self.om, self.am, iface).gen_iface_impl_file()
            for struct in pkg.structs:
                AniStructDeclGenerator(self.om, self.am, struct).gen_struct_decl_file()
                AniStructImplGenerator(self.om, self.am, struct).gen_struct_impl_file()
            for union in pkg.unions:
                AniUnionDeclGenerator(self.om, self.am, union).gen_union_decl_file()
                AniUnionImplGenerator(self.om, self.am, union).gen_union_impl_file()
            AniPackageHeaderGenerator(self.om, self.am, pkg).gen_package_header()
            AniPackageSourceGenerator(self.om, self.am, pkg).gen_package_source()
        AniConstructorGenerator(self.om, self.am, pg).gen_constructor()


class AniConstructorGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pg: PackageGroup):
        self.om = om
        self.am = am
        self.pg = pg
        self.target = CSourceWriter(
            self.om,
            f"temp/ani_constructor.cpp",
            FileKind.TEMPLATE,
        )

    def gen_constructor(self):
        with self.target:
            self.target.writelns(
                f"#if __has_include(<ani.h>)",
                f"#include <ani.h>",
                f"#elif __has_include(<ani/ani.h>)",
                f"#include <ani/ani.h>",
                f"#else",
                f'#error "ani.h not found. Please ensure the Ani SDK is correctly installed."',
                f"#endif",
            )
            with self.target.indented(
                f"ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {{",
                f"}}",
            ):
                self.target.writelns(
                    # f"::taihe::set_vm(vm);",
                    f"ani_env *env;",
                )
                with self.target.indented(
                    f"if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {{",
                    f"}}",
                ):
                    self.target.writelns(
                        f"return ANI_ERROR;",
                    )
                self.target.writelns(
                    f"ani_status status = ANI_OK;",
                )
                for pkg in self.pg.packages:
                    pkg_ani_info = PackageAniInfo.get(self.am, pkg)
                    self.target.add_include(pkg_ani_info.header)
                    with self.target.indented(
                        f"if (ANI_OK != {pkg_ani_info.cpp_ns}::ANIRegister(env)) {{",
                        f"}}",
                    ):
                        self.target.writelns(
                            f'std::cerr << "Error from {pkg_ani_info.cpp_ns}::ANIRegister" << std::endl;',
                            f"status = ANI_ERROR;",
                        )
                self.target.writelns(
                    f"*result = ANI_VERSION_1;",
                    f"return status;",
                )


class AniPackageHeaderGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_ani_info = PackageAniInfo.get(self.am, self.pkg)
        self.target = CHeaderWriter(
            self.om,
            f"include/{pkg_ani_info.header}",
            FileKind.CPP_HEADER,
        )

    def gen_package_header(self):
        pkg_ani_info = PackageAniInfo.get(self.am, self.pkg)
        with self.target:
            self.target.add_include("taihe/platform/ani.hpp")
            with self.target.indented(
                f"namespace {pkg_ani_info.cpp_ns} {{",
                f"}}",
                indent="",
            ):
                self.target.writelns(
                    f"ani_status ANIRegister(ani_env *env);",
                )


class AniPackageSourceGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, pkg: PackageDecl):
        self.om = om
        self.am = am
        self.pkg = pkg
        pkg_ani_info = PackageAniInfo.get(self.am, self.pkg)
        self.target = CSourceWriter(
            self.om,
            f"src/{pkg_ani_info.source}",
            FileKind.CPP_SOURCE,
        )

    def gen_package_source(self):
        pkg_ani_info = PackageAniInfo.get(self.am, self.pkg)
        with self.target:
            pkg_cpp_user_info = PackageCppUserInfo.get(self.am, self.pkg)
            self.target.add_include("taihe/object.hpp")
            self.target.add_include(pkg_ani_info.header)
            self.target.add_include(pkg_cpp_user_info.header)
            subregisters = self.gen_bindings()
            self.gen_package_register(subregisters)

    def gen_package_register(self, subregisters: list[str]):
        pkg_ani_info = PackageAniInfo.get(self.am, self.pkg)
        with self.target.indented(
            f"namespace {pkg_ani_info.cpp_ns} {{",
            f"}}",
            indent="",
        ):
            with self.target.indented(
                f"ani_status ANIRegister(ani_env *env) {{",
                f"}}",
            ):
                # TODO: set_vm in constructor
                with self.target.indented(
                    f"if (::taihe::get_vm() == nullptr) {{",
                    f"}}",
                ):
                    self.target.writelns(
                        f"ani_vm *vm;",
                    )
                    with self.target.indented(
                        f"if (ANI_OK != env->GetVM(&vm)) {{",
                        f"}}",
                    ):
                        self.target.writelns(
                            f"return ANI_ERROR;",
                        )
                    self.target.writelns(
                        f"::taihe::set_vm(vm);",
                    )
                self.target.writelns(
                    f"ani_status status = ANI_OK;",
                )
                for subregister in subregisters:
                    with self.target.indented(
                        f"if (ani_status ret = {subregister}(env); ret != ANI_OK && ret != ANI_ALREADY_BINDED) {{",
                        f"}}",
                    ):
                        self.target.writelns(
                            f'std::cerr << "Error from {subregister}, code: " << ret << std::endl;',
                            f"status = ANI_ERROR;",
                        )
                self.target.writelns(
                    f"return status;",
                )

    def gen_bindings(self):
        pkg_ani_info = PackageAniInfo.get(self.am, self.pkg)

        subregisters: list[str] = []

        utils_namespace = "local"
        utils_register_name = "ANIUtilsRegister"
        with self.target.indented(
            f"namespace {utils_namespace} {{",
            f"}}",
            indent="",
        ):
            mod_member_infos: dict[str, str] = {}
            obj_drop_cpp_name = "_obj_drop"
            self.gen_obj_drop(obj_drop_cpp_name)
            mod_member_infos.setdefault(
                pkg_ani_info.ns.mod.obj_drop,
                f"{utils_namespace}::{obj_drop_cpp_name}",
            )
            obj_dup_cpp_name = "_obj_dup"
            self.gen_obj_dup(obj_dup_cpp_name)
            mod_member_infos.setdefault(
                pkg_ani_info.ns.mod.obj_dup,
                f"{utils_namespace}::{obj_dup_cpp_name}",
            )
            native_invoke_cpp_name = "_native_invoke"
            self.gen_native_invoke(native_invoke_cpp_name)
            mod_member_infos.setdefault(
                pkg_ani_info.ns.mod.native_invoke,
                f"{utils_namespace}::{native_invoke_cpp_name}",
            )
            self.gen_subregister(
                utils_register_name,
                parent_scope=pkg_ani_info.ns.mod.scope,
                impl_desc=pkg_ani_info.ns.mod.impl_desc,
                member_infos=mod_member_infos,
            )
            subregisters.append(f"{utils_namespace}::{utils_register_name}")

        funcs_namespace = "local"
        funcs_register_name = "ANIFuncsRegister"
        with self.target.indented(
            f"namespace {funcs_namespace} {{",
            f"}}",
            indent="",
        ):
            pkg_member_infos: dict[str, str] = {}
            for func in self.pkg.functions:
                self.gen_native_func(func.name, func)
                func_ani_info = GlobFuncAniInfo.get(self.am, func)
                pkg_member_infos.setdefault(
                    func_ani_info.native_name,
                    f"{funcs_namespace}::{func.name}",
                )
            self.gen_subregister(
                funcs_register_name,
                parent_scope=pkg_ani_info.ns.scope,
                impl_desc=pkg_ani_info.ns.impl_desc,
                member_infos=pkg_member_infos,
            )
            subregisters.append(f"{funcs_namespace}::{funcs_register_name}")

        for iface in self.pkg.interfaces:
            methods_namespace = f"local::{iface.name}"
            methods_register_name = "ANIMethodsRegister"
            with self.target.indented(
                f"namespace {methods_namespace} {{",
                f"}}",
                indent="",
            ):
                iface_abi_info = IfaceAbiInfo.get(self.am, iface)
                iface_ani_info = IfaceAniInfo.get(self.am, iface)
                iface_member_infos: dict[str, str] = {}
                for ancestor in iface_abi_info.ancestor_dict:
                    for method in ancestor.methods:
                        self.gen_native_method(method.name, method, iface, ancestor)
                        method_ani_info = IfaceMethodAniInfo.get(self.am, method)
                        iface_member_infos.setdefault(
                            method_ani_info.native_name,
                            f"{methods_namespace}::{method.name}",
                        )
                self.gen_subregister(
                    methods_register_name,
                    parent_scope=iface_ani_info.scope,
                    impl_desc=iface_ani_info.impl_desc,
                    member_infos=iface_member_infos,
                )
                subregisters.append(f"{methods_namespace}::{methods_register_name}")

        return subregisters

    def gen_subregister(
        self,
        register_name: str,
        parent_scope: AniScope,
        impl_desc: str,
        member_infos: dict[str, str],
    ):
        with self.target.indented(
            f"static ani_status {register_name}(ani_env *env) {{",
            f"}}",
        ):
            self.target.writelns(
                f"{parent_scope} scope;",
            )
            with self.target.indented(
                f'if (ANI_OK != env->Find{parent_scope.suffix}("{impl_desc}", &scope)) {{',
                f"}}",
            ):
                self.target.writelns(
                    f"return ANI_ERROR;",
                )
            with self.target.indented(
                f"ani_native_function methods[] = {{",
                f"}};",
            ):
                for sts_name, cpp_name in member_infos.items():
                    self.target.writelns(
                        f'{{"{sts_name}", nullptr, reinterpret_cast<void*>({cpp_name})}},',
                    )
            self.target.writelns(
                f"return env->{parent_scope.suffix}_BindNative{parent_scope.member.suffix}s(scope, methods, sizeof(methods) / sizeof(ani_native_function));",
            )

    def gen_native_func(
        self,
        name: str,
        func: GlobFuncDecl,
    ):
        func_cpp_user_info = GlobFuncCppUserInfo.get(self.am, func)
        params_ani = ["[[maybe_unused]] ani_env *env"]
        args_ani = []
        vals_cpp = []
        for param in func.params:
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            arg_ani = f"ani_arg_{param.name}"
            val_cpp = f"cpp_val_{param.name}"
            params_ani.append(f"{param_ty_ani_info.ani_type} {arg_ani}")
            args_ani.append(arg_ani)
            vals_cpp.append(val_cpp)
        params_ani_str = ", ".join(params_ani)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_ani_name = return_ty_ani_info.ani_type
        else:
            return_ty_ani_name = "void"
        with self.target.indented(
            f"static {return_ty_ani_name} {name}({params_ani_str}) {{",
            f"}}",
        ):
            args_cpp = []
            for param, arg_ani, val_cpp in zip(
                func.params, args_ani, vals_cpp, strict=True
            ):
                param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
                param_ty_ani_info.from_ani(
                    self.target,
                    "env",
                    arg_ani,
                    val_cpp,
                )
                param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                args_cpp.append(
                    f"std::forward<{param_ty_cpp_info.as_param}>({val_cpp})"
                )
            args_cpp_str = ", ".join(args_cpp)
            function_call = f"{func_cpp_user_info.full_name}({args_cpp_str})"
            if isinstance(return_ty := func.return_ty, NonVoidType):
                return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
                return_ty_cpp_name = return_ty_cpp_info.as_owner
                result_cpp = "cpp_result"
                result_ani = "ani_result"
                self.target.writelns(
                    f"{return_ty_cpp_name} {result_cpp} = {function_call};",
                    f"if (::taihe::has_error()) {{ return {return_ty_ani_info.ani_type}{{}}; }}",
                )
                return_ty_ani_info.into_ani(
                    self.target,
                    "env",
                    result_cpp,
                    result_ani,
                )
                self.target.writelns(
                    f"return {result_ani};",
                )
            else:
                self.target.writelns(
                    f"{function_call};",
                )

    def gen_native_method(
        self,
        name: str,
        method: IfaceMethodDecl,
        iface: IfaceDecl,
        ancestor: IfaceDecl,
    ):
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        iface_ani_info = IfaceAniInfo.get(self.am, iface)
        ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
        params_ani = ["[[maybe_unused]] ani_env *env"]
        params_ani.append("[[maybe_unused]] ani_object object")
        args_ani = []
        vals_cpp = []
        for param in method.params:
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            arg_ani = f"ani_arg_{param.name}"
            val_cpp = f"cpp_val_{param.name}"
            params_ani.append(f"{param_ty_ani_info.ani_type} {arg_ani}")
            args_ani.append(arg_ani)
            vals_cpp.append(val_cpp)
        params_ani_str = ", ".join(params_ani)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_ani_name = return_ty_ani_info.ani_type
        else:
            return_ty_ani_name = "void"
        with self.target.indented(
            f"static {return_ty_ani_name} {name}({params_ani_str}) {{",
            f"}}",
        ):
            self.target.writelns(
                f"ani_long ani_data_ptr = {{}};",
                f'env->Object_GetField_Long(object, TH_ANI_FIND_CLASS_FIELD(env, "{iface_ani_info.impl_desc}", "{iface_ani_info.data_ptr}"), &ani_data_ptr);',
                f"ani_long ani_vtbl_ptr = {{}};",
                f'env->Object_GetField_Long(object, TH_ANI_FIND_CLASS_FIELD(env, "{iface_ani_info.impl_desc}", "{iface_ani_info.vtbl_ptr}"), &ani_vtbl_ptr);',
                f"DataBlockHead* cpp_data_ptr = reinterpret_cast<DataBlockHead*>(ani_data_ptr);",
                f"{iface_abi_info.vtable}* cpp_vtbl_ptr = reinterpret_cast<{iface_abi_info.vtable}*>(ani_vtbl_ptr);",
                f"{iface_cpp_info.full_weak_name} cpp_iface = {iface_cpp_info.full_weak_name}({{cpp_vtbl_ptr, cpp_data_ptr}});",
            )
            args_cpp = []
            for param, arg_ani, val_cpp in zip(
                method.params, args_ani, vals_cpp, strict=True
            ):
                return_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
                return_ty_ani_info.from_ani(
                    self.target,
                    "env",
                    arg_ani,
                    val_cpp,
                )
                param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                args_cpp.append(
                    f"std::forward<{param_ty_cpp_info.as_param}>({val_cpp})"
                )
            args_cpp_str = ", ".join(args_cpp)
            method_call = f"{ancestor_cpp_info.as_param}(cpp_iface)->{method_cpp_info.call_name}({args_cpp_str})"
            if isinstance(return_ty := method.return_ty, NonVoidType):
                return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
                return_ty_cpp_name = return_ty_cpp_info.as_owner
                result_cpp = "cpp_result"
                result_ani = "ani_result"
                self.target.writelns(
                    f"{return_ty_cpp_name} {result_cpp} = {method_call};",
                    f"if (::taihe::has_error()) {{ return {return_ty_ani_info.ani_type}{{}}; }}",
                )
                return_ty_ani_info.into_ani(
                    self.target,
                    "env",
                    result_cpp,
                    result_ani,
                )
                self.target.writelns(
                    f"return {result_ani};",
                )
            else:
                self.target.writelns(
                    f"{method_call};",
                )

    def gen_obj_drop(self, name: str):
        with self.target.indented(
            f"static void {name}([[maybe_unused]] ani_env *env, ani_long data_ptr) {{",
            f"}}",
        ):
            self.target.writelns(
                f"tobj_drop(reinterpret_cast<DataBlockHead*>(data_ptr));",
            )

    def gen_obj_dup(self, name: str):
        with self.target.indented(
            f"static ani_long {name}([[maybe_unused]] ani_env *env, ani_long data_ptr) {{",
            f"}}",
        ):
            self.target.writelns(
                f"return reinterpret_cast<ani_long>(tobj_dup(reinterpret_cast<DataBlockHead*>(data_ptr)));",
            )

    def gen_native_invoke(self, name: str):
        params_ani = []
        args_ani = []
        for i in range(16):
            arg_ani = f"arg_{i}"
            params_ani.append(f"ani_ref {arg_ani}")
            args_ani.append(arg_ani)
        params_ani_str = ", ".join(params_ani)
        args_ani_str = ", ".join(args_ani)
        return_type_ani_name = "ani_ref"
        with self.target.indented(
            f"static {return_type_ani_name} {name}([[maybe_unused]] ani_env *env, ani_long ani_cast_ptr, ani_long ani_func_ptr, ani_long ani_data_ptr, {params_ani_str}) {{",
            f"}}",
        ):
            self.target.writelns(
                f"return reinterpret_cast<{return_type_ani_name} (*)(ani_env *env, ani_long ani_func_ptr, ani_long ani_data_ptr, {params_ani_str})>(ani_cast_ptr)(env, ani_func_ptr, ani_data_ptr, {args_ani_str});",
            )


class AniIfaceDeclGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_ani_info = IfaceAniInfo.get(self.am, self.iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_ani_info.decl_header}",
            FileKind.C_HEADER,
        )

    def gen_iface_decl_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        iface_ani_info = IfaceAniInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include("taihe/platform/ani.hpp")
            self.target.add_include(iface_cpp_info.defn_header)
            with self.target.indented(
                f"template<> struct ::taihe::from_ani_t<{iface_cpp_info.as_owner}> {{",
                f"}};",
            ):
                self.target.writelns(
                    f"inline {iface_cpp_info.as_owner} operator()(ani_env* env, ani_object ani_obj) const;",
                )
            with self.target.indented(
                f"template<> struct ::taihe::into_ani_t<{iface_cpp_info.as_owner}> {{",
                f"}};",
            ):
                self.target.writelns(
                    f"inline ani_object operator()(ani_env* env, {iface_cpp_info.as_owner} cpp_obj) const;",
                )


class AniIfaceImplGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, iface: IfaceDecl):
        self.om = om
        self.am = am
        self.iface = iface
        iface_ani_info = IfaceAniInfo.get(self.am, self.iface)
        self.target = CHeaderWriter(
            self.om,
            f"include/{iface_ani_info.impl_header}",
            FileKind.C_HEADER,
        )

    def gen_iface_impl_file(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        iface_ani_info = IfaceAniInfo.get(self.am, self.iface)
        with self.target:
            self.target.add_include(iface_ani_info.decl_header)
            self.target.add_include(iface_cpp_info.impl_header)
            self.gen_iface_from_ani_func()
            self.gen_iface_into_ani_func()

    def gen_iface_from_ani_func(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        iface_ani_info = IfaceAniInfo.get(self.am, self.iface)
        with self.target.indented(
            f"inline {iface_cpp_info.as_owner} taihe::from_ani_t<{iface_cpp_info.as_owner}>::operator()(ani_env* env, ani_object ani_obj) const {{",
            f"}}",
        ):
            with self.target.indented(
                f"struct cpp_impl_t : ::taihe::dref_guard {{",
                f"}};",
            ):
                self.target.writelns(
                    f"cpp_impl_t(ani_env* env, ani_ref val) : ::taihe::dref_guard(env, val) {{}}",
                )
                for ancestor in iface_abi_info.ancestor_dict:
                    for method in ancestor.methods:
                        self.gen_iface_method(method)
                with self.target.indented(
                    f"uintptr_t getGlobalReference() const {{",
                    f"}}",
                ):
                    self.target.writelns(
                        f"return reinterpret_cast<uintptr_t>(this->ref);",
                    )
            self.target.writelns(
                f"return ::taihe::make_holder<cpp_impl_t, {iface_cpp_info.as_owner}, ::taihe::platform::ani::AniObject>(env, ani_obj);",
            )

    def gen_iface_method(self, method: IfaceMethodDecl):
        iface_ani_info = IfaceAniInfo.get(self.am, method.parent_iface)
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        method_ani_info = IfaceMethodAniInfo.get(self.am, method)
        params_cpp = []
        args_cpp = []
        args_ani = []
        for param in method.params:
            arg_cpp = f"cpp_arg_{param.name}"
            arg_ani = f"ani_arg_{param.name}"
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{param_ty_cpp_info.as_param} {arg_cpp}")
            args_cpp.append(arg_cpp)
            args_ani.append(arg_ani)
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = return_ty_cpp_info.as_owner
        else:
            return_ty_cpp_name = "void"
        with self.target.indented(
            f"{return_ty_cpp_name} {method_cpp_info.impl_name}({params_cpp_str}) {{",
            f"}}",
        ):
            self.target.writelns(
                f"::taihe::env_guard guard;",
                f"ani_env *env = guard.get_env();",
            )
            for param, arg_cpp, arg_ani in zip(
                method.params, args_cpp, args_ani, strict=True
            ):
                param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
                param_ty_ani_info.into_ani(
                    self.target,
                    "env",
                    arg_cpp,
                    arg_ani,
                )
            args_ani_str = ", ".join(["static_cast<ani_object>(this->ref)", *args_ani])
            pkg_ani_info = PackageAniInfo.get(self.am, method.parent_pkg)
            function = f'TH_ANI_FIND_{pkg_ani_info.ns.scope.upper}_FUNCTION(env, "{pkg_ani_info.ns.impl_desc}", "{method_ani_info.reverse_name}", nullptr)'
            if isinstance(return_ty := method.return_ty, NonVoidType):
                result_ani = "ani_result"
                result_cpp = "cpp_result"
                return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
                self.target.writelns(
                    f"{return_ty_ani_info.ani_type} {result_ani} = {{}};",
                    f"env->Function_Call_{return_ty_ani_info.ani_type.suffix}({function}, reinterpret_cast<{return_ty_ani_info.ani_type.base}*>(&{result_ani}), {args_ani_str});",
                )
                return_ty_ani_info.from_ani(
                    self.target,
                    "env",
                    result_ani,
                    result_cpp,
                )
                self.target.writelns(
                    f"return {result_cpp};",
                )
            else:
                self.target.writelns(
                    f"env->Function_Call_Void({function}, {args_ani_str});",
                )

    def gen_iface_into_ani_func(self):
        iface_abi_info = IfaceAbiInfo.get(self.am, self.iface)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.iface)
        iface_ani_info = IfaceAniInfo.get(self.am, self.iface)
        with self.target.indented(
            f"inline ani_object taihe::into_ani_t<{iface_cpp_info.as_owner}>::operator()(ani_env* env, {iface_cpp_info.as_owner} cpp_obj) const {{",
            f"}}",
        ):
            self.target.writelns(
                f"ani_long ani_vtbl_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.vtbl_ptr);",
                f"ani_long ani_data_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.data_ptr);",
                f"cpp_obj.m_handle.data_ptr = nullptr;",
                f"ani_object ani_obj;",
                f'env->Function_Call_Ref(TH_ANI_FIND_{iface_ani_info.parent_ns.scope.upper}_FUNCTION(env, "{iface_ani_info.parent_ns.impl_desc}", "{iface_ani_info.sts_ctor_name}", nullptr), reinterpret_cast<ani_ref*>(&ani_obj), ani_vtbl_ptr, ani_data_ptr);',
                f"return ani_obj;",
            )


class AniStructDeclGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, struct: StructDecl):
        self.om = om
        self.am = am
        self.struct = struct
        struct_ani_info = StructAniInfo.get(self.am, self.struct)
        self.target = CHeaderWriter(
            self.om,
            f"include/{struct_ani_info.decl_header}",
            FileKind.C_HEADER,
        )

    def gen_struct_decl_file(self):
        struct_cpp_info = StructCppInfo.get(self.am, self.struct)
        struct_ani_info = StructAniInfo.get(self.am, self.struct)
        with self.target:
            self.target.add_include("taihe/platform/ani.hpp")
            self.target.add_include(struct_cpp_info.defn_header)
            with self.target.indented(
                f"template<> struct ::taihe::from_ani_t<{struct_cpp_info.as_owner}> {{",
                f"}};",
            ):
                self.target.writelns(
                    f"inline {struct_cpp_info.as_owner} operator()(ani_env* env, ani_object ani_obj) const;",
                )
            with self.target.indented(
                f"template<> struct ::taihe::into_ani_t<{struct_cpp_info.as_owner}> {{",
                f"}};",
            ):
                self.target.writelns(
                    f"inline ani_object operator()(ani_env* env, {struct_cpp_info.as_owner} cpp_obj) const;",
                )


class AniStructImplGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, struct: StructDecl):
        self.om = om
        self.am = am
        self.struct = struct
        struct_ani_info = StructAniInfo.get(self.am, self.struct)
        self.target = CHeaderWriter(
            self.om,
            f"include/{struct_ani_info.impl_header}",
            FileKind.C_HEADER,
        )

    def gen_struct_impl_file(self):
        struct_cpp_info = StructCppInfo.get(self.am, self.struct)
        struct_ani_info = StructAniInfo.get(self.am, self.struct)
        with self.target:
            self.target.add_include(struct_ani_info.decl_header)
            self.target.add_include(struct_cpp_info.impl_header)
            self.gen_struct_from_ani_func()
            self.gen_struct_into_ani_func()

    def gen_struct_from_ani_func(self):
        struct_cpp_info = StructCppInfo.get(self.am, self.struct)
        struct_ani_info = StructAniInfo.get(self.am, self.struct)
        with self.target.indented(
            f"inline {struct_cpp_info.as_owner} taihe::from_ani_t<{struct_cpp_info.as_owner}>::operator()(ani_env* env, ani_object ani_obj) const {{",
            f"}}",
        ):
            fields_cpp = []
            for parts in struct_ani_info.sts_all_fields:
                final = parts[-1]
                final_ty_ani_info = TypeAniInfo.get(self.am, final.ty)
                field_ani = f"ani_field_{final.name}"
                field_cpp = f"cpp_field_{final.name}"
                self.target.writelns(
                    f"{final_ty_ani_info.ani_type} {field_ani} = {{}};",
                )
                if struct_ani_info.is_class():
                    self.target.writelns(
                        f'env->Object_GetField_{final_ty_ani_info.ani_type.suffix}(ani_obj, TH_ANI_FIND_CLASS_FIELD(env, "{struct_ani_info.type_desc}", "{final.name}"), reinterpret_cast<{final_ty_ani_info.ani_type.base}*>(&{field_ani}));',
                    )
                else:
                    self.target.writelns(
                        f'env->Object_CallMethod_{final_ty_ani_info.ani_type.suffix}(ani_obj, TH_ANI_FIND_CLASS_METHOD(env, "{struct_ani_info.type_desc}", "%%get-{final.name}", nullptr), reinterpret_cast<{final_ty_ani_info.ani_type.base}*>(&{field_ani}));',
                    )
                final_ty_ani_info.from_ani(
                    self.target,
                    "env",
                    field_ani,
                    field_cpp,
                )
                fields_cpp.append(field_cpp)
            fields_cpp_str = ", ".join(
                f"std::move({field_cpp})" for field_cpp in fields_cpp
            )
            self.target.writelns(
                f"return {struct_cpp_info.as_owner}{{{fields_cpp_str}}};",
            )

    def gen_struct_into_ani_func(self):
        struct_cpp_info = StructCppInfo.get(self.am, self.struct)
        struct_ani_info = StructAniInfo.get(self.am, self.struct)
        with self.target.indented(
            f"inline ani_object taihe::into_ani_t<{struct_cpp_info.as_owner}>::operator()(ani_env* env, {struct_cpp_info.as_owner} cpp_obj) const {{",
            f"}}",
        ):
            fields_ani = []
            for parts in struct_ani_info.sorted_sts_all_fields:
                final = parts[-1]
                field_ani = f"ani_field_{final.name}"
                final_ty_ani_info = TypeAniInfo.get(self.am, final.ty)
                final_ty_ani_info.into_ani(
                    self.target,
                    "env",
                    ".".join(("cpp_obj", *(part.name for part in parts))),
                    field_ani,
                )
                fields_ani.append(field_ani)
            fields_ani_sum = "".join(", " + field_ani for field_ani in fields_ani)
            self.target.writelns(
                f"ani_object ani_obj = {{}};",
                f'env->Function_Call_Ref(TH_ANI_FIND_{struct_ani_info.parent_ns.scope.upper}_FUNCTION(env, "{struct_ani_info.parent_ns.impl_desc}", "{struct_ani_info.sts_ctor_name}", nullptr), reinterpret_cast<ani_ref*>(&ani_obj){fields_ani_sum});',
                f"return ani_obj;",
            )


class AniUnionDeclGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, union: UnionDecl):
        self.om = om
        self.am = am
        self.union = union
        union_ani_info = UnionAniInfo.get(self.am, self.union)
        self.target = CHeaderWriter(
            self.om,
            f"include/{union_ani_info.decl_header}",
            FileKind.C_HEADER,
        )

    def gen_union_decl_file(self):
        union_cpp_info = UnionCppInfo.get(self.am, self.union)
        union_ani_info = UnionAniInfo.get(self.am, self.union)
        with self.target:
            self.target.add_include("taihe/platform/ani.hpp")
            self.target.add_include(union_cpp_info.defn_header)
            with self.target.indented(
                f"template<> struct ::taihe::from_ani_t<{union_cpp_info.as_owner}> {{",
                f"}};",
            ):
                self.target.writelns(
                    f"inline {union_cpp_info.as_owner} operator()(ani_env* env, ani_ref ani_value) const;",
                )
            with self.target.indented(
                f"template<> struct ::taihe::into_ani_t<{union_cpp_info.as_owner}> {{",
                f"}};",
            ):
                self.target.writelns(
                    f"inline ani_ref operator()(ani_env* env, {union_cpp_info.as_owner} cpp_value) const;",
                )


class AniUnionImplGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager, union: UnionDecl):
        self.om = om
        self.am = am
        self.union = union
        union_ani_info = UnionAniInfo.get(self.am, self.union)
        self.target = CHeaderWriter(
            self.om,
            f"include/{union_ani_info.impl_header}",
            FileKind.C_HEADER,
        )

    def gen_union_impl_file(self):
        union_cpp_info = UnionCppInfo.get(self.am, self.union)
        union_ani_info = UnionAniInfo.get(self.am, self.union)
        with self.target:
            self.target.add_include(union_ani_info.decl_header)
            self.target.add_include(union_cpp_info.impl_header)
            self.gen_union_from_ani_func()
            self.gen_union_into_ani_func()

    def gen_union_from_ani_func(self):
        union_cpp_info = UnionCppInfo.get(self.am, self.union)
        union_ani_info = UnionAniInfo.get(self.am, self.union)
        with self.target.indented(
            f"inline {union_cpp_info.as_owner} taihe::from_ani_t<{union_cpp_info.as_owner}>::operator()(ani_env* env, ani_ref ani_value) const {{",
            f"}}",
        ):
            for parts in union_ani_info.sts_all_fields:
                final = parts[-1]
                static_tags = []
                for part in parts:
                    path_cpp_info = UnionCppInfo.get(self.am, part.parent_union)
                    static_tags.append(
                        f"::taihe::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
                    )
                static_tags_str = ", ".join(static_tags)
                full_name = "_".join(part.name for part in parts)
                is_field_ani = f"ani_is_{full_name}"
                field_cpp = f"cpp_field_{full_name}"
                final_ty_ani_info = TypeAniInfo.get(self.am, final.ty)
                self.target.writelns(
                    f"ani_boolean {is_field_ani} = {{}};",
                )
                final_ty_ani_info.check_type(self.target, "env", is_field_ani)
                with self.target.indented(
                    f"if ({is_field_ani}) {{",
                    f"}}",
                ):
                    final_ty_ani_info.from_ani_boxed(
                        self.target,
                        "env",
                        "ani_value",
                        field_cpp,
                    )
                    self.target.writelns(
                        f"return {union_cpp_info.full_name}({static_tags_str}, std::move({field_cpp}));",
                    )
            self.target.writelns(
                f"__builtin_unreachable();",
            )

    def gen_union_into_ani_func(self):
        union_cpp_info = UnionCppInfo.get(self.am, self.union)
        union_ani_info = UnionAniInfo.get(self.am, self.union)
        with self.target.indented(
            f"inline ani_ref taihe::into_ani_t<{union_cpp_info.as_owner}>::operator()(ani_env* env, {union_cpp_info.as_owner} cpp_value) const {{",
            f"}}",
        ):
            with self.target.indented(
                f"switch (cpp_value.get_tag()) {{",
                f"}}",
                indent="",
            ):
                for field in self.union.fields:
                    with self.target.indented(
                        f"case {union_cpp_info.full_name}::tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        field_ani = f"ani_field_{field.name}"
                        field_ty_ani_info = TypeAniInfo.get(self.am, field.ty)
                        field_ty_ani_info.into_ani_boxed(
                            self.target,
                            "env",
                            f"cpp_value.get_{field.name}_ref()",
                            field_ani,
                        )
                        self.target.writelns(
                            f"return {field_ani};",
                        )