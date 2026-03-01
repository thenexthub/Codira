#!/usr/bin/env python3
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

import argparse
import concurrent.futures
import io
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import threading
import time
from collections import defaultdict
from pathlib import Path
from typing import Any, Dict, List, Optional


class Constants:
    """Constant definitions"""

    # Field name constants
    START_LINE = "start_line"
    BRANCH_INDEX = "branch_index"
    CASE_INDEX = "case_index"
    COUNT = "count"
    BRANCH_TYPE = "branch_type"
    BODY = "body"
    ALTERNATE = "alternate"
    STATEMENTS = "statements"
    CONSEQUENT = "consequent"
    FUNCTION_NAME = "function_name"
    LINE_TABLE = "line_table"
    NAME = "name"
    LOC = "loc"
    START = "start"
    LINE = "line"
    TYPE = "type"
    END = "end"
    BLOCK = "block"
    FINALIZER = "finalizer"
    HANDLER = "handler"
    KEY = "key"
    CASES = "cases"
    VALUE = "value"
    FUNCTION = "function"

    # File extensions
    EXT_ETS = ".ets"
    EXT_JSON = ".json"
    EXT_PA = ".pa"
    EXT_INFO = ".info"
    EXT_CSV = ".csv"

    # Branch types
    IF_STATEMENT = "IfStatement"
    SWITCH_STATEMENT = "SwitchStatement"
    SWITCH_CASE = "SwitchCase"
    CONDITIONAL_EXPRESSION = "ConditionalExpression"
    FOR_UPDATE_STATEMENT = "ForUpdateStatement"
    FOR_IN_STATEMENT = "ForInStatement"
    FOR_OF_STATEMENT = "ForOfStatement"
    WHILE_STATEMENT = "WhileStatement"
    DO_WHILE_STATEMENT = "DoWhileStatement"
    TRY_STATEMENT = "TryStatement"

    BRANCH_TYPES = {
        IF_STATEMENT,
        SWITCH_STATEMENT,
        SWITCH_CASE,
        CONDITIONAL_EXPRESSION,
        FOR_UPDATE_STATEMENT,
        FOR_IN_STATEMENT,
        FOR_OF_STATEMENT,
        WHILE_STATEMENT,
        DO_WHILE_STATEMENT,
        TRY_STATEMENT,
    }

    # AST node types
    NODE_METHOD_DEFINITION = "MethodDefinition"
    NODE_IDENTIFIER = "Identifier"
    NODE_CATCH_CLAUSE = "CatchClause"
    NODE_BLOCK_STATEMENT = "BlockStatement"

    # Regular expression patterns
    FUNCTION_PATTERN = (
        r"\.function\s+(?:[^\.]+\.)*((?:lambda\$invoke\$\d+|lambda_invoke-\d+|[^\.\s<]+)"
        r"\([^)]*\))\s+<.*?>\s+\{\s*#\s+offset:\s+(0x[0-9a-fA-F]+).*?LINE_NUMBER_TABLE:"
        r"(.*?)\}"
    )

    FUNCTION_PATTERN_HAP = (
        r"\.function\s+((?:[^\.]+\.)*)((?:lambda\$invoke\$\d+|lambda_invoke-\d+|[^\.\s<]+)"
        r"\([^)]*\))\s+<.*?>\s+\{\s*#\s+offset:\s+(0x[0-9a-fA-F]+).*?LINE_NUMBER_TABLE:"
        r"(.*?)\}"
    )

    LINE_TABLE_PATTERN = r"line\s+(\d+):\s+(\d+)"
    FUNCTION_NAME_PATTERN = r"{}\s*(?:<[^>]*>)?\s*\("  # Matching common function and template function

    # Default values
    DEFAULT_ANONYMOUS_FUNC_NAME = "anonymous"
    LAMBDA_INVOKE_1 = "lambda$invoke$"
    LAMBDA_INVOKE_2 = "lambda_invoke-"
    CTOR = "_ctor_"
    CONSTRUCTOR = "constructor"
    CCTOR = "_cctor_"


class Config:
    """Configuration management class"""

    # Directory configuration
    AST_DIR = "ast"
    PA_DIR = "pa"
    RUNTIME_DIR = "runtime_info"
    OUTPUT_FILE = "result.info"

    # CSV column names
    COLUMN_METHODID = "methodid"
    COLUMN_OFFSET = "offset"
    COLUMN_COUNTER = "counter"

    CSV_COLUMNS = [COLUMN_METHODID, COLUMN_OFFSET, COLUMN_COUNTER]

    @classmethod
    def validate_directories(cls):
        """Validate required directories exist"""
        required_dirs = [cls.AST_DIR, cls.PA_DIR, cls.RUNTIME_DIR]
        for directory in required_dirs:
            if not Path(runtime_config.workdir).joinpath(directory).exists():
                raise FileNotFoundError(f"Directory does not exist: {directory}")

    @classmethod
    def get_runtime_dir(cls) -> str:
        """Get output file path"""
        return str(Path(runtime_config.workdir).joinpath(cls.RUNTIME_DIR))

    @classmethod
    def get_output_file(cls) -> str:
        """Get output file path"""
        return str(Path(runtime_config.workdir).joinpath(cls.OUTPUT_FILE))


class RuntimeConfig:
    """
    Runtime configuration singleton class for managing global variables.
    Provides methods to get and set global variables.
    """
    _instance = None
    _initialized = False

    def __new__(cls):
        """Ensure only one instance of the class exists."""
        if cls._instance is None:
            cls._instance = super(RuntimeConfig, cls).__new__(cls)
        return cls._instance

    def __init__(self):
        """Initialize configuration variables, executed only when the instance is first created."""
        if not RuntimeConfig._initialized:
            self._workdir = os.getcwd()
            self._mode = 'host'
            self._report_type = 'all'
            self._change_info = None

            RuntimeConfig._initialized = True

    # debug related
    def __str__(self) -> str:
        """Return string representation of configurations"""
        return (
            f"RuntimeConfig("
            f"workdir='{self._workdir}', "
            f"mode='{self._mode}', "
            f"report_type='{self._report_type}', "
            f"change_info={self._change_info}"
            f")"
        )

    # workdir related
    @property
    def workdir(self) -> str:
        """Get current working directory"""
        return self._workdir

    @workdir.setter
    def workdir(self, value: str) -> None:
        """Set current working directory"""
        self._workdir = value

    # mode related
    @property
    def mode(self) -> str:
        """Get mode"""
        return self._mode

    @mode.setter
    def mode(self, value: str) -> None:
        """Set mode"""
        self._mode = value

    # report_type related
    @property
    def report_type(self) -> str:
        """Get report type"""
        return self._report_type

    @report_type.setter
    def report_type(self, value: str) -> None:
        """Set report type"""
        self._report_type = value

    # change_info related
    @property
    def change_info(self) -> Optional[Any]:
        """Get change info"""
        return self._change_info

    @change_info.setter
    def change_info(self, value: Optional[Any]) -> None:
        """Set change info"""
        self._change_info = value

    # reset related
    def reset(self) -> None:
        """Reset all configurations to default values"""
        self._workdir = os.getcwd()
        self._mode = 'host'
        self._report_type = 'all'
        self._change_info = None


# Create a globally accessible instance
runtime_config = RuntimeConfig()


class FileUtils:
    """File operation utility class"""

    @staticmethod
    def read_file(file_path: str, encoding: str = "utf-8") -> str:
        """Safely read file content"""
        if not Path(file_path).exists():
            raise FileNotFoundError(f"File does not exist: {file_path}")

        with open(file_path, "r", encoding=encoding) as f:
            return f.read()

    @staticmethod
    def write_file(file_path: str, content: str, open_mode: str = "w", encoding: str = "utf-8") -> bool:
        """Safely write file"""
        try:
            Path(file_path).parent.mkdir(parents=True, exist_ok=True)
            with Path(file_path).open(open_mode, encoding=encoding) as f:
                f.write(content)
            return True
        except Exception as e:
            print(f"Error writing to file {file_path}: {e}")
            return False

    @staticmethod
    def ensure_extension(file_path: str, extension: str) -> str:
        """Ensure file has correct extension"""
        if not file_path.endswith(extension):
            return file_path + extension
        return file_path

    @staticmethod
    def find_files(directory: str, pattern: str) -> List[Path]:
        """Find files matching pattern"""
        return list(Path(directory).rglob(pattern))


