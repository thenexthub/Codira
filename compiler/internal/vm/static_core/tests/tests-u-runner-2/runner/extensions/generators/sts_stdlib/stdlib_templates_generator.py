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
#

import os
from pathlib import Path

from jinja2 import Environment, FileSystemLoader, TemplateSyntaxError, select_autoescape

from runner import common_exceptions
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.sts_utils.constants import OUT_EXTENSION, SKIP_PREFIX, TEMPLATE_EXTENSION, VARIABLE_START_STRING
from runner.sts_utils.exceptions import InvalidFileFormatException, UnknownTemplateException
from runner.sts_utils.file_structure import walk_test_subdirs
from runner.sts_utils.metainformation import InvalidMetaException, find_all_metas
from runner.sts_utils.test_parameters import load_params
from runner.utils import iter_files, write_2_file

_LOGGER = Log.get_logger(__file__)


class StdlibTemplatesGenerator(IGenerator):
    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self.__jinja_env = Environment(
            loader=FileSystemLoader(str(source)),
            autoescape=select_autoescape(),
            variable_start_string=VARIABLE_START_STRING
        )

    @staticmethod
    def __deduplicate_key_content(key: str, key_uniq_suffix: int) -> tuple[str, int]:
        key_updated = str(key) + "_" + str(key_uniq_suffix).rjust(3, '0')
        key_uniq_suffix += 1
        return key_updated, key_uniq_suffix

    def generate(self) -> list[str]:
        """
        Renders all templates and saves them.
        """
        generated_tests = []
        for dir_name in walk_test_subdirs(self._source):
            dir_outpath = self._target / dir_name.path.relative_to(self._source)
            generated_tests_tmp = self.render_and_write_templates(self._source, dir_name.path, dir_outpath)
            generated_tests.extend(generated_tests_tmp)
        return generated_tests

    def render_and_write_templates(self, root_path: Path, dirpath: Path, outpath: Path) -> list[str]:
        """
        Recursively walk the FS, save rendered templates
        Loads parameters and renders all templates in `dirpath`.
        Saves the results in `outpath`
        """
        generated_test_list = []
        os.makedirs(outpath, exist_ok=True)
        params = load_params(dirpath)
        for name, path, in iter_files(dirpath, allowed_ext=[TEMPLATE_EXTENSION]):
            name_without_ext, _ = os.path.splitext(name)
            if name_without_ext.startswith(SKIP_PREFIX):
                continue
            template_relative_path = os.path.relpath(path, root_path)
            rendered_template = self.__render_template(template_relative_path, params)
            tests = self.__split_into_tests(rendered_template, path)
            if len(tests) <= 0:
                raise common_exceptions.UnknownException("Internal error: there should be tests")
            for key, test in tests.items():
                output_filepath = outpath / f"{name_without_ext}_{key}{OUT_EXTENSION}"
                write_2_file(file_path=output_filepath, content=test, disallow_overwrite=True)
                generated_test_list.append(str(output_filepath))
        return generated_test_list

    def __render_template(self, filepath: str, params: dict[str, list] | None = None) -> str:
        """
        Renders a single template and returns result as string
        """
        if params is None:
            params = {}
        try:
            template = self.__jinja_env.get_template(filepath.replace(os.path.sep, '/'))
            return template.render(**params)
        except TemplateSyntaxError as inv_format_exp:
            message = f"Template Syntax Error: ${inv_format_exp.message}"
            _LOGGER.default(message)
            raise InvalidFileFormatException(message=message, filepath=filepath) from inv_format_exp
        except Exception as common_exp:
            message = f"UnknownTemplateException by filepath {filepath}.\n{common_exp}"
            _LOGGER.default(message)
            raise UnknownTemplateException(filepath=filepath, exception=common_exp) from common_exp

    def __split_into_tests(self, text: str, filepath: str) -> dict[str, str]:
        """
        Splits rendered template into multiple tests.
        'filepath' is needed for exceptions
        """
        result = {}
        key_uniq = 1

        try:
            meta_info = find_all_metas(text)
            start_indices = [metainfile[0] for metainfile in meta_info]
        except InvalidMetaException as inv_m_exp:
            raise InvalidFileFormatException(
                filepath=filepath,
                message=f"Splitting tests fails: {inv_m_exp.message}"
            ) from inv_m_exp
        size_indices = len(start_indices)
        for i in range(1, size_indices):
            left = start_indices[i - 1]
            right = start_indices[i]
            desc = meta_info[i - 1][2]
            key = desc["desc"]["function"]
            if key in result:
                key, key_uniq = self.__deduplicate_key_content(key, key_uniq)
            result[key] = text[left:right]

        if size_indices == 1:
            i = 0
        desc = meta_info[i][2]
        key = desc["desc"]["function"]
        if key in result:
            key, key_uniq = self.__deduplicate_key_content(key, key_uniq)
        result[key] = text[start_indices[-1]:]

        return result
