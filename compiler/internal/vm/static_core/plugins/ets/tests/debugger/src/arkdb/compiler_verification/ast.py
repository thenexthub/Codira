#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import difflib
import json
from copy import deepcopy
from typing import Any, Callable, Final, TypeVar

from arkdb.compiler_verification import ExpressionEvaluationNames
from arkdb.runnable_module import ScriptFile

T = TypeVar("T")
Transformer = Callable[[T], T]

AstNode = dict[str, Any]
AstNodeOrList = AstNode | list[AstNode]

# NOTE(dslynko): remove this utility when es2panda-generated cctor calls are removed
CCTOR_PREFIX: Final[str] = "_$trigger_cctor$_for_$"


class AstComparisonError(AssertionError):
    pass


class AstComparator:
    def __init__(self, expression: ScriptFile, expected: ScriptFile, expected_imports_base: bool = False):
        self.expression = expression
        self.expected = expected
        self.expected_imports_base = expected_imports_base
        self._eval_method_name: str | None = None

    def get_eval_method_name(self) -> str:
        if self._eval_method_name is None:
            # Initialize `_eval_method_name` by traversing expression AST.
            self._prepare_expression_statements()
        if self._eval_method_name is None:
            raise RuntimeError()
        return self._eval_method_name

    def compare(self):
        expected_stmts, expected_imports = self._prepare_expected_statements()
        expression_stmts, expression_imports = self._prepare_expression_statements()
        _compare_ast_statements(expression_stmts, expected_stmts)
        _compare_ast_import_decls(expression_imports, expected_imports)

    def _prepare_expected_statements(self):
        statements_list = self.expected.ast.get("statements")
        if not isinstance(statements_list, list):
            raise RuntimeError()
        statements_list = deepcopy(statements_list)

        if self.expected_imports_base:
            base_test_file_name = self.expected.source_file.name
            if (idx := base_test_file_name.find(".")) > 0 and idx < len(base_test_file_name) - 1:
                base_test_file_name = base_test_file_name[:idx]

            statements_filter = _get_import_statements_sources_filter({base_test_file_name: ""})
            statements_list = list(map(statements_filter, statements_list))

        def _find_prefix_recursively(ast_node: Any, prefix: str):
            if isinstance(ast_node, dict):
                if (name := ast_node.get("name")) and isinstance(name, str) and name.startswith(prefix):
                    return True
                return any(_find_prefix_recursively(x, prefix) for x in ast_node.values())
            if isinstance(ast_node, list):
                return any(_find_prefix_recursively(x, prefix) for x in ast_node)
            return False

        def _imports_trigger_cctor(x: dict) -> bool:
            return x.get("type") == "ImportSpecifier" and _find_prefix_recursively(x.get("local"), CCTOR_PREFIX)

        def _remove_cctor_call(ast_node: Any) -> Any:
            if isinstance(ast_node, dict):
                if (stmts := ast_node.get("statements")) and isinstance(stmts, list):
                    ast_node["statements"] = [x for x in stmts if not _find_prefix_recursively(x, CCTOR_PREFIX)]
                    return ast_node
                for key, value in ast_node.items():
                    ast_node[key] = _remove_cctor_call(value)
            elif isinstance(ast_node, list):
                return [
                    _remove_cctor_call(x) for x in ast_node if not (isinstance(x, dict) and _imports_trigger_cctor(x))
                ]
            return ast_node

        return _split_statements(statements_list, additional_filters=[_remove_cctor_call])

    def _prepare_expression_statements(self):
        statements_list = self.expression.ast.get("statements")
        if not isinstance(statements_list, list):
            raise RuntimeError()
        statements_list = deepcopy(statements_list)

        method_names_candidates: set[str] = set()

        def _replace_in_ast_node(ast_node: AstNode):
            for key, value in ast_node.items():
                if key == "name" and isinstance(value, str):
                    if ExpressionEvaluationNames.EVAL_METHOD_GENERATED_NAME_RE.match(value):
                        method_names_candidates.add(value)
                        ast_node[key] = ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME
                    elif ExpressionEvaluationNames.EVAL_METHOD_RETURN_VALUE_RE.match(value):
                        ast_node[key] = ExpressionEvaluationNames.EVAL_PATCH_RETURN_VALUE
                else:
                    ast_node[key] = replace_generated_names(value)

        def replace_generated_names(ast_node: Any) -> Any:
            if isinstance(ast_node, dict):
                _replace_in_ast_node(ast_node)
            elif isinstance(ast_node, list):
                return list(map(replace_generated_names, ast_node))
            return ast_node

        stmts, imports = _split_statements(
            statements_list,
            additional_filters=[replace_generated_names],
        )

        method_names_list = list(method_names_candidates)
        if len(method_names_list) != 1 or method_names_list[0] == "":
            error_message = f"Failed to find expected evaluation method name; candidates: {method_names_list}"
            raise AstComparisonError(error_message)
        self._eval_method_name = method_names_list[0]

        return stmts, imports


