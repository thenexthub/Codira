#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

import logging
import os
from unittest import TestCase

from pathlib import Path
from typing import Tuple, Dict, List, Any

from jinja2 import Environment, FileSystemLoader, select_autoescape, TemplateSyntaxError

from runner.plugins.ets.utils.metainformation import InvalidMetaException, find_all_metas
from runner.plugins.ets.utils.test_parameters import load_params
from runner.utils import iter_files, write_2_file

from runner.plugins.ets.utils.exceptions import InvalidFileFormatException, UnknownTemplateException

from runner.plugins.ets.utils.constants import \
    SKIP_PREFIX, \
    VARIABLE_START_STRING, \
    TEMPLATE_EXTENSION, OUT_EXTENSION


_LOGGER = logging.getLogger("runner.plugins.ets.stdlib_templates.stdlib_templates_generator")


class StdlibTemplatesGenerator:
    def __init__(self, template_root_path: Path) -> None:
        self.__jinja_env = Environment(
            loader=FileSystemLoader(str(template_root_path)),
            autoescape=select_autoescape(),
            variable_start_string=VARIABLE_START_STRING
        )

    @staticmethod
    def __deduplicate_key_content(key: str, key_uniq_suffix: int) -> Tuple[str, int]:
        key_updated = str(key) + "_" + str(key_uniq_suffix).rjust(3, '0')
        key_uniq_suffix += 1
        return key_updated, key_uniq_suffix

    def render_and_write_templates(self, root_path: Path, dirpath: Path, outpath: Path) -> List[str]:
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
            TestCase().assertTrue(len(tests) > 0, "Internal error: there should be tests")
            for key, test in tests.items():
                output_filepath = outpath / f"{name_without_ext}_{key}{OUT_EXTENSION}"
                write_2_file(file_path=output_filepath, content=test)
                generated_test_list.append(str(output_filepath))
        return generated_test_list

    def __render_template(self, filepath: str, params: Any = None) -> str:
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
            _LOGGER.critical(message)
            raise InvalidFileFormatException(message=message, filepath=filepath) from inv_format_exp
        except Exception as common_exp:
            message = f"UnknownTemplateException by filepath {filepath}.\n{common_exp}"
            _LOGGER.critical(message)
            raise UnknownTemplateException(filepath=filepath, exception=common_exp) from common_exp

    def __split_into_tests(self, text: str, filepath: str) -> Dict[str, str]:
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

        i = size_indices - 1
        desc = meta_info[i][2]
        key = desc["desc"]["function"]
        if key in result:
            key, key_uniq = self.__deduplicate_key_content(key, key_uniq)
        result[key] = text[start_indices[-1]:]

        return result