class CoverageInfo:
    def __init__(self):
        self.source_file_path = ""
        self.runtime_info = None
        self.functions = defaultdict(self._default_func_info)
        self.line_stats = defaultdict(int)
        self.func_stats = defaultdict(int)
        self.function_definition_line = {}
        self.function_name_to_lines = defaultdict(set)
        self.branch_stats = {}
        self.process_file_result = False
        self.branch_info_for_diff = {}
        self.related_funcs = set()

    def reset_stats(self):
        """Reset statistics"""
        self.source_file_path = ""
        self.runtime_info = None
        self.functions.clear()
        self.line_stats.clear()
        self.func_stats.clear()
        self.function_definition_line.clear()
        self.function_name_to_lines.clear()
        self.branch_stats.clear()
        self.process_file_result = False
        self.branch_info_for_diff = {}
        self.related_funcs.clear()

    def export_lcov(self, output_file: str) -> bool:
        """Export coverage report in LCOV format"""
        if not self.process_file_result:
            print("Coverage information not parsed")
            return False

        try:
            output_file = FileUtils.ensure_extension(output_file, Constants.EXT_INFO)
            lcov_content = self._generate_lcov_content()

            if runtime_config.report_type == 'diff':
                if self.source_file_path not in runtime_config.change_info:
                    return False
                lcov_content = self._filter_lcov_content(lcov_content)
                if lcov_content is None:
                    print("Failed to generate LCOV content")
                    return False

            return FileUtils.write_file(output_file, lcov_content, "a")
        except Exception as e:
            print(f"Error exporting LCOV: {e}")
            return False

    def process_data(self, src_file_path: str, pa_file_path: str,
                     ast_file_path: str, runtime_info: List[Dict]):
        """Set file paths and parse"""
        self.reset_stats()
        self.source_file_path = src_file_path

        # Parse files
        self.process_file_result = (
                self._process_ast_file(ast_file_path) and
                self._process_pa_file(pa_file_path) and
                self._process_runtime_info(runtime_info)
        )

    def _filter_lcov_content(self, lcov_content: str) -> str:
        """Filter LCOV content based on change_info"""
        if runtime_config.change_info is None:
            print("change_info is none!")
            return None

        if self.source_file_path not in runtime_config.change_info:
            return None

        filtered_lines = []
        for line in lcov_content.splitlines():
            if self._check_should_retain(line):
                filtered_lines.append(line)

        return "\n".join(filtered_lines) + "\n"

    def _check_should_retain(self, line: str) -> bool:
        """Process single line"""
        if line.startswith("SF:"):
            return True
        # Match line coverage data line
        elif line.startswith("DA:"):
            line_content = line[3:]
            line_parts = line_content.split(",")
            if len(line_parts) >= 2:
                try:
                    line_num = int(line_parts[0])
                    change_lines = runtime_config.change_info.get(self.source_file_path, set())
                    if line_num in change_lines:
                        return True
                except ValueError:
                    return False
        # Match function line
        elif line.startswith('FN:'):
            parts = line[3:].split(',')
            if len(parts) >= 2:
                try:
                    line_num = int(parts[0].strip())
                    func_name = parts[1].strip()
                    change_lines = runtime_config.change_info.get(self.source_file_path, set())
                    if line_num in change_lines:
                        self.related_funcs.add(func_name)
                        return True
                except ValueError:
                    return False
        # Match function coverage line
        elif line.startswith('FNDA:'):
            parts = line[5:].split(',')
            if len(parts) >= 2:
                try:
                    coverage = parts[0].strip()
                    func_name = parts[1].strip()
                    if func_name in self.related_funcs:
                        return True
                except ValueError:
                    return False
        # Match branch coverage line
        elif line.startswith('BRDA:'):
            parts = line[5:].split(',')
            if len(parts) >= 4:
                try:
                    if line in self.branch_info_for_diff:
                        line_range = self.branch_info_for_diff[line]
                        change_lines = runtime_config.change_info.get(self.source_file_path, set())
                        all_included = set(line_range).issubset(change_lines)
                        if all_included:
                            return True
                except ValueError:
                    return False
        elif line.startswith("end_of_record"):
            return True

        return False

    def _default_func_info(self) -> Dict:
        """Default function information template"""
        return {
            Constants.FUNCTION_NAME: "",
            Constants.LINE_TABLE: {},
        }

    def _parse_line_table(self, line_table_text: str) -> Dict[int, int]:
        """Parse line table text"""
        line_pattern = re.compile(Constants.LINE_TABLE_PATTERN)
        return {
            int(offset): int(line_num)
            for line_num, offset in line_pattern.findall(line_table_text)
        }

    def _process_ast_file(self, ast_file_path: str) -> bool:
        """Parse AST file"""
        try:
            content = FileUtils.read_file(ast_file_path)
            data = json.loads(content)
            self._traverse_ast(data)
            return True
        except Exception as e:
            print(f"Error parsing AST file {ast_file_path}: {e}")
            return False

    def _traverse_ast(self, node: Any):
        """Traverse AST nodes"""
        if isinstance(node, dict):
            self._process_ast_node(node)

            # Recursively traverse child nodes
            for key, value in node.items():
                if key != "parent":  # Avoid recursion trap
                    self._traverse_ast(value)

        elif isinstance(node, list):
            for item in node:
                self._traverse_ast(item)

    def _process_ast_node(self, node: Dict):
        """Process single AST node"""
        node_type = node.get(Constants.TYPE)

        # Process function declarations
        if node_type == Constants.NODE_METHOD_DEFINITION:
            self._process_function_node(node)

        # Process branch nodes
        elif node_type in Constants.BRANCH_TYPES:
            self._process_branch_node(node)

    def _process_function_node(self, node: Dict):
        """Process function node"""
        # Check if it is in the interface declaration, and if so, skip it
        if self._is_in_interface_declaration(node):
            return

        node_key = node.get(Constants.KEY)
        if node_key and Constants.TYPE in node_key:
            if node_key[Constants.TYPE] != Constants.NODE_IDENTIFIER:
                return
            func_name = node_key.get(Constants.NAME)
            define_line = node.get(Constants.LOC, {}).get(Constants.START, {}).get(Constants.LINE)
            if func_name and define_line and self._validate_function_name_in_source(func_name, define_line):
                self.function_definition_line[define_line] = func_name
                self.function_name_to_lines[func_name].add(define_line)

    def _is_in_interface_declaration(self, node: Dict) -> bool:
        """check if the node is in interface declaration"""
        value = node.get(Constants.VALUE)
        if value and isinstance(value, dict):
            function = value.get(Constants.FUNCTION)
            if function and isinstance(function, dict):
                body = function.get(Constants.BODY)
                if body is None:
                    return True

                if isinstance(body, dict):
                    statements = body.get(Constants.STATEMENTS)
                    if statements is None or (isinstance(statements, list) and len(statements) == 0):
                        return True

        return False

    def _parse_function_info(self, func_name: str, bc_offset: int, line_table_text: str) -> bool:
        """Validate if the function is defined in the source file and register it if valid."""
        if func_name == Constants.CTOR:
            func_name = Constants.CONSTRUCTOR
        if func_name.startswith("%%async-"):
            func_name = func_name.replace("%%async-", "")

        # Function is defined in source file
        if (
                func_name in self.function_name_to_lines
                or func_name.startswith(Constants.LAMBDA_INVOKE_1)
                or Constants.LAMBDA_INVOKE_2 in func_name
        ):
            # Parse line table
            line_table = self._parse_line_table(line_table_text)
            self.functions[bc_offset] = {
                Constants.FUNCTION_NAME: func_name,
                Constants.LINE_TABLE: line_table,
            }

    def _extract_function_name(self, node: Dict) -> str:
        """Extract function name"""
        id_node = node.get("id")
        if id_node and Constants.NAME in id_node:
            return id_node[Constants.NAME]
        return Constants.DEFAULT_ANONYMOUS_FUNC_NAME

    def _validate_function_name_in_source(self, func_name: str, start_line: int) -> bool:
        """Validate function name exists in source file"""
        try:
            pattern = Constants.FUNCTION_NAME_PATTERN.format(re.escape(func_name))
            content = FileUtils.read_file(self.source_file_path)
            lines = content.splitlines()
            if 0 < start_line <= len(lines):
                target_line = lines[start_line - 1]
                return re.search(pattern, target_line) is not None

        except Exception as e:
            print(f"Error validating function name: {e}")

        return False

    def _process_branch_node(self, node: Dict):
        """Process branch node"""
        node_type = node.get(Constants.TYPE)

        try:
            if node_type == Constants.SWITCH_STATEMENT:
                self._process_switch_statement(node)
            elif node_type == Constants.IF_STATEMENT:
                self._process_if_statement(node)
            elif node_type in [
                Constants.FOR_UPDATE_STATEMENT,
                Constants.FOR_IN_STATEMENT,
                Constants.FOR_OF_STATEMENT,
                Constants.WHILE_STATEMENT,
                Constants.DO_WHILE_STATEMENT
            ]:
                self._process_loop_statement(node)
            elif node_type == Constants.TRY_STATEMENT:
                self._process_try_statement(node)

        except Exception as e:
            print(f"Error processing branch node: {e}")

    def _process_switch_statement(self, node: Dict):
        """Process switch statement"""
        # Validate node parameter
        if not isinstance(node, dict):
            print("Warning: SwitchStatement node is not a dictionary")
            return

        switch_cases = node.get(Constants.CASES, [])
        if not isinstance(switch_cases, list):
            print("Warning: SwitchStatement cases is not a list")
            return

        # Validate required keys in node
        if Constants.LOC not in node or not isinstance(node[Constants.LOC], dict):
            print("Warning: SwitchStatement node missing loc information")
            return

        loc = node[Constants.LOC]
        if Constants.START not in loc or not isinstance(loc[Constants.START], dict):
            print("Warning: SwitchStatement node missing start information")
            return

        start = loc[Constants.START]
        if Constants.LINE not in start:
            print("Warning: SwitchStatement node missing line information")
            return

        start_line = start[Constants.LINE]
        for case_index, case in enumerate(switch_cases):
            # Validate case structure
            if not isinstance(case, dict):
                print(f"Warning: Case {case_index} is not a dictionary")
                continue

            # Check if case is SwitchCase type and has consequent statements
            if (
                    case.get(Constants.TYPE) == Constants.SWITCH_CASE
                    and Constants.CONSEQUENT in case
                    and isinstance(case[Constants.CONSEQUENT], list)
                    and len(case[Constants.CONSEQUENT]) > 0
            ):

                # Get the first consequent node
                consequent_list = case[Constants.CONSEQUENT]
                first_consequent = consequent_list[0]

                # Validate first consequent
                if not isinstance(first_consequent, dict):
                    print(
                        f"Warning: First consequent in case {case_index} is not a dictionary"
                    )
                    continue

                enter_node = first_consequent

                # Check if it's a block statement
                if (
                        Constants.TYPE in first_consequent
                        and first_consequent[Constants.TYPE] == Constants.NODE_BLOCK_STATEMENT
                ):
                    # For block statements, get the first statement in the block
                    statements = first_consequent.get(Constants.STATEMENTS)
                    if isinstance(statements, list) and len(statements) > 0:
                        enter_node = statements[0]

                # Create branch entry
                self._create_branch_entry(enter_node, start_line, case_index)

    def _process_if_branch(self, node: Dict, start_line: int, branch_index: int):
        """Process if statement branch (consequent or alternate)"""
        # Handle block statements
        if (
                Constants.TYPE in node and
                node[Constants.TYPE] == Constants.NODE_BLOCK_STATEMENT and
                Constants.STATEMENTS in node
        ):
            statements = node[Constants.STATEMENTS]
            if isinstance(statements, list) and len(statements) > 0:
                # Check for nested if statements in block
                first_statement = statements[0]
                if (
                        isinstance(first_statement, dict) and
                        first_statement.get(Constants.TYPE) == Constants.IF_STATEMENT
                ):
                    # For nested if statements, recursively process them
                    self._process_if_statement(first_statement)
                else:
                    self._create_branch_entry(first_statement, start_line, branch_index, branch_type='if')
        # Handle direct statements with STATEMENTS key
        elif Constants.STATEMENTS in node:
            statements = node[Constants.STATEMENTS]
            if isinstance(statements, list) and len(statements) > 0:
                # Check for nested if statements
                first_statement = statements[0]
                if (
                        isinstance(first_statement, dict) and
                        first_statement.get(Constants.TYPE) == Constants.IF_STATEMENT
                ):
                    # For nested if statements, recursively process them
                    self._process_if_statement(first_statement)
                else:
                    self._create_branch_entry(first_statement, start_line, branch_index, branch_type='if')
        # Handle direct node without STATEMENTS key
        else:
            # Check if it's a nested if statement
            if (
                    isinstance(node, dict) and
                    node.get(Constants.TYPE) == Constants.IF_STATEMENT
            ):
                # For nested if statements, recursively process them
                self._process_if_statement(node)
            else:
                self._create_branch_entry(node, start_line, branch_index, branch_type='if')

    def _process_if_statement(self, node: Dict):
        """Process if statement"""
        loc = node.get(Constants.LOC, {})
        start = loc.get(Constants.START, {})
        start_line = start.get(Constants.LINE)

        if not start_line:
            print(f"Warning: IfStatement node missing line information")
            return

        # Process consequent branch
        if (
                Constants.CONSEQUENT in node
                and isinstance(node[Constants.CONSEQUENT], dict)
        ):
            self._process_if_branch(node[Constants.CONSEQUENT], start_line, 0)

        # Process alternate branch (else)
        if (
                Constants.ALTERNATE in node
                and isinstance(node[Constants.ALTERNATE], dict)
        ):
            self._process_if_branch(node[Constants.ALTERNATE], start_line, 1)

    def _process_loop_statement(self, node: Dict):
        """Process loop statement"""
        loc = node.get(Constants.LOC, {})
        start_line = loc.get(Constants.START, {}).get(Constants.LINE)

        if not start_line:
            print(f"Warning: Loop statement node missing line information")
            return

        body = node.get(Constants.BODY)
        first_statement = None

        if isinstance(body, dict):
            # Prioritize attempting to retrieve from the statements list
            statements = None
            # Simplified condition check - if STATEMENTS exists, use it regardless of type
            if Constants.STATEMENTS in body:
                statements = body[Constants.STATEMENTS]

            if isinstance(statements, list) and statements:
                first_statement = statements[0]
            elif Constants.LOC in body:
                first_statement = body

        if first_statement:
            self._create_branch_entry(first_statement, start_line, 0, branch_type='loop')
        else:
            print(f"Warning: Loop statement has unexpected body structure")

    def _process_try_statement(self, node: Dict):
        """Process try statement"""
        loc = node.get(Constants.LOC, {})
        if not isinstance(loc, dict):
            print(f"Warning: TryStatement node has invalid loc information")
            return

        start = loc.get(Constants.START, {})
        if not isinstance(start, dict):
            print(f"Warning: TryStatement node has invalid start information")
            return

        start_line = start.get(Constants.LINE)
        if not start_line:
            print(f"Warning: TryStatement node missing line information")
            return

        case_index = 0

        # try block
        if (
                Constants.BLOCK in node
                and isinstance(node[Constants.BLOCK], dict)
                and Constants.STATEMENTS in node[Constants.BLOCK]
        ):
            block_statements = node[Constants.BLOCK][Constants.STATEMENTS]
            if isinstance(block_statements, list) and len(block_statements) > 0:
                self._create_branch_entry(block_statements[0], start_line, case_index)
            case_index += 1

        # catch block
        handlers = node.get(Constants.HANDLER, [])
        if isinstance(handlers, list):
            for handler in handlers:
                if (
                        isinstance(handler, dict)
                        and handler.get(Constants.TYPE) == Constants.NODE_CATCH_CLAUSE
                ):
                    body = handler.get(Constants.BODY, {})
                    if isinstance(body, dict) and Constants.STATEMENTS in body:
                        body_statements = body[Constants.STATEMENTS]
                        if (
                                isinstance(body_statements, list)
                                and len(body_statements) > 0
                        ):
                            self._create_branch_entry(
                                body_statements[0], start_line, case_index
                            )
                        case_index += 1

        # finally block
        if (
                Constants.FINALIZER in node
                and isinstance(node[Constants.FINALIZER], dict)
                and Constants.STATEMENTS in node[Constants.FINALIZER]
        ):
            finalizer_statements = node[Constants.FINALIZER][Constants.STATEMENTS]
            if isinstance(finalizer_statements, list) and len(finalizer_statements) > 0:
                self._create_branch_entry(
                    finalizer_statements[0], start_line, case_index
                )

    def _create_branch_entry(self, node: Dict, start_line: int,
                             case_index: int, default_count: int = 0, branch_type: str = 'other'):
        """Create branch entry"""
        if node and Constants.LOC in node:
            line_num = node[Constants.LOC][Constants.START].get(Constants.LINE)
            end_line = node[Constants.LOC][Constants.END].get(Constants.LINE)
            line_range = (line_num, end_line)
            if line_range not in self.branch_stats:
                self.branch_stats[line_range] = {
                    Constants.START_LINE: start_line,
                    Constants.BRANCH_INDEX: 0,
                    Constants.CASE_INDEX: case_index,
                    Constants.COUNT: default_count,
                    Constants.BRANCH_TYPE: branch_type
                }

    def _process_runtime_info(self, runtime_info: List[Dict]) -> bool:
        """Process runtime information"""
        if runtime_info is None or not runtime_info:
            print(f"Warning: No runtime information available for {self.source_file_path}")
            return False

        try:
            for row in runtime_info:
                self._process_runtime_row(row)
            return True
        except Exception as e:
            print(f"Error processing runtime information: {e}")
            return False

    def _process_runtime_row(self, row: Dict):
        """Process single row of runtime data"""
        func_id = row[Config.COLUMN_METHODID]
        bc_offset = row[Config.COLUMN_OFFSET]
        line_count = row[Config.COLUMN_COUNTER]

        func_info = self.functions.get(func_id)
        if not func_info:
            return
        # Record line number and branch execution count
        line_num = self._find_line_number(func_id, bc_offset)
        if line_num is not None:
            self.line_stats[line_num] += line_count
            for key_range in self.branch_stats.keys():
                if key_range[0] == line_num:
                    self.branch_stats[key_range][Constants.COUNT] += line_count
                    break

        func_name = func_info[Constants.FUNCTION_NAME]
        # Record function execution count
        if bc_offset == 0 and func_name in self.function_name_to_lines:
            # Find function definition line
            smaller_lines = [
                line
                for line in self.function_name_to_lines[func_name]
                if line <= line_num
            ]
            if smaller_lines:
                definition_line = max(smaller_lines)
                self.func_stats[f"{func_name}_{definition_line}"] += line_count

    def _find_line_number(self, func_id: int, bc_offset: int) -> Optional[int]:
        """Find line number corresponding to bytecode offset"""
        func_info = self.functions.get(func_id)
        if not func_info:
            return None
        return func_info[Constants.LINE_TABLE].get(bc_offset)

    def _generate_lcov_content(self) -> str:
        """Generate LCOV content"""
        lines = []

        # File header
        lines.extend([
            "TN:",
            f"SF:{self.source_file_path}"
        ])

        # Function information
        lines.extend(self._generate_function_lines())

        # Line coverage
        lines.extend(self._generate_line_coverage_lines())

        # Branch coverage
        lines.extend(self._generate_branch_coverage_lines())

        # File footer
        lines.append("end_of_record")

        return "\n".join(lines) + "\n"

    def _generate_function_lines(self) -> List[str]:
        """Generate function-related LCOV lines"""
        lines = []

        # Function definitions
        for start_line, name in sorted(self.function_definition_line.items()):
            lines.append(f"FN:{start_line},{name}_{start_line}")

        # Function execution counts
        for func_name, count in self.func_stats.items():
            lines.append(f"FNDA:{count},{func_name}")

        return lines

    def _generate_line_coverage_lines(self) -> List[str]:
        """Generate line coverage-related LCOV lines"""
        # Collect all line numbers
        line_coverage = {
            line_num: 0
            for func_info in self.functions.values()
            for line_num in func_info[Constants.LINE_TABLE].values()
        }

        # Update counts for executed line numbers
        line_coverage.update(self.line_stats)

        return [
            f"DA:{line_num},{count}"
            for line_num, count in sorted(line_coverage.items())
        ]

    def _generate_branch_coverage_lines(self) -> List[str]:
        """Generate branch coverage lines in LCOV format"""
        lines = []

        # collect all branches and group by(start_line, branch_index)
        branches_by_location = {}
        for _, branch in self.branch_stats.items():
            key = (branch[Constants.START_LINE], branch[Constants.BRANCH_INDEX])
            if key not in branches_by_location:
                branches_by_location[key] = {}
            branches_by_location[key][branch[Constants.CASE_INDEX]] = branch

        # process each group, ensure complete branch information
        for (start_line, branch_index), case_branches in branches_by_location.items():
            # check if only one branch exists
            if len(case_branches) == 1:
                existing_case_index = list(case_branches.keys())[0]
                existing_branch = case_branches[existing_case_index]
                existing_count = existing_branch[Constants.COUNT]

                # get branch type, default is 'other'
                branch_type = existing_branch.get(Constants.BRANCH_TYPE, 'other')
                missing_case_index = 1
                n = self.line_stats.get(start_line, 0)
                if branch_type == 'loop':
                    missing_count = 1 if n > 0 else 0
                else:
                    missing_count = max(0, n - existing_count)

                # add missing branch
                missing_branch = {
                    Constants.START_LINE: start_line,
                    Constants.BRANCH_INDEX: branch_index,
                    Constants.CASE_INDEX: missing_case_index,
                    Constants.COUNT: missing_count,
                    Constants.BRANCH_TYPE: branch_type  # retain the branch type
                }
                case_branches[missing_case_index] = missing_branch

            # generate all branch BRDA lines, sorted by case_index
            for case_index in sorted(case_branches.keys()):
                branch = case_branches[case_index]
                brda_line = (
                    f"BRDA:{branch[Constants.START_LINE]},"
                    f"{branch[Constants.BRANCH_INDEX]},"
                    f"{branch[Constants.CASE_INDEX]},"
                    f"{branch[Constants.COUNT]}"
                )
                lines.append(brda_line)

                if runtime_config.report_type == 'diff':
                    # find original branch_key or use None
                    original_branch_key = None
                    for key, b in self.branch_stats.items():
                        if (b[Constants.START_LINE] == start_line and
                            b[Constants.BRANCH_INDEX] == branch_index and
                            b[Constants.CASE_INDEX] == case_index):
                            original_branch_key = key
                            break
                    self.branch_info_for_diff[brda_line] = original_branch_key

        return lines


