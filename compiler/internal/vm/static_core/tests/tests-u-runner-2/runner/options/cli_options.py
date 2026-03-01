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

import argparse
from argparse import ArgumentParser, Namespace
from glob import glob
from pathlib import Path
from typing import Any, cast

from runner import utils
from runner.common_exceptions import FileNotFoundException, IncorrectEnumValue, InvalidConfiguration
from runner.enum_types.config_type import ConfigType
from runner.logger import Log
from runner.options import cli_options_utils as cli_utils
from runner.options.cli_options_utils import CliOptionsConsts
from runner.options.options_general import GeneralOptions
from runner.options.options_test_suite import TestSuiteOptions

_LOGGER = Log.get_logger(__file__)

CliOptionType = str | int | float | bool | list
OptionType = CliOptionType | dict | Path | None
ArgListType = list[str]


class CliOptions:
    __CFG_TYPE = "type"
    __CFG_PARAMETERS = "parameters"
    __CFG_RUNNER = "runner"

    def __init__(self, argv: ArgListType):
        self.data: dict[str, OptionType] = {}
        if not argv or len(argv) < 2:
            raise InvalidConfiguration("No parameters specified. Expected at least workflow and test-suite")
        self.argv: ArgListType = argv
        self.__extract_configs()
        self.__process_runner_options(self.argv)

    @staticmethod
    def __parse_option(arg_list: ArgListType, option_name: str, default_value: CliOptionType | None) \
            -> tuple[CliOptionType | None, ArgListType]:
        """
        Search an option with `option_name` in `arg_list`.
        :param arg_list: list of strings incoming from argv (CLI)
        :param option_name: the name of the searched option
        :param default_value: the default value
        :return: If the option is found, its value is returned.
        The type of the returned value is the type of `default_value`.
        In other case the `default_value` is returned.
        """
        option_values: list[CliOptionType | None] = []
        while CliOptions.__has_option(option_name, arg_list):
            option_value, arg_list = CliOptions.__parse_option_with_space(arg_list, option_name)
            if option_value is None:
                option_value, arg_list = CliOptions.__parse_option_with_equal(arg_list, option_name)
            option_values.append(option_value)
        if len(option_values) == 0 or (len(option_values) == 1 and option_values[0] is None):
            return default_value, arg_list
        if len(option_values) > 1 and not isinstance(default_value, list):
            return option_values[-1], arg_list
        if isinstance(default_value, list) and default_value:
            option_values.extend(default_value)
        return option_values[0] if len(option_values) == 1 else option_values, arg_list

    @staticmethod
    def __is_option_value_int(value: CliOptionType | None) -> tuple[bool, int | None]:
        """
        Detects whether the value is int,
        if not tries to convert to int
        :param value: value get from CLI
        :return: the first item: True if returned value is int,
        the second item: the returned value if it's int or None
        """
        if isinstance(value, int):
            return True, int(value)
        if isinstance(value, str):
            try:
                return True, int(value)
            except (TypeError, ValueError):
                return False, None
        return False, None

    @staticmethod
    def __is_option_value_str(value: CliOptionType | None) -> tuple[bool, str | None]:
        """
        Detects whether the value is str,
        :param value: value get from CLI
        :return: the first item: True if returned value is str,
        the second item: the returned value if it's str or None
        """
        if isinstance(value, str):
            return True, str(value)
        return False, None

    @staticmethod
    def __is_option_value_bool(value: CliOptionType | None) -> tuple[bool, bool | None]:
        """
        Detects whether the value is bool,
        if not tries to convert to bool
        :param value: value get from CLI
        :return: the first item: True if returned value is bool,
        the second item: the returned value if it's bool or None
        """
        if isinstance(value, bool):
            return True, bool(value)
        if isinstance(value, str):
            if value.lower() == "true":
                return True, True
            if value.lower() == "false":
                return True, False
        return False, None

    @staticmethod
    def __get_option_value(option_name: str, value: CliOptionType | None) -> CliOptionType:
        """
        Converts the value from any/str to specific type
        """
        result_tuple = CliOptions.__is_option_value_int(value)
        result: bool = result_tuple[0]
        res_value: CliOptionType | None = result_tuple[1]
        if result and res_value is not None:
            return res_value
        result, res_value = CliOptions.__is_option_value_bool(value)
        if result and res_value is not None:
            return res_value
        result, res_value = CliOptions.__is_option_value_str(value)
        if result and res_value is not None:
            return res_value
        message = f"Invalid '{res_value}' value for the option '{option_name}'."
        raise InvalidConfiguration(message)

    @staticmethod
    def __has_option(option_name: str, arg_list: ArgListType) -> bool:
        for arg in arg_list:
            if arg == option_name or arg.startswith(f"{option_name}="):
                return True
        return False

    @staticmethod
    def __parse_option_with_space(arg_list: ArgListType, option_name: str) \
            -> tuple[CliOptionType | None, ArgListType]:
        """
        Search the option `option_name` in the list of arguments and
        tries to get its value supposing it's divided with a space
        :param arg_list: list of arguments
        :param option_name: the searched option name
        :return: option value if found, in other case None as the first item,
        and the remaining list of arguments as the second item.
        """
        if option_name in arg_list:
            option_index = arg_list.index(option_name)
            if len(arg_list) > option_index + 1:
                option_value = CliOptions.__get_option_value(option_name, arg_list[option_index + 1])
                arg_list = arg_list[:option_index] + arg_list[option_index + 2:]
                return option_value, arg_list
            message = f"Missing {option_name} value."
            raise InvalidConfiguration(message)

        return None, arg_list

    @staticmethod
    def __parse_option_with_equal(arg_list: ArgListType, option_name: str) \
            -> tuple[CliOptionType | None, ArgListType]:
        """
        Search the option `option_name` in the list of arguments and
        tries to get its value supposing it's divided with an equal sign `=`
        :param arg_list: list of arguments
        :param option_name: the searched option name
        :return: option value if found, in other case None as the first item,
        and the remaining list of arguments as the second item.
        """
        for index, arg in enumerate(arg_list):
            local_option_name = f"{option_name}="
            if arg.startswith(local_option_name):
                option_value = arg[len(local_option_name):]
                if option_value:
                    converted_option_value = CliOptions.__get_option_value(option_name, option_value)
                    arg_list = arg_list[:index] + arg_list[index + 1:]
                    return converted_option_value, arg_list
                message = f"Missing {option_name} value."
                raise InvalidConfiguration(message)
        return None, arg_list

    @staticmethod
    def __runner_options(runner_cli: ArgListType) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description="General options parser")

        GeneralOptions.add_cli_args(parser)
        try:
            runner_args = parser.parse_args(runner_cli)
        # NOTE(pronai): is it necessary to capture *all* exceptions?
        except BaseException as exc:
            raise IncorrectEnumValue(str(exc.args)) from exc
        return runner_args

    @staticmethod
    def __test_suite_options(options: ArgListType) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description="Test suite options parser")

        TestSuiteOptions.add_cli_args(parser)

        test_suite_args = parser.parse_args(options)
        return test_suite_args

    def __process_workflow(self, cfg_name: str) -> None:
        self.__process_command(utils.get_config_workflow_folder(), cfg_name)

    def __process_test_suite(self, cfg_name: str) -> None:
        self.__process_command(utils.get_config_test_suite_folder(), cfg_name)

    def __process_command(self, folder: Path, cfg_name: str) -> None:
        cfg_path = folder.joinpath(cfg_name + '.yaml')
        try:
            data = utils.load_config(str(cfg_path))
        except FileNotFoundError as exc:
            existing_files = [Path(cfg).stem for cfg in glob(str(folder.joinpath("**/*.yaml")), recursive=True)]
            message = (f"Configuration file '{cfg_name}' not found. "
                       f"Expected one of: {sorted(existing_files)}")
            raise FileNotFoundException(message) from exc
        if (cfg_type := data.get(self.__CFG_TYPE)) in self.data:
            raise InvalidConfiguration(f"{cfg_name} redefines {cfg_type} config.")
        cli_params = cast(dict, data.get(self.__CFG_PARAMETERS, {}))

        self.data[str(cfg_type)] = cfg_name
        self.data[f"{cfg_name}.path"] = cfg_path
        self.data[f"{cfg_name}.data"] = data

        if cfg_type == ConfigType.TEST_SUITE.value:
            self.__init_test_suite_defaults(cfg_name)

        for key, default_value in cli_params.items():
            value, self.argv = self.__parse_option(self.argv, f"--{key}", default_value)
            self.data[f"{cfg_name}.{self.__CFG_PARAMETERS}.{key}"] = value

        if cfg_type == ConfigType.TEST_SUITE.value:
            self.__process_options_from_expected(cfg_name)

    def __process_options_from_expected(self, cfg_name: str) -> None:
        key = ""
        is_key = False
        n_processed = 0
        for index, arg in enumerate(self.argv[:]):
            if arg.startswith("--") and is_key:
                self.data[key] = True
                is_key = False
            if arg.startswith("--") and not is_key:
                key = f"{cfg_name}.{self.__CFG_PARAMETERS}.{utils.convert_underscore(arg[2:])}"
                if key not in self.data:
                    continue
                is_key = True
                self.argv.pop(index - n_processed)
                n_processed += 1
            elif not arg.startswith("--") and is_key:
                self.data[key] = arg
                is_key = False
                self.argv.pop(index - n_processed)
                n_processed += 1
            else:
                continue
        if is_key:
            self.data[key] = True

    def __init_test_suite_defaults(self, test_suite_name: str) -> None:
        test_suite_args = self.__test_suite_options([])
        for option in test_suite_args.__dict__:
            option_with_minus = utils.convert_underscore(option)
            key_with_minus = f"{test_suite_name}.{self.__CFG_PARAMETERS}.{option_with_minus}"
            self.data[key_with_minus] = test_suite_args.__dict__[option]

    def __process_runner_options(self, not_processed_args: ArgListType) -> None:
        runner_options = self.__runner_options(not_processed_args)
        for option in runner_options.__dict__:
            self.data[f"{self.__CFG_RUNNER}.{option}"] = runner_options.__dict__[option]

    def __extract_configs(self) -> None:
        workflow_cli = self.argv.pop(0)
        self.__process_workflow(workflow_cli)
        test_suite_cli = self.argv.pop(0)
        self.__process_test_suite(test_suite_cli)


