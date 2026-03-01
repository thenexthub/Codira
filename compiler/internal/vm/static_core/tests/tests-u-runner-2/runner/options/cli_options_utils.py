#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

from collections.abc import Generator
from pathlib import Path
from typing import TYPE_CHECKING, Any, NewType, cast

from runner import utils
from runner.enum_types.base_enum import BaseEnum

if TYPE_CHECKING:
    from runner.options.cli_options import ConfigsLoader

ConfigPropertyType = Any  # type: ignore[explicit-any]
WorkflowName = NewType('WorkflowName', str)
TestSuiteName = NewType('TestSuiteName', str)


class CliOptionsConsts(BaseEnum):
    CFG_TYPE = "type"
    CFG_PARAMETERS = "parameters"
    CFG_RUNNER = "runner"
    VERBOSE = "verbose"
    VERBOSE_FILTER = "verbose-filter"
    QEMU = "qemu"


def get_config_list(cfg_path: Path) -> list[str]:
    files: Generator[Path, None, None] = cfg_path.rglob("*.yaml")
    return sorted(f.relative_to(cfg_path).as_posix().replace(".yaml", "") for f in files)


def restore_default_list(full_options: dict[str, ConfigPropertyType],
                         default_options: dict[str, list]) -> dict[str, ConfigPropertyType]:
    for key, val in full_options.items():
        if val is None:
            full_options[key] = default_options.get(key, None)

    return full_options


def restore_duplicated_options(configs_loader: "ConfigsLoader",
                               full_options: dict) -> dict[str, ConfigPropertyType]:
    workflow_data = utils.load_config(configs_loader.get_workflow_path())
    workflow_params = cast(dict, workflow_data.get(CliOptionsConsts.CFG_PARAMETERS.value, {}))

    test_suite_data = utils.load_config(configs_loader.get_test_suite_path())
    test_suite_params = cast(dict, test_suite_data.get(CliOptionsConsts.CFG_PARAMETERS.value, {}))

    duplicated_params = workflow_params.keys() & test_suite_params.keys()

    for param in duplicated_params:
        workflow_param_name = f"{configs_loader.workflow_name}.{CliOptionsConsts.CFG_PARAMETERS.value}.{param}"
        test_suite_param_name = f"{configs_loader.test_suite_name}.{CliOptionsConsts.CFG_PARAMETERS.value}.{param}"
        if workflow_param_name not in full_options:
            full_options[workflow_param_name] = (
                workflow_params)[param]

        if test_suite_param_name not in full_options:
            full_options[test_suite_param_name] = (
                test_suite_params)[param]

    return full_options


def add_config_info(configs_loader: "ConfigsLoader",
                    full_options: dict[str, ConfigPropertyType]) -> dict[str, ConfigPropertyType]:
    full_options["workflow"] = configs_loader.workflow_name
    full_options[f"{configs_loader.workflow_name}.path"] = configs_loader.get_workflow_path()
    full_options[f"{configs_loader.workflow_name}.data"] = utils.load_config(str(configs_loader.get_workflow_path()))

    full_options["test-suite"] = configs_loader.test_suite_name
    full_options[f"{configs_loader.test_suite_name}.path"] = configs_loader.get_test_suite_path()
    full_options[f"{configs_loader.test_suite_name}.data"] = (
        utils.load_config(str(configs_loader.get_test_suite_path())))

    return full_options