class HostSimpleCoverageInfo(CoverageInfo):
    """Host simple coverage information class"""

    def __init__(self):
        super().__init__()

    def _process_pa_file(self, pa_file_path: str) -> bool:
        """Parse assembly file and build function information dictionary"""
        try:
            content = FileUtils.read_file(pa_file_path)

            function_pattern = re.compile(
                Constants.FUNCTION_PATTERN,
                re.DOTALL
            )

            for match in function_pattern.finditer(content):
                full_signature = match.group(1).strip()
                func_name = full_signature.split("(")[0]
                bc_offset = int(match.group(2), 16)
                line_table_text = match.group(3)
                self._parse_function_info(func_name, bc_offset, line_table_text)

            return True

        except Exception as e:
            print(f"Error parsing PA file {pa_file_path}: {e}")
            return False


class HapCoverageInfo(CoverageInfo):
    """Hap coverage information class"""

    def __init__(self):
        super().__init__()
        self.source_file_path_trans = None
        self._function_pattern = None
        self._get_pa_file_content()

    def reset_stats(self):
        super().reset_stats()
        self.source_file_path_trans = None
        self._function_pattern = None
        self._get_pa_file_content()

    def process_data(self, src_file_path: str, pa_file_path: str,
                     ast_file_path: str, runtime_info: List[Dict]):
        """Set file paths and parse"""
        self.reset_stats()
        self.source_file_path = src_file_path
        self.source_file_path_trans = self._trans_source_path(src_file_path)
        if self.source_file_path_trans is None:
            print("source_file_path_trans is none!")
            return
        # Parse files
        self.process_file_result = (
                self._process_ast_file(ast_file_path) and
                self._process_pa_file() and
                self._process_runtime_info(runtime_info)
        )

    def _get_pa_file_content(self):
        pa_file_path = os.path.join(runtime_config.workdir, Config.PA_DIR)
        pa_file = find_first_pa_file(pa_file_path)
        if pa_file is not None:
            self._pa_file_content = FileUtils.read_file(pa_file)
        else:
            self._pa_file_content = None
            print(f"get nothing from pa file: {pa_file}")

    def _is_valid_path_prefix(self, prefix: str, full_path: str) -> bool:
        """Check if the full path starts with the given prefix"""
        path_part = full_path.split(' ', 1)[-1]

        prefix_parts = prefix.split('.')
        path_parts = path_part.split('.')

        if len(path_parts) < len(prefix_parts):
            return False

        for i, prefix_part in enumerate(prefix_parts):
            if prefix_part != path_parts[i]:
                return False

        if len(path_parts) == len(prefix_parts):
            return True

        last_prefix_part = prefix_parts[-1]
        next_part_after_prefix = path_parts[len(prefix_parts)]

        if next_part_after_prefix.startswith(last_prefix_part) and len(next_part_after_prefix) > len(last_prefix_part):
            return False

        return True

    def _get_function_pattern(self):
        if self._function_pattern is None:
            self._function_pattern = re.compile(
                Constants.FUNCTION_PATTERN_HAP,
                re.DOTALL
            )
        return self._function_pattern

    def _trans_source_path(self, src_file_path: str):
        match = re.search(r'entry/.+?\.ets$', src_file_path)
        if match:
            file_path = Path(match.group())
            file_part = file_path.name
            dir_part = '.'.join(file_path.parent.parts)
            return f"{dir_part}.{file_part.replace('.ets', '')}"
        else:
            print("src path is not expected")
            return None

    def _process_pa_file(self) -> bool:
        """Parse assembly file and build function information dictionary"""
        try:
            if self._pa_file_content is None:
                print(f"Warning: No PA file content available for {self.source_file_path}")
                return False

            function_pattern = self._get_function_pattern()

            for match in function_pattern.finditer(self._pa_file_content):
                path_part = match.group(1)
                if not self._is_valid_path_prefix(self.source_file_path_trans, path_part):
                    continue
                full_signature = match.group(2).strip()
                func_name = full_signature.split("(")[0]
                bc_offset = int(match.group(3), 16)
                line_table_text = match.group(4)
                self._parse_function_info(func_name, bc_offset, line_table_text)

            return True

        except Exception as e:
            print(f"Error parsing PA file, source_file_path: {self.source_file_path}: {e}")
            return False


