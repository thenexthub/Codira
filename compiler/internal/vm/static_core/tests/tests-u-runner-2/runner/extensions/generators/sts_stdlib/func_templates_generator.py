#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
#

from collections.abc import Generator
from pathlib import Path
from typing import Any

import jinja2
import yaml

from runner.common_exceptions import YamlException
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.sts_utils.constants import VARIABLE_START_STRING
from runner.utils import write_2_file

TemplatePropertyType = Any  # type: ignore[explicit-any]
_LOGGER = Log.get_logger(__file__)


class FuncTestsCodeGenerator(IGenerator):
    _TEST_SEPARATOR = "---"
    _TEMPLATE_PATTERN = '*.ets.j2'

    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self._jinja_env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(str(source)),
            autoescape=jinja2.select_autoescape(),
            variable_start_string=VARIABLE_START_STRING
        )

    @staticmethod
    def _get_file_basename(path: Path) -> str:
        name = path.name
        for suffix in path.suffixes:
            name = name.replace(suffix, "")
        return name

    @staticmethod
    def _add_generator_artefact(test: str, template: Path, template_config: Path) -> str:
        test = f"/* Template name : {template}*/\n" + test
        test = f"/* Config name : {template_config}*/\n\n" + test
        return test

    @staticmethod
    def _load_parameters(config_file_path: Path) -> dict[str, TemplatePropertyType]:
        try:
            text = config_file_path.read_text(encoding='utf-8')
            params: dict[str, TemplatePropertyType] = yaml.safe_load(text)
            _LOGGER.all("; ".join(f"{key} = {value}" for key, value in params.items()))
        except yaml.YAMLError as common_exp:
            raise YamlException(f"Could not load YAML: {common_exp} filepath={config_file_path}") from common_exp
        return params

    def generate(self) -> list[str]:
        """
        Renders all templates and saves them.
        """
        self._target.mkdir(parents=True, exist_ok=True)
        generated_test_list: list[str] = []
        templates_fname_list: list[Path] = list(self._find_all_templates(self._source))
        for template_fname in templates_fname_list:
            generated_test_list += self._render_template(template_fname, self._source, self._target)
        return generated_test_list

    def _find_all_templates(self, current_dir: Path) -> Generator[Path, None, None]:
        _LOGGER.all(f"Start searching in : {current_dir}")
        _LOGGER.all(f"{str(current_dir) + self._TEMPLATE_PATTERN}")
        return current_dir.rglob(self._TEMPLATE_PATTERN)

    def _get_template_config(self, template_path: Path) -> Path:
        template_basename = self._get_file_basename(template_path)
        config_name = "config-" + template_basename + ".yaml"
        _LOGGER.all(f"Config path {config_name}")
        return template_path.with_name(config_name)

    def _render_template(self,
                         template_path: Path,
                         template_root_path: Path,
                         outpath: Path) -> Generator[str, None, None]:
        template_config_path = self._get_template_config(template_path)
        template_relative_path = template_path.relative_to(template_root_path)
        _LOGGER.all(f"Load template from {template_relative_path}")
        template = self._jinja_env.get_template(str(template_relative_path))
        params = self._load_parameters(template_config_path)
        content = template.render(**params).strip()
        content_list = self._split_tests(content)
        test_idx = 1
        template_name = self._get_file_basename(template_path)

        for test in content_list:
            test = self._add_generator_artefact(test, template_path, template_config_path)
            output_fname = "test-" + template_name + "-" + str(test_idx).zfill(4) + ".ets"
            outpath_final = outpath / template_relative_path.with_name(output_fname)
            write_2_file(outpath_final, test, disallow_overwrite=True)
            test_idx += 1
            yield outpath_final.as_posix()

    def _split_tests(self, content: str) -> list[str]:
        content = content.rstrip(self._TEST_SEPARATOR)
        content_list = content.split("---")
        _LOGGER.summary(f"size : {len(content_list)}")
        return content_list
