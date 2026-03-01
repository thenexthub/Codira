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

import glob
import logging
import os
import pathlib
import typing
import jinja2
import yaml
import runner.logger as LOG
import runner.plugins.ets.utils.constants
import runner.utils


class EtsFuncTestsCodeGenerator:
    _TEST_SEPARATOR = "---"
    _TEMPLATE_PATTERN = '/*.ets.j2'


    def __init__(self, template_root_path: pathlib.Path):
        self._logger = logging.getLogger("runner.generators.ets_func_tests")
        self._jinja_env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(str(template_root_path)),
            autoescape=jinja2.select_autoescape(),
            variable_start_string=runner.plugins.ets.utils.constants.VARIABLE_START_STRING
        )

    @staticmethod
    def __get_true_file_basename(path:str) -> str:
        base_fname = os.path.basename(path)
        while '.' in base_fname:
            base_fname, _ = os.path.splitext(base_fname)
        return base_fname
    
    @staticmethod
    def __save_artefact_of_generator(test: str, template_fname: str, template_config_fname: str) -> str:
        test = f"/* Template name : {template_fname}*/\n" + test
        test = f"/* Config name : {template_config_fname}*/\n\n" + test
        return test

    def render_and_write_templates(self, 
                                   template_root_path: pathlib.Path, 
                                   current_dir: pathlib.Path, 
                                   outpath: pathlib.Path) -> typing.List[str]:
        os.makedirs(outpath, exist_ok=True)
        generated_test_list = []
        templates_fname_list = self.__find_all_templates(current_dir)
        for template_fname in templates_fname_list:
            generated_test_list.extend(self.__render_template(template_fname, template_root_path, outpath))
        return generated_test_list

    def __find_all_templates(self, current_dir: pathlib.Path) -> typing.List[str]:
        LOG.Log.summary(self._logger, f"Start searching in : {current_dir}")
        LOG.Log.summary(self._logger, f"{str(current_dir) + self._TEMPLATE_PATTERN}")
        templates_fname_list = glob.glob(str(current_dir) + self._TEMPLATE_PATTERN, recursive=False)
        LOG.Log.summary(self._logger, f"Found {len(templates_fname_list)} templates")
        return templates_fname_list

    def __build_template_config_file_name(self, template_path_fname: str) -> str:
        path_parser = pathlib.Path(template_path_fname)
        template_base_fname = self.__get_true_file_basename(template_path_fname)
        template_path = path_parser.parent
        config_fname = "config-" + template_base_fname + ".yaml"
        LOG.Log.summary(self._logger, f"Config path {config_fname}")
        result = os.path.join(template_path, config_fname)
        return result

    def __render_template(self, 
                          template_fname: str, 
                          template_root_path: pathlib.Path, 
                          outpath: pathlib.Path) -> typing.List[str]:
        generated_tests_list = []
        template_config_name = self.__build_template_config_file_name(template_fname)
        path_parser = pathlib.Path(template_fname)
        template_relative_path = os.path.relpath(path_parser.parent, template_root_path)
        template_relative_fname = os.path.join(template_relative_path, path_parser.name)
        LOG.Log.summary(self._logger, f"Load template from {template_relative_fname}")
        template = self._jinja_env.get_template(template_relative_fname)
        params = self.__load_parameters(template_config_name)
        content = template.render(**params)
        content = content.strip()
        content_list = self.__split_tests(content)
        test_idx = 1
        path_parser = pathlib.Path(template_fname)
        fname = self.__get_true_file_basename(template_fname)

        for test in content_list:
            test = self.__save_artefact_of_generator(test, template_fname, template_config_name)
            output_fname = "test-" + fname + "-" + str(test_idx).zfill(4) + ".ets"
            outpath_final = outpath / output_fname
            generated_tests_list.append(str(outpath))
            runner.utils.write_2_file(outpath_final, test)
            test_idx += 1
        return generated_tests_list

    def __load_parameters(self, config_file_path : str) -> typing.Any:
        with os.fdopen(os.open(config_file_path, os.O_RDONLY, 0o755), "r", encoding='utf8') as f_handle:
            text = f_handle.read()
        try:
            params = yaml.safe_load(text)
            LOG.Log.summary(self._logger, params)
        except Exception as common_exp:
            raise Exception(f"Could not load YAML: {str(common_exp)} filepath={config_file_path}") from common_exp
        return params

    def __split_tests(self, content : str) -> typing.List[str]:
        content = content.rstrip(self._TEST_SEPARATOR)
        content_list = content.split("---")
        LOG.Log.summary(self._logger, f"size : {len(content_list)}")
        return content_list
    