class HostMultiCoverageInfo(HapCoverageInfo):
    """HostMulti coverage information class"""

    def _trans_source_path(self, src_file_path: str):
        match = re.search(r'.+?\.ets$', src_file_path)
        if match:
            file_part = Path(match.group()).name
            dir_part = '.'.join(Path(match.group()).parent.parts)
            dir_part = dir_part.lstrip('.')
            return f"{dir_part}.{file_part.replace('.ets', '')}"
        else:
            print("src path is not expected")
            return None


# ==================== Utility Functions ====================
def create_file_mapping(src_dir: str, ast_dir: str, pa_dir: str) -> List[Dict]:
    """Create file mapping list"""
    src_path = Path(src_dir)
    data = []

    for ets_file in src_path.rglob(f"*{Constants.EXT_ETS}"):
        relative_path = ets_file.relative_to(src_path)

        if (
            runtime_config.mode == "host"
            or (
                runtime_config.mode == "hap"
                and "entry/src/main/ets" in relative_path.as_posix()
            )
            or runtime_config.mode == "host-multi"
        ):
            data.append({
                "src": str(src_path / relative_path),
                "ast": str(Path(ast_dir) / relative_path.with_suffix(Constants.EXT_JSON)),
                "pa": str(Path(pa_dir) / relative_path.with_suffix(Constants.EXT_PA)),
            })

    return data


