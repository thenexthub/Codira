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

from io import StringIO

import pytest
from taihe.driver.backend import BackendRegistry
from taihe.driver.contexts import CompilerInstance, CompilerInvocation
from taihe.utils.diagnostics import DiagBase, DiagnosticsManager
from taihe.utils.exceptions import (
    AttrArgMissingError,
    AttrArgOrderError,
    AttrArgRedefError,
    AttrArgTypeError,
    AttrArgUnrequiredError,
    AttrConflictError,
    AttrNotExistError,
    AttrTargetError,
    DeclarationNotInScopeError,
    DeclNotExistError,
    DeclRedefError,
    DuplicateExtendsWarn,
    EnumValueError,
    GenericArgumentsError,
    IDLSyntaxError,
    InvalidPackageNameError,
    NotATypeError,
    PackageNotExistError,
    PackageNotInScopeError,
    RecursiveReferenceError,
    SymbolConflictWithNamespaceError,
    TypeUsageError,
)
from taihe.utils.sources import SourceBuffer
from typing_extensions import override


class SemanticTestDiagnosticsManager(DiagnosticsManager):
    errors: list[DiagBase]

    def __init__(self):
        self.errors = []

    @override
    def emit(self, diag: DiagBase) -> None:
        self.errors.append(diag)


class SemanticTestCompilerInstance(CompilerInstance):
    def __init__(self, invocation: CompilerInvocation):
        super().__init__(invocation, dm=SemanticTestDiagnosticsManager)

    def add_source(self, pkg_name: str, source: str):
        self.source_manager.add_source(SourceBuffer(pkg_name, StringIO(source)))

    @override
    def collect(self):
        pass

    def assert_has_error(self, ty: type[DiagBase] = DiagBase):
        pass
        if not any(isinstance(err, ty) for err in self.diagnostics_manager.errors):
            print(f"Known: {self.diagnostics_manager.errors}")
            pytest.fail(f"Assertion mismatch: expect {ty}")

    def assert_no_error(self, ty: type[DiagBase] = DiagBase):
        pass
        if any(isinstance(err, ty) for err in self.diagnostics_manager.errors):
            print(f"Known: {self.diagnostics_manager.errors}")
            pytest.fail(f"Assertion mismatch: unexpected {ty}")


backend_registry = BackendRegistry()
backend_registry.register_all()


#############################
# Tests for semantic errors #
#############################


pre_backend_names = ["pretty-print"]
pre_backend_factories = backend_registry.collect_required_backends(pre_backend_names)
pre_backend_configs = [b() for b in pre_backend_factories]
pre_invocation = CompilerInvocation(backend_configs=pre_backend_configs)


def test_invalid_package_name():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source("package.1", "")
    test_instance.run()
    test_instance.assert_has_error(InvalidPackageNameError)


def test_package_not_exist():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "use a;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotExistError)


def test_package_not_in_scope_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct BadStruct {\n"
        "    a: unimported.package.Type;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotInScopeError)


def test_package_not_in_scope_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
        "struct B{\n"
        "    b: A.Type;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotInScopeError)


def test_package_not_in_scope_3():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "from package.example use B;\n"
        "struct A {\n"
        "    a: B.Type;\n"
        "}\n"
    )
    test_instance.add_source(
        "package.example",
        "struct B{\n"
        "    b: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotInScopeError)


def test_decl_redef_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "function bad_func(a: i32, a: i32);\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "from package.example1 use A;\n"
        "from package.example2 use A;\n"
    )
    test_instance.add_source(
        "package.example1",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.add_source(
        "package.example2",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_3():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "use package.example1 as example;\n"
        "use package.example2 as example;\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.add_source("package.example2", "")
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_4():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct BadStruct {\n"
        "    a: i32;\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_package_redef():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source("package", "")
    test_instance.add_source("package", "")
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_symbol_conflict_namespace():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "use package.example1.a;\n"
    )
    test_instance.add_source(
        "package.example1",
        "struct a {\n"
        "    A: String;\n"
        "}\n"
    )
    test_instance.add_source("package.example1.a", "")
    test_instance.run()
    test_instance.assert_has_error(SymbolConflictWithNamespaceError)


def test_decl_not_exist_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "from package.example1 use A;\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.run()
    test_instance.assert_has_error(DeclNotExistError)


def test_decl_not_exist_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "use package.example1;\n"
        "struct BadStruct {\n"
        "    a: package.example1.B;\n"
        "}\n"
    )
    test_instance.add_source(
        "package.example1",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclNotExistError)


def test_declaration_not_in_scope_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct BadStruct {\n"
        "    a: UnimportedType;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclarationNotInScopeError)


def test_declaration_not_in_scope_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "use package.example1 as example\n"
        "struct A {\n"
        "    a: example;\n"
        "}\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.run()
    test_instance.assert_has_error(DeclarationNotInScopeError)


def test_recursive_inclusion():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: B;\n"
        "}\n"
        "struct B {\n"
        "    a: C;\n"
        "}\n"
        "struct C {\n"
        "    a: A;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(RecursiveReferenceError)


def test_extends_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "interface BadIface: i32 {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(TypeUsageError)


def test_param_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "function foo(p: void);\n"
    )
    test_instance.run()
    test_instance.assert_has_error(TypeUsageError)


def test_enum_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "enum foo: void {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(TypeUsageError)


def test_generic_arg_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: Vector<void>;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(TypeUsageError)


def test_generic_arg_count():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: Vector<i32, i32>;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(GenericArgumentsError)


def test_duplicate_extends():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "interface TestIface {}\n"
        "interface BadIface: TestIface, TestIface {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DuplicateExtendsWarn)


def test_idl_syntax():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: $;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(IDLSyntaxError)


def test_not_a_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "function good_func(a: i32): void;\n"
        "struct A {\n"
        "    a: good_func;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(NotATypeError)


def test_enum_value():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "enum MyEnum: String {\n"
        "    A = 0,\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(EnumValueError)


#################################
# Tests for ANI-specific errors #
#################################


ani_backend_names = ["cpp-author", "ani-bridge", "pretty-print"]
ani_backend_factories = backend_registry.collect_required_backends(ani_backend_names)
ani_backend_configs = [b() for b in ani_backend_factories]
ani_invocation = CompilerInvocation(backend_configs=ani_backend_configs)


def test_attr_arg_order_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        '@!namespace(module="abc", "a.b")\n'
        "interface IFoo {\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrArgOrderError)


def test_attr_arg_redef_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        '@!namespace(module="abc", module="def")\n'
        "interface IFoo {\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrArgRedefError)


def test_attr_arg_missing_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    a(): void;\n"
        "}\n"
        "@static\n"
        "function f(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrArgMissingError)


def test_attr_arg_unrequired_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    a(): void;\n"
        "}\n"
        '@static("IFoo", xxx="b")\n'
        "function f(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrArgUnrequiredError)


def test_attr_arg_type_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    a(): void;\n"
        "}\n"
        "@static(123)\n"
        "function f(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrArgTypeError)


def test_attr_not_exist_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@clasz\n"
        "interface IFoo {\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrNotExistError)


def test_attr_conflict_error_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "union MyUnion {\n"
        "    @null\n"
        "    @undefined\n"
        "    a;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrConflictError)


def test_attr_conflict_error_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@class\n"
        "@class\n"
        "interface IFoo {\n"
        "    getX(): String;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrConflictError)


def test_attr_target_error():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@class\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AttrTargetError)