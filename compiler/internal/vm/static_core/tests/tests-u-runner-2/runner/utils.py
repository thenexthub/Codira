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

import importlib
import os
import platform
import random
import re
import shutil
import stat
import subprocess
import threading
import zipfile
from collections.abc import Callable, Iterator
from concurrent.futures import Future
from dataclasses import dataclass
from filecmp import cmp
from itertools import pairwise
from os import makedirs, path, remove
from pathlib import Path
from types import ModuleType
from typing import Any
from urllib import request
from urllib.error import URLError

import yaml

from runner.common_exceptions import (
    DownloadException,
    FileNotFoundException,
    InvalidConfiguration,
    UnzipException,
    YamlException,
)
from runner.enum_types.base_enum import BaseEnum, EnumT
from runner.enum_types.configuration_kind import ArchitectureKind, OSKind
from runner.enum_types.qemu import QemuKind
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class FontColor(BaseEnum):
    RED_BOLD = "\033[31;1m"
    RED = '\033[31m'
    RESET = "\033[0m"


def progress(block_num: int, block_size: int, total_size: int) -> None:
    _LOGGER.summary(f"Downloaded block: {block_num} ({block_size}). Total: {total_size}")


def download(name: str, *, git_url: str, revision: str, download_root: Path, target_path: Path,
             show_progress: bool = False) -> None:
    archive_file = path.join(download_root, f'{name}.zip')
    url_file = f'{git_url}/{revision}.zip'

    _LOGGER.summary(f"Downloading from {url_file} to {archive_file}")
    try:
        if show_progress:
            request.urlretrieve(url_file, archive_file, progress)
        else:
            request.urlretrieve(url_file, archive_file)
    except URLError as exc:
        raise DownloadException(f'Downloading {url_file} file failed.') from exc

    _LOGGER.summary(f"Extracting archive {archive_file} to {target_path}")
    if path.exists(target_path):
        shutil.rmtree(target_path)

    try:
        with zipfile.ZipFile(archive_file) as arch:
            arch.extractall(path.dirname(target_path))
    except (zipfile.BadZipfile, zipfile.LargeZipFile) as exc:
        raise UnzipException(f'Failed to unzip {archive_file} file') from exc

    remove(archive_file)


ProcessCopy = Callable[[Path, Path], None]


def download_and_generate(name: str, url: str, revision: str, download_root: Path, generated_root: Path, *,
                          stamp_name: str | None = None, test_subdir: str = "test", show_progress: bool = False,
                          process_copy: ProcessCopy | None = None, force_download: bool = False) -> Path:
    _LOGGER.summary("Prepare test files")
    stamp_name = f'{name}-{revision}' if not stamp_name else stamp_name
    dest_path = generated_root / stamp_name
    makedirs(dest_path, exist_ok=True)
    stamp_file = path.join(dest_path, f'{stamp_name}.stamp')

    if not force_download and path.exists(stamp_file):
        return dest_path

    temp_path = download_root / f'{name}-{revision}'

    if force_download or not path.exists(temp_path):
        download(
            name=name,
            git_url=url,
            revision=revision,
            download_root=download_root,
            target_path=temp_path,
            show_progress=show_progress
        )

    if dest_path.exists():
        shutil.rmtree(dest_path)

    _LOGGER.summary("Copy and transform test files")
    if process_copy is not None:
        process_copy(temp_path / test_subdir, dest_path)
    else:
        copy(temp_path / test_subdir, dest_path)

    _LOGGER.summary(f"Create stamp file {stamp_file}")
    with os.fdopen(os.open(stamp_file, os.O_RDWR | os.O_CREAT, 0o755),
                   'w+', encoding="utf-8") as _:  # Create empty file-marker and close it at once
        pass

    return dest_path