def print_file_header(csv_file: Path, f: io.TextIOWrapper, max_lines: int = 10):
    """Print header information for a CSV file"""
    total_lines = 0
    first_lines = []

    # First pass: count lines and collect first 10 lines
    for line in f:
        total_lines += 1
        if total_lines <= max_lines:
            first_lines.append(line.strip())

    print(f"Total lines in {csv_file.name}: {total_lines}")
    print(f"First {max_lines} lines:")
    for i, line_content in enumerate(first_lines, 1):
        print(f"Line {i}: {line_content}")

    print("-" * 70)

    # Reset file pointer and continue with original processing
    f.seek(0)


def summarize_runtime_data(runtime_dir: str,
                           delimiter: str = ',',
                           skip_errors: bool = True,
                           max_errors: int = 10) -> Optional[List[Dict]]:
    """Summarize runtime data"""
    try:
        csv_files = FileUtils.find_files(runtime_dir, f"*{Constants.EXT_CSV}")
        if not csv_files:
            print(f"No CSV files {runtime_dir} found")
            return None

        # Use a dictionary to group by (methodid, offset) and sum counters
        grouped_data = defaultdict(int)
        total_lines = 0
        error_count = 0
        skipped_lines = []

        for csv_file in csv_files:
            print(f"Processing file: {csv_file.name}")

            with open(csv_file, 'r', encoding='utf-8') as f:
                print_file_header(csv_file, f)

                for line_num, line in enumerate(f, 1):
                    try:
                        line = line.strip()

                        # Skip empty lines and comment lines
                        if not line or line.startswith('#'):
                            continue

                        # Split line data
                        parts = line.split(delimiter)
                        if len(parts) != 3:
                            raise ValueError(f"Incorrect number of columns, expected 3, got {len(parts)}")

                        # Convert data types
                        methodid = int(parts[0].strip())
                        offset = int(parts[1].strip())
                        counter = int(parts[2].strip())

                        # Validate data range
                        if methodid < 0 or offset < 0 or counter < 0:
                            raise ValueError("Values cannot be negative")

                        # Group and sum
                        grouped_data[(methodid, offset)] += counter
                        total_lines += 1

                    except Exception as e:
                        error_count += 1
                        skipped_lines.append((line_num, line, str(e)))

                        if not skip_errors:
                            raise

                        if error_count <= max_errors:
                            print(f"Warning: Error at line {line_num} - {e}")

        # Error statistics
        if error_count > 0:
            print(f"Warning: Total {error_count} error lines skipped")
            if error_count > max_errors:
                print(f"(Only showing first {max_errors} errors)")

        if not grouped_data:
            print("Error: No valid runtime data")
            return None

        # Convert to list of dictionaries
        result = [
            {
                Config.COLUMN_METHODID: methodid,
                Config.COLUMN_OFFSET: offset,
                Config.COLUMN_COUNTER: counter
            }
            for (methodid, offset), counter in grouped_data.items()
        ]

        print(f"Successfully processed: {total_lines} lines of runtime data")
        return result

    except FileNotFoundError:
        print(f"File not found: {runtime_dir}")
        return None
    except PermissionError:
        print(f"Permission error: Cannot read file")
        return None
    except Exception as e:
        print(f"Error summarizing runtime data: {e}")
        return None


