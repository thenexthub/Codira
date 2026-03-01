#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import re
from functools import cached_property
from pathlib import Path
from typing import Any, Optional, cast

from runner import utils
from runner.common_exceptions import FileNotFoundException
from runner.logger import Log
from runner.options.macros import MacroNotExpanded, Macros, ParameterNotFound
from runner.options.options import IOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.step import Step, StepKind
from runner.utils import indent as utils_indent

_LOGGER = Log.get_logger(__file__)

WORKFLOW = "workflow"
WORKFLOW_NAME = "workflow-name"
WORKFLOW_KIND = "workflow-kind"
PARAMETERS = "parameters"
STEPS = "steps"
FROM = "from"


class WorkflowOptions(IOptions):

    def __init__(self, cfg_content: dict[str, Any], parent_test_suite: TestSuiteOptions,  # type: ignore[explicit-any]
                 *, parent_workflow: Optional['WorkflowOptions'] = None,
                 parent_step_name: str | None = None) -> None:
        super().__init__(None)
        self._parent = parent_test_suite if parent_workflow is None else parent_workflow
        inner = parent_workflow is not None
        self.__name = cfg_content[WORKFLOW] if not inner else cfg_content[WORKFLOW_NAME]
        self.__data = cfg_content[f"{self.__name}.data"] if not inner else cfg_content
        self.__test_suite = parent_test_suite
        self.__steps: list[Step] = []
        self.workflow_type = self.__data.get(WORKFLOW_KIND, "ets")
        self.parent_step_name: str = parent_step_name if parent_step_name is not None else ""
        workflow_kind = "It's a main config" if not inner \
            else f"It's a nested config with the parent '{parent_workflow.__getattribute__('name')}'"
        _LOGGER.all(f"Going to load workflow '{self.__name}'. {workflow_kind}")
        if not inner:
            self.__parameters = dict((arg_key[(len(self.__name) + len(PARAMETERS) + 2):], arg_value)
                                     for arg_key, arg_value in cfg_content.items()
                                     if arg_key.startswith(f"{self.__name}.{PARAMETERS}."))
            for param, value_in_workflow in self.__parameters.items():
                value_in_test_suite = self.__test_suite.get_parameter(param)
                if value_in_test_suite is not None:
                    self.__parameters[param] = value_in_test_suite
                    value_in_workflow = value_in_test_suite
                self.__parameters[param] = self.__expand_macro_for_param(value_in_workflow, param)
        else:
            self.__parameters = cfg_content[PARAMETERS]
        self.__load_steps(self.__data[STEPS])

    def __str__(self) -> str:
        indent = 2
        result = [f"{self.__name}\n{utils_indent(indent)}{PARAMETERS}:"]
        for param_name in sorted(self.__parameters.keys()):
            param_value = self.__parameters[param_name]
            result.append(f"{utils_indent(indent + 1)}{param_name}: {param_value}")
        result.append(f"{utils_indent(indent)}{STEPS}:")
        for step in self.__steps:
            result.append(str(step))

        return "\n".join(result)

    @cached_property
    def name(self) -> str:
        return str(self.__name)

    @cached_property
    def steps(self) -> list[Step]:
        return self.__steps

    @cached_property
    def parameters(self) -> dict[str, Any]:  # type: ignore[explicit-any]
        return self.__parameters

    def get_command_line(self) -> str:
        options = ' '.join([
        ])
        options_str = re.sub(r'\s+', ' ', options, flags=re.IGNORECASE | re.DOTALL)

        return options_str

    def get_parameter(self, key: str, default: Any | None = None) -> Any | None:  # type: ignore[explicit-any]
        return self.__parameters.get(key, default)

    def check_binary_artifacts(self) -> None:
        for step in self.steps:
            if step.executable_path is not None and not step.executable_path.is_file():
                raise FileNotFoundException(
                    f"Specified binary at {step.executable_path} was not found")

    def check_types(self) -> None:
        types: dict[StepKind, int] = {}
        for step in self.steps:
            types[step.step_kind] = types.get(step.step_kind, 0) + 1

    def pretty_str(self) -> str:
        result: list[str] = [step.pretty_str() for step in self.steps if str(step.executable_path) and step.enabled]
        return '\n'.join(result)

    def __expand_macro_for_param(self, value_in_workflow: str | list, param: str) -> str | list:
        if (isinstance(value_in_workflow, str) and utils.has_macro(value_in_workflow) and
                not self.__test_suite.is_defined_in_collections(param)):
            return self.__expand_macro_for_str(value_in_workflow)
        if isinstance(value_in_workflow, list):
            return self.__expand_macro_for_list(value_in_workflow)
        return value_in_workflow

    def __prepare_imported_configs(self, parent_step_name: str, imported_configs: dict[str, dict[str, str]]) -> None:
        from_name = cast(str, imported_configs.get(FROM))
        config_name_path = utils.get_config_workflow_folder() / f"{from_name}.yaml"
        args = {}
        for param, param_value in imported_configs.items():
            if param == FROM:
                continue
            args.update(self.__prepare_imported_config(param, param_value))
        self.__load_imported_config(parent_step_name, config_name_path, args)

    def __prepare_imported_config(self, param: str, param_value: Any) -> dict[str, Any]:  # type: ignore[explicit-any]
        args = {}
        if isinstance(param_value, str):
            args[param] = Macros.correct_macro(param_value, self)
        elif isinstance(param_value, list):
            args[param] = self.__prepare_list(param_value)
        else:
            args[param] = param_value
        return args

    def __prepare_list(self, param_value: list[str]) -> list[str]:
        result_list = []
        for item in param_value:
            corrected_item = Macros.correct_macro(item, self)
            for sub_item in (corrected_item if isinstance(corrected_item, list) else [corrected_item]):
                if sub_item and sub_item in self.__parameters and self.__parameters[sub_item]:
                    result_list.append(self.__parameters[sub_item])
                elif corrected_item:
                    result_list.append(sub_item)
        return result_list

    def __load_imported_config(self, parent_step_name: str, cfg_path: Path,  # type: ignore[explicit-any]
                               actual_params: dict[str, Any]) -> None:
        _LOGGER.all(f"Going to load config from '{cfg_path}' with actual parameters {actual_params}")
        cfg_content = utils.load_config(cfg_path)
        params = cast(dict, cfg_content.get(PARAMETERS, {}))
        for param, _ in params.items():
            if param in actual_params:
                params[param] = actual_params[param]
        parent_step_name = f"{self.parent_step_name}.{parent_step_name}" if self.parent_step_name else parent_step_name
        workflow_options = WorkflowOptions(
            cfg_content=cfg_content,
            parent_test_suite=self.__test_suite,
            parent_workflow=self,
            parent_step_name=parent_step_name)
        for step in workflow_options.steps:
            names = [st.name for st in self.__steps]
            if step.name not in [names]:
                self.__steps.append(step)
        for param_name, param_value in workflow_options.parameters.items():
            if param_name not in self.__parameters:
                self.__parameters[param_name] = param_value

    def __load_steps(self, steps: dict[str, dict]) -> None:
        for step_name, step_content in steps.items():
            if FROM in step_content:
                self.__prepare_imported_configs(step_name, step_content)
            else:
                self.__load_step(step_name, step_content)

    def __load_step(self, step_name: str, step_content: dict[str, str | list | dict[str, list]]) -> None:
        _LOGGER.all(f"Going to load step '{step_name}'")
        for (step_item, step_value) in step_content.items():
            if isinstance(step_value, str):
                step_content[step_item] = Macros.correct_macro(step_value, self)
        new_args = []
        for arg in step_content['args']:
            arg = Macros.correct_macro(arg, self) \
                if not self.__test_suite.is_defined_in_collections(arg) else arg
            if isinstance(arg, list):
                new_args.extend(arg)
            else:
                new_args.append(arg)
        step_content['args'] = new_args
        new_env: dict[str, list] = {}
        if 'env' in step_content:
            new_env_var = []
            for env, val in cast(dict, step_content['env']).items():
                for env_line in val:
                    new_env_var.append(Macros.correct_macro(env_line, self))
                new_env[env] = new_env_var
            step_content['env'] = new_env
        step = Step(step_name, step_content)
        if step.enabled:
            if not step.executable_path_exists() and step.step_kind != StepKind.GTEST_RUNNER:
                raise FileNotFoundException(f"Step executable path is empty or incorrect: {step.executable_path}"
                                            f"\nCheck step '{step_name}' in the '{self.name}' workflow")

            parent_step_name = f"{self.parent_step_name}." if self.parent_step_name else ""
            step.name = f"{parent_step_name}{step.name}"
            _LOGGER.all(f"Step '{step.name}' is loaded and added")
            self.__steps.append(step)
        else:
            _LOGGER.all(f"Step '{step_name}' is disabled and won't be loaded")

    def __expand_macro_for_str(self, value_in_workflow: str) -> str | list[str]:
        try:
            return Macros.correct_macro(value_in_workflow, self)
        except ParameterNotFound as pnf:
            _LOGGER.all(str(pnf))
        except MacroNotExpanded as pnf:
            _LOGGER.all(str(pnf))
        return value_in_workflow

    def __expand_macro_for_list(self, value_in_workflow: list) -> str | list[str]:
        expanded_in_workflow: list[str] = []
        for value in value_in_workflow:
            try:
                expanded_value = Macros.correct_macro(value, self)
            except (ParameterNotFound, MacroNotExpanded) as pnf:
                _LOGGER.all(str(pnf))
                return value_in_workflow
            if isinstance(expanded_value, list):
                expanded_in_workflow.extend(expanded_value)
            else:
                expanded_in_workflow.append(expanded_value)
        return expanded_in_workflow