def _del_locations(ast: Any) -> Any:
    if isinstance(ast, dict):
        for key, value in ast.copy().items():
            if key == "loc":
                del ast[key]
            else:
                ast[key] = _del_locations(value)
    elif isinstance(ast, list):
        return list(map(_del_locations, ast))
    return ast


def _split_statements(statements_list: list[AstNode], additional_filters: list[Transformer] | None = None):
    all_filters = additional_filters if additional_filters else []
    all_filters.append(_del_locations)
    for f in all_filters:
        statements_list = f(statements_list)

    imports_list: list[AstNode] = []

    def filter_lambda(stmt_node: AstNode) -> bool:
        if stmt_node.get("type") == "ImportDeclaration":
            imports_list.append(stmt_node)
            return False
        return True

    # Collect import declarations in imports_list and remove them from AST.
    filtered_list = filter(filter_lambda, statements_list)
    return list(filtered_list), imports_list


def _get_import_statements_sources_filter(imports_replacement_map: dict[str, str]) -> Callable[[AstNode], AstNode]:
    def imports_filter(stmt_node: AstNode) -> AstNode:
        if stmt_node.get("type") != "ImportDeclaration":
            return stmt_node

        source_node = stmt_node.get("source")
        if not isinstance(source_node, dict):
            raise RuntimeError()
        import_path = source_node.get("value")
        if not isinstance(import_path, str):
            raise RuntimeError()

        # ArkTS paths always have "/" as delimiter.
        if (pos := import_path.rfind("/")) != -1:
            pos += 1
            import_path_name = import_path[pos:]
            if (new_import_path := imports_replacement_map.get(import_path_name)) is not None:
                stmt_node["source"]["value"] = new_import_path
        return stmt_node

    return imports_filter


def dump_ast(ast: AstNodeOrList):
    return json.dumps(ast, indent=4).splitlines()


def _compare_ast_statements(patched_output: AstNodeOrList, expected_output: AstNodeOrList):
    diff = difflib.ndiff(dump_ast(patched_output), dump_ast(expected_output))
    error_list = [x for x in diff if x[0] not in ExpressionEvaluationNames.NON_DIFF_MARKS]
    if error_list:
        raise AstComparisonError("AST comparison failed:\n" + "\n".join(error_list))


def _compare_ast_import_decls(expression_imports: list[AstNode], expected_imports: list[AstNode]):
    expression_specifiers_list: list[AstNode] = []
    expected_local_name_to_specifier: dict[str, AstNode] = {}

    for import_decl in expression_imports:
        specifiers_list = import_decl.get("specifiers")
        if not isinstance(specifiers_list, list):
            raise RuntimeError()
        for specifier in specifiers_list:
            expression_specifiers_list.append(specifier)

    for import_decl in expected_imports:
        specifiers_list = import_decl.get("specifiers")
        if not isinstance(specifiers_list, list):
            raise RuntimeError()
        for specifier in specifiers_list:
            local_import_name = specifier.get("local").get("name")
            expected_local_name_to_specifier[local_import_name] = specifier
    if len(expected_local_name_to_specifier) != len(expression_specifiers_list):
        error_report = (
            f"Imports expected size {len(expected_local_name_to_specifier)}"
            f" do not match with patch imports size {len(expression_specifiers_list)}"
        )
        raise AstComparisonError(error_report)

    for specifier in expression_specifiers_list:
        local_identifier_node = specifier.get("local")
        if not isinstance(local_identifier_node, dict):
            raise RuntimeError()
        local_import_name = local_identifier_node.get("name")
        expected_specifier = expected_local_name_to_specifier.get(local_import_name, None)
        if expected_specifier is None:
            error_report = f"Patch import specifier {local_import_name} do not contained in expected specifiers"
            raise AstComparisonError(error_report)

        _compare_ast_statements(specifier, expected_specifier)