def find_first_pa_file(pa_dir):
    """Returns the path of the first .pa file found, or None if no file is found."""
    for dirpath, _, filenames in os.walk(pa_dir):
        for filename in filenames:
            if filename.lower().endswith('.pa'):
                return os.path.join(dirpath, filename)
    return None


def export_lcov(args):
    """Export coverage information in LCOV format"""
    try:
        # Validate directories
        Config.validate_directories()

        # Create file mapping
        file_mappings = create_file_mapping(
            args.src,
            str(Path(runtime_config.workdir) / Config.AST_DIR),
            str(Path(runtime_config.workdir) / Config.PA_DIR)
        )
        print(f"Found {len(file_mappings)} file mappings")

        # Summarize runtime data
        runtime_df = summarize_runtime_data(Config.get_runtime_dir())
        if runtime_df is None:
            print("No runtime data found")
            return

        # Delete old result file
        if Path(Config.get_output_file()).exists():
            Path(Config.get_output_file()).unlink()

        # Process each file

        success_count = 0
        if runtime_config.mode == 'host':
            coverage_info = HostSimpleCoverageInfo()
        elif runtime_config.mode == 'host-multi':
            coverage_info = HostMultiCoverageInfo()
        else:
            coverage_info = HapCoverageInfo()

        for row in file_mappings:
            try:
                coverage_info.process_data(
                    row["src"],
                    row["pa"],
                    row["ast"],
                    runtime_df
                )
                if coverage_info.export_lcov(Config.get_output_file()):
                    success_count += 1

            except FileNotFoundError as e:
                print(f"File not found: {e}")
            except Exception as e:
                print(f"Error processing file {row['src']}: {e}")

        if runtime_config.report_type == 'all':
            print(f"Successfully processed {success_count}/{len(file_mappings)} files")
        else:
            if runtime_config.change_info is None:
                print("change_info is none!")
                return
            num_files = len(runtime_config.change_info)
            print(f"Successfully processed {success_count}/{num_files} files")


    except Exception as e:
        print(f"Program execution error: {e}")


def gen_pa(args):
    print(f"Generating PA from {args.src} and outputting to {args.output}")
    src_path = Path(args.src)
    output_path = Path(args.output)

    # Clean output directory if it exists
    if output_path.exists():
        print(f"Cleaning existing output directory: {output_path}")
        shutil.rmtree(output_path)

    # Create output directory
    output_path.mkdir(parents=True, exist_ok=True)
    # Find all .abc files in the source directory and its subdirectories
    abc_files = list(src_path.rglob("*.abc"))

    if not abc_files:
        print(f"No .abc files found in {args.src}")
        sys.exit(1)

    print(f"Found {len(abc_files)} .abc files to process")

    # Process each .abc file
    for abc_file in abc_files:
        # Calculate relative path from source directory to maintain directory structure
        relative_path = abc_file.relative_to(src_path)
        # Create corresponding output file path with .pa extension
        pa_file = output_path / relative_path.with_suffix('.pa')
        # Create parent directory for the output file
        pa_file.parent.mkdir(parents=True, exist_ok=True)

        cmd = [
            shlex.quote(args.ark_disasm_path),
            "--verbose",
            shlex.quote(str(abc_file)),
            shlex.quote(str(pa_file))
        ]
        print(f"Execute command: {' '.join(cmd)}")
        try:
            result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print(f"Error occurred while processing {abc_file}:")
            print(f"Command: {' '.join(e.cmd)}")
            print(f"Return code: {e.returncode}")
            if e.stdout:
                print(f"Stdout: {e.stdout}")
            if e.stderr:
                print(f"Stderr: {e.stderr}")
            sys.exit(1)

    print(f"Successfully processed all {len(abc_files)} .abc files")


class ArktsconfigGenerator:
    def add_optional_arguments(self, parser: argparse.ArgumentParser) -> None:
        parser.add_argument("--std-path", required=False, default=None,
                        help="Path to the standard library")
        parser.add_argument("--escompat-path", required=False, default=None, help="Path to the escompat library")
        parser.add_argument("--ets-arkts-path", required=False, nargs='+', default=[],
                            help="Path to the ets-arkts library")
        parser.add_argument("--ets-api-path", required=False, nargs='+', default=[],
                            help="Path to the ets-api library")
        parser.add_argument("--include", required=False, nargs='+', default=None, help="Files or patterns to include")
        parser.add_argument("--exclude", required=False, nargs='+', default=None, help="Files or patterns to exclude")
        parser.add_argument("--files", required=False, default=None,
                            help="Path to file containing list of files to process")
        parser.add_argument("--cache-path", type=str, default=None, help="Path to cache directory")
        parser.add_argument("--package", required=False, default="", help="Package name for the project")
        
        parser.add_argument("--paths-keys", nargs="+", required=False, default=[],
                        help="List of keys for custom paths")
        parser.add_argument("--paths-values", nargs="+", required=False, default=[],
                        help="List of values for custom paths. Each value corresponds to a key in --paths-keys")

    def generate(self, args):
        paths = {}
        ets_arkts_paths = args.ets_arkts_path
        ets_api_paths = args.ets_api_path

        for scan_path in ets_arkts_paths + ets_api_paths:
            scanned_paths = self._scan_directory_for_paths(scan_path)
            for key, value in scanned_paths.items():
                if key in paths:
                    paths[key].extend(value)
                else:
                    paths[key] = value

        if args.std_path:
            paths["std"] = [os.path.abspath(args.std_path)]
        if args.escompat_path:
            paths["escompat"] = [os.path.abspath(args.escompat_path)]

        paths_keys = args.paths_keys
        paths_values = args.paths_values
        if paths_keys and paths_values:
            if len(paths_keys) != len(paths_values):
                class PathsLengthMismatchError(Exception):
                    pass
                raise PathsLengthMismatchError(
                    "paths_keys and paths_values must have the same length"
                )
            for key, value in zip(paths_keys, paths_values):
                paths[key] = [os.path.abspath(value)]

        config = {
            "compilerOptions": {
                "rootDir": os.path.abspath(Path(args.src)),
                "baseUrl": os.path.abspath(Path(args.src).parent),
                "paths": paths,
                "outDir": args.cache_path or os.path.join(args.output, 'cache'),
                "package": args.package,
                "useEmptyPackage": True
            }
        }

        if args.include:
            config["include"] = args.include

        if args.exclude:
            config["exclude"] = args.exclude

        files_path = args.files
        if files_path:
            if not os.path.exists(files_path):
                print(f"[IO ERROR] File not found: {files_path}", file=sys.stderr)
                sys.exit()
            fd = os.open(files_path, os.O_RDONLY)
            with os.fdopen(fd, 'r') as f:
                config["files"] = [line.strip() for line in f.readlines()]

        config_path = os.path.join(args.output, "arktsconfig.json")
        os.makedirs(os.path.dirname(config_path), exist_ok=True)
        fd = os.open(config_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o777)
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            json.dump(config, f, indent=2, ensure_ascii=False)
        return config_path

    def _get_key_from_file_name(self, file_name: str) -> str:
        if ".d." in file_name:
            file_name = file_name.replace(".d.", ".")
        return os.path.splitext(file_name)[0]

    def _is_target_file(self, file_name: str) -> bool:
        target_extensions = [".d.ets", ".ets"]
        return any(file_name.endswith(ext) for ext in target_extensions)

    def _scan_directory_for_paths(self, directory: str) -> Dict[str, List[str]]:
        paths = {}
        for root, _, files in os.walk(directory):
            for file in files:
                if not self._is_target_file(file):
                    continue
                file_path = os.path.abspath(os.path.join(root, file))
                file_name = self._get_key_from_file_name(file)
                file_abs_path = os.path.abspath(os.path.join(root, file_name))
                file_rel_path = os.path.relpath(file_abs_path, start=directory)
                # Split the relative path into components
                path_components = file_rel_path.split(os.sep)
                first_level_dir = path_components[0] if len(path_components) > 0 else ""
                second_level_dir = path_components[1] if len(path_components) > 1 else ""
                # Determine the key based on directory structure
                if first_level_dir == "arkui" and second_level_dir == "runtime-api":
                    key = file_name
                else:
                    key = file_rel_path.replace(os.sep, ".")
                if key in paths:
                    paths[key].append(file_path)
                else:
                    paths[key] = [file_path]
        return paths