class ConfigsLoader:

    def __init__(self, workflow_name: str, test_suite_name: str):
        self.workflow_name = workflow_name
        self.test_suite_name = test_suite_name

    def get_workflow_path(self) -> Path:
        return utils.get_config_workflow_folder().joinpath(self.workflow_name + '.yaml')

    def get_test_suite_path(self) -> Path:
        return utils.get_config_test_suite_folder().joinpath(self.test_suite_name + '.yaml')

    def load_workflow_config(self) -> dict[str, cli_utils.ConfigPropertyType]:
        return utils.load_config(self.get_workflow_path())

    def load_test_suite_config(self) -> dict[str, cli_utils.ConfigPropertyType]:
        return utils.load_config(self.get_test_suite_path())


class CliParserBuilder:

    def __init__(self, configs_loader: ConfigsLoader):
        self.configs = configs_loader

    @staticmethod
    def runner_options(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
        GeneralOptions.add_cli_args(parser, f"{CliOptionsConsts.CFG_RUNNER.value}")
        return parser

    @staticmethod
    def create_parser_for_runner() -> argparse.ArgumentParser:
        return CliParserBuilder.runner_options(argparse.ArgumentParser(
                add_help=False, description="Config 'URunner' options",
                conflict_handler="resolve"))

    @staticmethod
    def default_test_suite_options(parser: argparse.ArgumentParser, test_suite_name: str) -> argparse.ArgumentParser:
        TestSuiteOptions.add_cli_args(parser, f"{test_suite_name}.{CliOptionsConsts.CFG_PARAMETERS.value}")
        return parser

    @staticmethod
    def gather_config_options(cfg_name: str, cfg_data: dict,
                              parser: argparse.ArgumentParser) -> tuple[argparse.ArgumentParser, dict]:

        if CliOptionsConsts.CFG_TYPE.value not in cfg_data:
            raise InvalidConfiguration(f"Cannot detect type of config {cfg_name}. "
                                       f"Have you specified key {CliOptionsConsts.CFG_TYPE.value}?")
        cli_params = cfg_data.get(CliOptionsConsts.CFG_PARAMETERS.value, {})

        config = parser.add_argument_group(title=f"Config '{cfg_name}' options")

        keys_with_default_lists = {}
        for key, default_value in cli_params.items():
            if isinstance(default_value, list):
                keys_with_default_lists[f'{cfg_name}.{CliOptionsConsts.CFG_PARAMETERS.value}.{key}'] = default_value
            try:
                config.add_argument(
                    f"--{key}", action='append' if isinstance(default_value, list) else 'store',
                    default=None if isinstance(default_value, list) else default_value,
                    dest=f'{cfg_name}.{CliOptionsConsts.CFG_PARAMETERS.value}.{key}',
                    help=f"Config '{cfg_name}', parameter '{key}'"
                )
            except (argparse.ArgumentError, argparse.ArgumentTypeError) as exp:
                raise InvalidConfiguration(f"An exception occurred when adding parameter {key} "
                                           f"with default value = {default_value}") from exp
        return parser, keys_with_default_lists

    def create_parser_for_workflow(self) -> tuple[argparse.ArgumentParser, dict[str, list]]:
        parser_for_config, keys_from_wf = self.create_parser_for_config(
            config_name=self.configs.workflow_name,
            config_data=self.configs.load_workflow_config()
        )
        return parser_for_config, keys_from_wf

    def create_parser_for_test_suite(self) -> tuple[argparse.ArgumentParser, dict[str, list]]:
        parser_for_test_suite, keys_from_ts = self.create_parser_for_config(
            config_name=self.configs.test_suite_name,
            config_data=self.configs.load_test_suite_config())
        return parser_for_test_suite, keys_from_ts

    def create_parser_for_default_test_suite(self) -> argparse.ArgumentParser:
        return CliParserBuilder.default_test_suite_options(argparse.ArgumentParser(
            add_help=False, description="Config 'URunner' options",
            conflict_handler="resolve"), self.configs.test_suite_name)

    def create_parser_for_config(self, config_name: str,
                                 config_data: dict) -> tuple[argparse.ArgumentParser, dict]:
        wf_config = argparse.ArgumentParser(
            add_help=False, description=f"Config '{config_name}' options",
            conflict_handler="resolve",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)
        parser_options, keys_list = self.gather_config_options(cfg_name=config_name, cfg_data=config_data,
                                                               parser=wf_config)
        return parser_options, keys_list


class HelpAction(argparse.Action):

    def __call__(self, parser: ArgumentParser, namespace: Namespace, values: Any,    # type: ignore[explicit-any]
                 option_string: str | None = None) -> None:
        parser.print_help()
        namespace.remaining = [option_string]


class CliOptionsParser:

    def __init__(self, configs_loader: ConfigsLoader, runner_parser: argparse.ArgumentParser,
                 test_suite_parser: argparse.ArgumentParser,
                 default_test_suite_parser: argparse.ArgumentParser,
                 workflow_parser: argparse.ArgumentParser, *argv: str):
        self.configs = configs_loader

        self.runner_parser = runner_parser
        self.test_suite_parser = test_suite_parser
        self.default_test_suite_parser = default_test_suite_parser
        self.workflow_parser = workflow_parser
        self.argv = argv
        self.full_options: dict[str, OptionType] = {}

    def parse_args(self) -> None:

        usage = f"\n%(prog)s {self.configs.workflow_name} {self.configs.test_suite_name} [options]\n\n"
        parser = argparse.ArgumentParser(
            usage=usage,
            description="Universal test runner",
            parents=[self.runner_parser, self.test_suite_parser, self.default_test_suite_parser,
                     self.workflow_parser],
            conflict_handler="resolve",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)
        try:
            self.full_options = vars(parser.parse_args(self.argv))
        except (argparse.ArgumentError, argparse.ArgumentTypeError):
            parser.print_help()


def get_args() -> dict[str, OptionType]:

    workflow_list = cli_utils.get_config_list(Path(utils.get_config_workflow_folder()))
    test_suite_list = cli_utils.get_config_list(Path(utils.get_config_test_suite_folder()))

    help_desc = ("\nTo explore possible workflow and test suite options:  \n"
                 " './runner.sh <WORKFLOW_NAME> <TEST_SUITE_NAME> --help'\n"
                 "\nExample: \n"
                 " ./runner.sh panda-int ets-cts --help\n")

    parser = argparse.ArgumentParser(description="URunner argument parser\n"
                                                 f"{help_desc}",
                                     add_help=False,
                                     formatter_class=argparse.RawTextHelpFormatter,
                                     usage=argparse.SUPPRESS)

    parser.add_argument("workflow_name", metavar="<WORKFLOW_NAME>",
                         choices=workflow_list,
                        help="{%(choices)s}")
    parser.add_argument("test_suite_name", metavar="<TEST_SUITE_NAME>",
                        help="{%(choices)s}",
                        choices=test_suite_list)
    parser.add_argument("-h", "--help", action=HelpAction, dest=argparse.SUPPRESS, nargs=0)

    args, remaining = parser.parse_known_args()
    remaining = getattr(args, "remaining", []) + remaining

    configs_loader = ConfigsLoader(args.workflow_name, args.test_suite_name)

    parser_builder = CliParserBuilder(configs_loader)
    test_suite_parser, key_lists_ts = parser_builder.create_parser_for_test_suite()
    workflow_parser, key_lists_wf = parser_builder.create_parser_for_workflow()

    cli = CliOptionsParser(configs_loader, parser_builder.create_parser_for_runner(),
                            test_suite_parser,
                            parser_builder.create_parser_for_default_test_suite(),
                            workflow_parser, *remaining)
    cli.parse_args()

    cli.full_options[f"{CliOptionsConsts.CFG_RUNNER.value}.cli_options"] = remaining

    parsed_options = cli.full_options
    parsed_options = cli_utils.restore_duplicated_options(configs_loader, parsed_options)
    parsed_options = cli_utils.add_config_info(configs_loader, parsed_options)
    parsed_options = cli_utils.restore_default_list(parsed_options, key_lists_ts | key_lists_wf)

    return parsed_options