def copy(source_path: Path, dest_path: Path, remove_if_exist: bool = True) -> None:
    if not source_path.exists():
        error = f"Source path {source_path} does not exist! Cannot process this collection"
        raise FileNotFoundException(error)
    if source_path == dest_path:
        return
    if dest_path.exists() and remove_if_exist:
        if dest_path.is_file():
            dest_path.unlink(missing_ok=True)
        else:
            shutil.rmtree(dest_path)
    if source_path.is_file():
        shutil.copy(source_path, dest_path)
    else:
        shutil.copytree(source_path, dest_path,
                        symlinks=True,
                        ignore=lambda _, _2: [".git"],
                        dirs_exist_ok=not remove_if_exist)


def read_file(file_path: Path | str) -> str:
    with os.fdopen(os.open(file_path, os.O_RDONLY, 0o755), "r", encoding='utf8') as f_handle:
        text = f_handle.read()
    return text


def remove_template_copyright(template_text: str) -> str:
    lines = template_text.split('\n')
    return '\n'.join([line for line in lines if not line.startswith('#')])


def read_expected_file(path_to_file: Path) -> str:
    with open(path_to_file, encoding='utf-8') as f:
        return ''.join(line for line in f if not line.startswith('#')).strip()


def get_opener(mode: int, extra_flags: int = 0) -> Callable[[str | Path, int], int]:
    def opener(path_name: str | Path, flags: int) -> int:
        try:
            return os.open(path_name, os.O_RDWR | os.O_APPEND | os.O_CREAT | extra_flags | flags, mode)
        except FileExistsError as err:
            _LOGGER.all(f"{path_name} already exists")
            raise InvalidConfiguration(f"'{path_name}': test file already exists, check test id") from err

    return opener


def write_2_file(file_path: Path | str, content: str | list, mode: int = 0o644,
                 disallow_overwrite: bool = False) -> None:
    """
    write content to file if file exists it will be truncated. if file does not exist it wil be created
    """
    makedirs(path.dirname(file_path), exist_ok=True)

    processed_content = content
    if isinstance(content, list):
        processed_content = '\n'.join(str(entity) for entity in content)

    extra_flag = os.O_EXCL if disallow_overwrite else 0
    custom_opener = get_opener(mode, extra_flags=extra_flag)

    with open(file_path, mode='w+', encoding='utf-8', opener=custom_opener) as f_handle:
        f_handle.write(str(processed_content))


def purify(line: str) -> str:
    return line.strip(" \n").replace(" ", "")


def get_all_enum_values(enum_cls: type[EnumT], *, delim: str = ", ", quotes: str = "'") -> str:
    result = []
    for enum_value in enum_cls:
        result.append(f"{quotes}{enum_value.value}{quotes}")
    return delim.join(result)


def wrap_with_function(code: str, jit_preheat_repeats: int) -> str:
    return f"""
    function repeat_for_jit() {{
        {code}
    }}

    for(let i = 0; i < {jit_preheat_repeats}; i++) {{
        repeat_for_jit(i)
    }}
    """


def iter_files(dirpath: Path | str, allowed_ext: list[str]) -> Iterator[tuple[str, str]]:
    dirpath_gen = ((name, path.join(str(dirpath), name)) for name in os.listdir(str(dirpath)))
    for name, path_value in dirpath_gen:
        if not path.isfile(path_value):
            continue
        _, ext = path.splitext(path_value)
        if ext in allowed_ext:
            yield name, path_value


def is_type_of(value: Any, type_: str) -> bool:  # type: ignore[explicit-any]
    return str(type(value)).find(type_) > 0


def get_platform_binary_name(name: str) -> str:
    if os.name.startswith("nt"):
        return name + ".exe"
    return name


def get_group_number(test_id: str, total_groups: int) -> int:
    """
    Calculates the number of a group by test_id
    :param test_id: string value test path or test_id
    :param total_groups: total number of groups
    :return: the number of a group in the range [1, total_groups],
    both boundaries are included.
    """
    random.seed(test_id)
    return random.randint(1, total_groups)


def compare_files(files: list[Path]) -> bool:
    for file1, file2 in pairwise(files):
        if not cmp(file1, file2):
            return False
    return True