class GenAst:
    def __init__(self, init_mode='host'):
        self.mode = init_mode
        self.compile_count = 0
        self.ets_files = 0
        self._lock = threading.Lock()
        self.src_path = None
        self.es2panda_path = None
        self.ast_dir = None
        self.abc_dir = None
        self.arktsconfig_path = None

    def gen_ast(self, args):
        start_time = time.time()
        error_occurred = False
        if self.mode == 'host-multi':
            self.arktsconfig_path = ArktsconfigGenerator().generate(args)

        self.src_path = Path(args.src).resolve()
        if not self.src_path.exists():
            print(f"Error: Source directory '{self.src_path}' does not exist")
            return False

        self.es2panda_path = Path(args.es2panda).resolve()
        if not self.es2panda_path.exists():
            print(f"Error: es2panda executable '{self.es2panda_path}' does not exist")
            return False

        output_path = Path(args.output).resolve()
        output_path.mkdir(parents=True, exist_ok=True)
        self.ast_dir = output_path / "ast"
        self.abc_dir = output_path / "abc"
        # Clear existing ast and abc directories before processing
        if self.ast_dir.exists():
            shutil.rmtree(self.ast_dir)
        if self.abc_dir.exists():
            shutil.rmtree(self.abc_dir)
        self.ast_dir.mkdir(parents=True, exist_ok=True)
        self.abc_dir.mkdir(parents=True, exist_ok=True)

        self.ets_files = list(self.src_path.rglob(f"*{Constants.EXT_ETS}"))
        if not self.ets_files:
            print(f"Warning: No .ets files found in '{self.src_path}'")
            return True

        print(f"Found {len(self.ets_files)} .ets files, Mode: {self.mode}")

        # Always use multithreaded processing
        max_workers = min(os.cpu_count() or 4, len(self.ets_files))
        print(f"Using {max_workers} threads for parallel processing")

        with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
            future_to_file = {
                executor.submit(self._process_ets_file, ets_file): ets_file
                for ets_file in self.ets_files
            }

            for future in concurrent.futures.as_completed(future_to_file):
                success, rel_path, error_info = future.result()

                if self._handle_compilation_error(rel_path, error_info):
                    print("Terminating due to compilation error in host mode")
                    for f in future_to_file:
                        if not f.done():
                            f.cancel()
                    error_occurred = True
                    sys.exit(1)

        if self.mode == 'host-multi':
            if not self._link_abc(args.ark_link, self.abc_dir, args.abc_link_name):
                print(f"Error: Failed to link ABC files in {self.abc_dir} to {args.abc_link_name}")
                sys.exit(1)
        if not error_occurred:
            elapsed_time = time.time() - start_time
            print(f"Processing complete: {self.compile_count}/{len(self.ets_files)} files succeeded")
            print(f"AST/ABC files saved to: {output_path}")
            print(f"Compilation time: {elapsed_time:.2f} seconds")
        return not error_occurred

    def _validate_ast_file(self, ast_file_path):
        """Validate if AST file is generated correctly and return result with error message."""
        try:
            with open(ast_file_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            return ast_file_path.exists() and ast_file_path.stat().st_size > 0 and data is not None
        except Exception:
            return False

    def _process_ets_file(self, ets_file):
        """Work function for processing a single ETS file"""
        with self._lock:
            print(f"[{self.compile_count}/{len(self.ets_files)}] compiling file: {ets_file}")
            self.compile_count += 1

        try:
            rel_path = ets_file.relative_to(self.src_path)
            ast_output = self.ast_dir / rel_path.with_suffix(Constants.EXT_JSON)
            abc_output = self.abc_dir / rel_path.with_suffix(".abc")
            ast_output.parent.mkdir(parents=True, exist_ok=True)
            abc_output.parent.mkdir(parents=True, exist_ok=True)

            if self.mode == 'host-multi':
                cmd = [str(self.es2panda_path), str(ets_file), f"--dump-ast:output={ast_output}", "--opt-level=0",
                       "--output", str(abc_output), "--extension", "ets", "--arktsconfig", str(self.arktsconfig_path)]
            else:
                cmd = [str(self.es2panda_path), str(ets_file), f"--dump-ast:output={ast_output}", "--opt-level=0",
                       "--output", str(abc_output)]
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

            if result.returncode != 0 and self.mode != 'hap':
                error_msg = (
                        result.stderr.strip()
                        or result.stdout.strip()
                        or f"Command failed with return code {result.returncode}"
                )
                return False, rel_path, (error_msg, result.returncode)

            if self.mode == 'hap' and abc_output.exists():
                try:
                    abc_output.unlink()
                except Exception as e:
                    print(f"Warning: Failed to delete ABC file {abc_output}: {e}")

            if not self._validate_ast_file(ast_output):
                base_msg = f"AST file validation failed: {ast_output}"
                error_details = result.stderr.strip() or result.stdout.strip() or ""
                error_msg = f"{base_msg}\n{error_details}" if error_details else base_msg
                return False, rel_path, (error_msg, None)

            return True, rel_path, None
        except Exception as e:
            return False, rel_path, (str(e), None)

    def _handle_compilation_error(self, rel_path, error_info):
        """Handle compilation error information"""
        if self.mode == 'hap' or not error_info:
            return False

        error_msg, return_code = error_info
        print(f"Error processing {rel_path}:\n{error_msg}")
        if return_code is not None:
            print(f"Return code: {return_code}")
        return return_code is not None

    def _clear_abc(self, src_path, abc_dir):
        if not abc_dir.exists():
            print(f"Error: ABC directory '{abc_dir}' does not exist")
            return False
        src = Path(src_path).resolve()
        abc_dir_resolved = abc_dir.resolve()
        if src == abc_dir_resolved or not abc_dir_resolved.is_relative_to(src):
            print(f"Error: abc_dir must be a subdirectory of src_path")
            return False
        for item in src.iterdir():
            if item != abc_dir_resolved:
                if item.is_dir():
                    shutil.rmtree(item)
                else:
                    item.unlink()
        print(f"Cleared all files and directories in '{src_path}' except '{abc_dir}'")
        return True

    def _link_abc(self, ark_link_path, src_path, output_name):
        src = Path(src_path).resolve()
        if not src.exists():
            print(f"Error: Source directory '{src_path}' does not exist")
            return False

        ark_link_path = Path(ark_link_path).resolve()
        if not ark_link_path.exists():
            print(f"Error: ark_link executable not found at {ark_link_path}")
            return False

        abc_files = list(src.rglob("*.abc"))
        if not abc_files:
            print(f"Error: No .abc files found in '{src_path}'")
            return False

        output_file = src / output_name
        output_file.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            str(ark_link_path),
            "--output",
            str(output_file),
            "--"
        ]
        cmd.extend(str(abc_file) for abc_file in abc_files)
        print(f"Linking ABC files: {' '.join(cmd)}")
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if result.returncode != 0:
            error_msg = result.stderr.strip() or result.stdout.strip() or f"Command failed code {result.returncode}"
            print(f"Error linking ABC files: {error_msg}")
            return False

        print(f"Successfully linked ABC files to {output_file}")
        if not self._clear_abc(src, output_file):
            print(f"Failed to clear ABC files: {output_file}")
        return True


