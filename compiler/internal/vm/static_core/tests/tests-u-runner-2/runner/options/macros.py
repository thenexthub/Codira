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

from os import environ

from runner.common_exceptions import MacroNotExpanded, ParameterNotFound
from runner.logger import Log
from runner.options.local_env import LocalEnv
from runner.options.options import IOptions
from runner.utils import expand_file_name, get_all_macros, has_macro, replace_macro

_LOGGER = Log.get_logger(__file__)


class Macros:
    __SPACE_SUBSTITUTION = "%20"

    @staticmethod
    def add_steps_2_macro(macro: str) -> str:
        steps = 'steps'
        if macro.startswith(f'{steps}.') and not macro.startswith(f'{steps}.{steps}.'):
            return f"{steps}.{macro}"
        return macro

    @staticmethod
    def remove_parameters_and_minuses(macro: str) -> str:
        parameters = "parameters."
        if macro.find(parameters) >= 0:
            macro = macro.replace(parameters, "")
        return macro.replace("-", "_")

    @classmethod
    def expand_macros_in_path(cls, value: str, config: IOptions) -> str | None:
        corrected = cls.correct_macro(value, config)
        return expand_file_name(corrected) if isinstance(corrected, str) else None

    @classmethod
    def correct_macro(cls, raw_value: str, config: IOptions) -> str | list[str]:
        """
        Macro can be expanded into single value of str or to list of str
        :param raw_value: the source line with one or several macros
        :param config: object with options where macros are searched
        :return: the line with expanded macros or list of lines

        If a macro is expanded into list of str the 2 cases are possible:
        - if a source line contains only 1 macro (expanded into list) and no other symbols:
            in this case the list of str is returned
        - if a source line contains several macros or 1 macro and other symbols:
            in this case the list is converted into the str
        """
        result_str = raw_value
        if not has_macro(result_str):
            return result_str
        result_str, not_found = cls.__process(result_str, raw_value, config)
        remained_macros = [item for item in get_all_macros(result_str) if item not in not_found]
        if len(remained_macros) > 0:
            processed = cls.correct_macro(result_str, config)
            if isinstance(processed, list):
                result_str = " ".join(processed)
            else:
                result_str = processed
        if result_str != raw_value:
            _LOGGER.all(f"Corrected macro: '{raw_value}' => '{result_str}'")
        result_list = [value.replace(cls.__SPACE_SUBSTITUTION, " ") for value in result_str.split()]
        return result_list[0] if len(result_list) == 1 else result_list

    @classmethod
    def __process(cls, result: str, raw_value: str, config: IOptions) -> tuple[str, list[str]]:
        not_found: list[str] = []
        prop_value: str | list[str] | None = None
        for macro in get_all_macros(result):
            prop_value = environ.get(macro)
            if prop_value is not None:
                local_env = LocalEnv.get()
                local_env.add(macro, str(prop_value))
            prop_value = config.get_value(macro) if prop_value is None else prop_value
            if prop_value is None:
                macro3 = cls.remove_parameters_and_minuses(macro)
                prop_value = config.get_value(macro3)
            macro2 = cls.add_steps_2_macro(macro)
            prop_value = config.get_value(macro2) if prop_value is None else prop_value
            prop_value = config.get_value(cls.remove_parameters_and_minuses(macro2)) \
                if prop_value is None else prop_value
            if prop_value is None:
                not_found.append(macro)
                if macro != 'test-id':
                    raise ParameterNotFound(
                        f"Cannot expand macro '{macro}' at value '{raw_value}'. "
                        "Check yaml path or provide the value as environment variable")
                continue
            if isinstance(prop_value, list):
                prop_value_list = [value.replace(" ", cls.__SPACE_SUBSTITUTION) for value in prop_value]
                prop_value = " ".join(prop_value_list)
            if isinstance(prop_value, str) and prop_value.find(f"${{{macro}}}") != -1:
                raise MacroNotExpanded(
                    f"The macro '{macro}' at value '{raw_value}' is expanded into '{prop_value}'. "
                    "Check yaml path or provide the value as environment variable")
            result = replace_macro(str(result), macro, str(prop_value))
        return result, not_found