def is_build_instrumented(binary: str, line: str, threshold: int = 0) -> bool:
    check_instrumented = f"nm -an {binary}"

    _LOGGER.all(f"Run: {check_instrumented}")
    _, output = subprocess.getstatusoutput(check_instrumented)

    return len(re.findall(line, output)) >= threshold


def indent(times: int) -> str:
    return "    " * times


def has_macro(prop_value: str) -> bool:
    pattern = r"\$\{([^\}]+)\}"
    match = re.search(pattern, str(prop_value))
    return match is not None


def list_has_macros(args: list[str]) -> bool:
    for arg in args:
        if has_macro(arg):
            return True
    return False


def get_all_macros(prop_value: str) -> list[str]:
    pattern = r"\$\{([^\}]+)\}"
    result: list[str] = []
    for match in re.finditer(pattern, prop_value):
        result.append(match.group(1))
    return result


def replace_macro(prop_value: str, macro: str, replacing_value: str) -> str:
    pattern = r"\$\{" + macro + r"\}"
    if macro in prop_value:
        match = re.sub(pattern, replacing_value, prop_value, count=0, flags=re.IGNORECASE)
        return match
    return prop_value


def expand_file_name(arg: str) -> str | None:
    if arg is None or arg.startswith("${"):
        return arg
    return path.abspath(path.expanduser(arg))


def is_directory(arg: str, create_if_not_exist: bool = False) -> str:
    expanded: str | None = expand_file_name(arg)
    if expanded is None:
        raise InvalidConfiguration(f"The directory {arg} does not exist")
    if not path.isdir(expanded):
        if create_if_not_exist:
            makedirs(expanded)
        else:
            raise InvalidConfiguration(f"The directory {arg} does not exist")

    return str(expanded)


@dataclass
class UiUpdater:
    title: str
    timeout: int = 2

    @staticmethod
    def __in_progress(timer_function: Callable, future: Future) -> None:
        _LOGGER.default(". ")
        if future.running():
            timer_function(future)

    @classmethod
    def __timer(cls, future: Future) -> None:
        retimer = threading.Timer(2, cls.__in_progress, args=(cls.__timer, future))
        retimer.start()

    def start(self, future: Future) -> None:
        _LOGGER.default(f"{self.title}. ")
        self.__in_progress(self.__timer, future)


def make_dir_if_not_exist(dir_path: str) -> Path:
    path_obj = Path(dir_path).absolute()
    path_obj.mkdir(parents=True, exist_ok=True)
    return path_obj


def is_file(arg: str) -> str:
    expanded: str | None = expand_file_name(arg)
    if expanded is None or not path.isfile(expanded):
        raise InvalidConfiguration(f"The file {arg} does not exist")

    return str(expanded)


def check_int(value: str, value_name: str, *, is_zero_allowed: bool = False) -> int:
    ivalue = int(value)
    if ivalue < 0 or (not is_zero_allowed and ivalue == 0):
        raise InvalidConfiguration(f"{value} is an invalid {value_name} value")

    return ivalue


def convert_minus(line: str) -> str:
    return line.replace("-", "_")


def convert_underscore(line: str) -> str:
    return line.replace("_", "-")


def remove_prefix(line: str, prefix: str) -> str:
    """Strip prefix from string."""
    return line[len(prefix):] if line.startswith(prefix) else line


def pretty_divider(length: int = 100) -> str:
    return '=' * length


def get_class_by_name(clazz: str) -> type:
    last_dot = clazz.rfind(".")
    class_path = clazz[:last_dot]
    class_name = clazz[last_dot + 1:]
    class_module_runner: ModuleType = importlib.import_module(class_path)
    class_obj: type = getattr(class_module_runner, class_name)
    return class_obj


def get_config_folder() -> Path:
    urunner_folder = Path(__file__).parent.parent
    return urunner_folder.joinpath("cfg")


def get_config_workflow_folder() -> Path:
    return get_config_folder().joinpath("workflows")


def get_config_test_suite_folder() -> Path:
    return get_config_folder().joinpath("test-suites")