def get_diff_lines(diff_file_path):
    """
    Read change information from diff.txt file and generate info file containing SF, DA and end_of_record

    Parameters:
        diff_file_path: diff.txt file path
        output_info_path: output info file path
    """
    # Dictionary to store change information, format: {file_path: {line_number_set}}
    diff_info = {}

    # Regular expressions
    file_pattern = re.compile(r'^\+\+\+ b/(.*)$')  # Match file path
    hunk_pattern = re.compile(r'^@@ -(\d+),(\d+) \+(\d+),(\d+) @@')  # Match code block
    add_line_pattern = re.compile(r'^\+(?!\+\+).*$')  # Match added lines
    del_line_pattern = re.compile(r'^\-(?!\-\-).*$')  # Match deleted lines

    current_file = None
    current_hunk_start_new = None  # Starting line number in new file
    lines_in_hunk_new = 0  # Line counter in new file

    # Step 1: Parse diff.txt to get change information
    try:
        with open(diff_file_path, 'r', encoding='utf-8') as f:
            for line in f:
                # Skip "\ No newline at end of file" line
                if line.strip() == r'\ No newline at end of file':
                    continue

                # Match file path
                file_match = file_pattern.match(line)
                if file_match:
                    current_file = file_match.group(1)
                    # Initialize line number set for this file
                    diff_info[current_file] = set()
                    current_hunk_start_new = None
                    lines_in_hunk_new = 0
                    continue

                # Match code block, get starting line number in new file
                hunk_match = hunk_pattern.match(line)
                if hunk_match and current_file:
                    current_hunk_start_new = int(hunk_match.group(3))  # Starting line number in new file
                    lines_in_hunk_new = 0
                    continue

                # Match deleted lines (but we don't track them in diff_info as they don't exist in new file)
                if del_line_pattern.match(line) and line.strip() != '-':
                    continue

                # Match added lines and calculate actual line number
                if (current_file and current_hunk_start_new is not None and
                        add_line_pattern.match(line) and line.strip() != '+'):
                    # Calculate actual line number in new file
                    actual_line_num = current_hunk_start_new + lines_in_hunk_new
                    diff_info[current_file].add(actual_line_num)
                    lines_in_hunk_new += 1
                else:
                    # For unchanged lines or other lines, increment both counters
                    if not del_line_pattern.match(line):  # Skip incrementing new file counter for deleted lines
                        lines_in_hunk_new += 1
        return diff_info
    except FileNotFoundError:
        print(f"Error: Cannot find diff file '{diff_file_path}'")
        return None
    except Exception as e:
        print(f"Error parsing diff file: {str(e)}")
        return None


def validate_result_file():
    output_file_path = Config.get_output_file()
    cmd = ['lcov', '--list', str(output_file_path)]
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        timeout=30
    )

    if result.returncode != 0:
        print(f"Warning: {output_file_path} is illegal, and the incremental code may be invalid." \
              f" return code {result.returncode}")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description='Coverage report generation tool')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Add gen_report subcommand
    report_parser = subparsers.add_parser('gen_report', help='Generate coverage report')
    report_parser.add_argument('--src', required=True, help='Source code directory path')
    report_parser.add_argument('--workdir', '-w', type=str, default='.', help='Work directory')
    report_parser.add_argument('--mode', '-m', type=str, choices=['host', 'host-multi', 'hap'], default='host',
                               help='Mode')
    report_group = report_parser.add_mutually_exclusive_group()
    report_group.add_argument('--all', '-a', action='store_true', help='Full coverage report')
    report_group.add_argument('--diff', '-d', action='store_true', help='Incremental coverage report')

    # Add gen_ast subcommand
    gen_ast_parser = subparsers.add_parser('gen_ast', help='Generate AST and ABC files from .ets source files')
    gen_ast_parser.add_argument('--src', required=True, help='Source code directory path')
    gen_ast_parser.add_argument('--es2panda', required=True, help='Path to es2panda executable')
    gen_ast_parser.add_argument('--ark-link', help='Path to ark_link executable')
    gen_ast_parser.add_argument('--output', required=True, help='Output directory path')
    gen_ast_parser.add_argument('--abc-link-name', help='Name of the linked ABC file in host-multi mode')
    gen_ast_parser.add_argument('--mode', '-m', type=str, choices=['host', 'hap', 'host-multi'], default='host',
                                help='Mode')
    ArktsconfigGenerator().add_optional_arguments(gen_ast_parser)

    # Add gen_pa subcommand
    pa_parser = subparsers.add_parser('gen_pa', help='Generate PA data')
    pa_parser.add_argument('--src', required=True, help='Source directory')
    pa_parser.add_argument('--output', default='.', help='Output directory (default: .)')
    pa_parser.add_argument('--ark_disasm-path', required=True, help='Path to ark_disasm executable')

    args = parser.parse_args()
    if args.command in ['gen_report', 'gen_ast']:
        runtime_config.mode = args.mode
        print(f"mode: {runtime_config.mode}")

    if args.command == 'gen_report':
        # Check if genhtml command exists
        genhtml_path = shutil.which('genhtml')
        if not genhtml_path:
            print("Error: genhtml command not found. Please install lcov package.")
            sys.exit(1)

        runtime_config.workdir = args.workdir
        runtime_config.report_type = 'all' if args.all else 'diff'
        print(f"report_type: {runtime_config.report_type}, workdir: {runtime_config.workdir}")

        # generate full coverage report or incremental coverage report
        if (args.all or args.diff):
            # Get the lines that have been changed
            if (args.diff):
                runtime_config.change_info = get_diff_lines('diff.txt')
                if runtime_config.change_info is None:
                    print("change_info is none!")
                    sys.exit(1)

            # Get result.info
            export_lcov(args)
            # Check if result.info file exists
            result_file = os.path.join(runtime_config.workdir, 'result.info')
            if not os.path.exists(result_file):
                print(f"Error: {runtime_config.workdir}/result.info file does not exist")
                sys.exit(1)

            try:
                # Check if there are valid data in result.info
                validate_result_file()

                # Execute genhtml command to generate full coverage report or incremental coverage report
                cmd = [genhtml_path, str(result_file), '-o', str(os.path.join(runtime_config.workdir, 'report')), '-s',
                       '--branch-coverage']
                subprocess.run(
                    cmd,
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
                print("Coverage report generated in report directory")
            except subprocess.CalledProcessError as e:
                print(f"Failed to generate coverage report: {e.stderr}")
                print(f"Command: {' '.join(e.cmd)}")
                print(f"Return code: {e.returncode}")
                print(f"Stdout: {e.stdout}")
                sys.exit(1)

        else:
            print("Please specify a parameter")
    elif args.command == 'gen_ast':
        ast_generator = GenAst(init_mode=runtime_config.mode)
        ast_generator.gen_ast(args)
    elif args.command == 'gen_pa':
        gen_pa(args)
    else:
        print("Please specify a command")


if __name__ == "__main__":
    main()