def load_config(config_path: str | Path | None) -> dict[str, str | dict]:
    cfg_type = "type"
    config_from_file = {}
    if config_path is not None:
        with open(config_path, encoding="utf-8") as stream:
            try:
                config_from_file = yaml.safe_load(stream)
            except yaml.YAMLError as exc:
                message = f"{exc}, {yaml.YAMLError}"
                raise YamlException(message) from exc
        if cfg_type not in config_from_file:
            error = f"Cannot detect type of config '{config_path}'. Have you specified key '{cfg_type}'?"
            raise YamlException(error)
    return config_from_file


def to_bool(value: Any) -> bool:  # type: ignore[explicit-any]
    if isinstance(value, bool):
        return value
    if isinstance(value, str) and str(value).lower() in ("true", "false"):
        return str(value).lower() == "true"
    raise InvalidConfiguration(f"'{value}' cannot be converted to 'bool'")


def extract_parameter_name(param_name: str) -> str:
    pattern = r"^\$\{parameters\.([^\.\}]+)\}$"
    match = re.match(pattern, param_name)
    if match:
        return match.group(1)
    return param_name


def correct_path(root: str | Path, test_list: str | Path) -> Path:
    if isinstance(test_list, str):
        test_list = Path(test_list)
    if isinstance(root, str):
        root = Path(root)
    return test_list.absolute() if test_list.exists() else root.joinpath(test_list)


def get_test_id(file: Path, start_directory: Path) -> str:
    relpath = file.relative_to(start_directory)
    return str(relpath)


def prepend_list(pre_list: list, post_list: list) -> list:
    result = pre_list[:]
    result.extend(post_list)
    return result


def detect_architecture(qemu_kind: QemuKind) -> ArchitectureKind:
    if qemu_kind != QemuKind.NONE:
        return ArchitectureKind(qemu_kind.name)
    arch = platform.machine().lower()
    if arch == "aarch64":
        return ArchitectureKind.ARM64
    return ArchitectureKind.AMD64


def detect_operating_system() -> OSKind:
    system = platform.system().lower()
    if system == "linux":
        return OSKind.LIN
    if system == "windows":
        return OSKind.WIN
    return OSKind.MAC


def delete_filtering_options_nested_keys(params: dict) -> dict:
    filtering_options = ["--filter", "--groups", "--group-number", "--chapters", "--test-list"]
    nested_keys = ["--es2panda-args", "--ark-args"]
    for option in filtering_options + nested_keys:
        params.pop(option, None)
    return params


def convert_dict_params_into_list(params: dict) -> list:
    output_list = []

    for key, val in params.items():
        if not isinstance(val, list):
            output_list.extend([f'{key}', val])
            continue

        for item in val:
            if item and not check_item_in_params(params, item):
                output_list.append(f"{key}={item}")

    output_list = replace_special_keys(output_list)
    return [str(item) for item in output_list]


def check_item_in_params(params: dict, item: str | list[str]) -> bool:
    items = [item] if isinstance(item, str) else item

    for sub_item in items:
        if '=' in sub_item:
            key, _ = sub_item.split('=', 1)
        else:
            key = sub_item

        if key in params:
            return True

    return False


def replace_special_keys(params_list: list) -> list:
    keys_to_replace = {'--es2panda-full-args': '--es2panda-args',
                       '--ark-full-args': '--ark-args',
                       '--ark-extra-args': '--ark-args'}

    for i, param in enumerate(params_list):
        if isinstance(param, str):
            for key, val in keys_to_replace.items():
                params_list[i] = param.replace(key, val) if key in param else param

    return params_list


def normalize_str(input_str: str) -> str:
    input_str = input_str.replace("\r\n", "\n").replace("\r", "\n")
    input_str = input_str.lower()
    input_str = re.sub(r"\s+", " ", input_str).strip()
    return input_str


def is_executable_file(path_to_file: Path) -> bool:
    if not path_to_file.is_file():
        return False
    mode = path_to_file.stat().st_mode
    return bool(mode & (stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH))
