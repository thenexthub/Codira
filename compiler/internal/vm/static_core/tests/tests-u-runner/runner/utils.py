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
import random
import shutil
import zipfile
from enum import Enum
from filecmp import cmp
from itertools import tee
from os import makedirs, path, remove
from pathlib import Path
from typing import TypeVar, Callable, Optional, Type, Union, Any, List, Iterator, Tuple, Iterable
from urllib import request
from urllib.error import URLError

from runner.logger import Log

_LOGGER = logging.getLogger("runner.utils")

EnumT = TypeVar("EnumT", bound=Enum)


def progress(block_num: int, block_size: int, total_size: int) -> None:
    Log.summary(_LOGGER, f"Downloaded block: {block_num} ({block_size}). Total: {total_size}")


def download(name: str, git_url: str, revision: str, target_path: str, show_progress: bool = False) -> None:
    archive_file = path.join(path.sep, 'tmp', f'{name}.zip')
    url_file = f'{git_url}/{revision}.zip'

    Log.summary(_LOGGER, f"Downloading from {url_file} to {archive_file}")
    try:
        if show_progress:
            request.urlretrieve(url_file, archive_file, progress)
        else:
            request.urlretrieve(url_file, archive_file)
    except URLError:
        Log.exception_and_raise(_LOGGER, f'Downloading {url_file} file failed.')

    Log.summary(_LOGGER, f"Extracting archive {archive_file} to {target_path}")
    if path.exists(target_path):
        shutil.rmtree(target_path)

    try:
        with zipfile.ZipFile(archive_file) as arch:
            arch.extractall(path.dirname(target_path))
    except (zipfile.BadZipfile, zipfile.LargeZipFile):
        Log.exception_and_raise(_LOGGER, f'Failed to unzip {archive_file} file')

    remove(archive_file)


ProcessCopy = Callable[[str, str], None]


def generate(name: str, url: str, revision: str, generated_root: Path, *,
             stamp_name: Optional[str] = None, test_subdir: str = "test", show_progress: bool = False,
             process_copy: Optional[ProcessCopy] = None, force_download: bool = False) -> str:
    Log.summary(_LOGGER, "Prepare test files")
    stamp_name = f'{name}-{revision}' if not stamp_name else stamp_name
    dest_path = path.join(generated_root, stamp_name)
    makedirs(dest_path, exist_ok=True)
    stamp_file = path.join(dest_path, f'{stamp_name}.stamp')

    if not force_download and path.exists(stamp_file):
        return dest_path

    temp_path = path.join(path.sep, 'tmp', name, f'{name}-{revision}')

    if force_download or not path.exists(temp_path):
        download(name, url, revision, temp_path, show_progress)

    if path.exists(dest_path):
        shutil.rmtree(dest_path)

    Log.summary(_LOGGER, "Copy and transform test files")
    if process_copy is not None:
        process_copy(path.join(temp_path, test_subdir), dest_path)
    else:
        copy(path.join(temp_path, test_subdir), dest_path)

    Log.summary(_LOGGER, f"Create stamp file {stamp_file}")
    with os.fdopen(os.open(stamp_file, os.O_RDWR | os.O_CREAT, 0o755),
                   'w+', encoding="utf-8") as _:  # Create empty file-marker and close it at once
        pass

    return dest_path


def copy(source_path: Union[Path, str], dest_path: Union[Path, str], remove_if_exist: bool = True) -> None:
    try:
        if source_path == dest_path:
            return
        if path.exists(dest_path) and remove_if_exist:
            shutil.rmtree(dest_path)
        shutil.copytree(source_path, dest_path, dirs_exist_ok=not remove_if_exist)
        shutil.copymode(source_path, dest_path)
    except OSError as ex:
        Log.exception_and_raise(_LOGGER, str(ex))


def read_file(file_path: Union[Path, str]) -> str:
    with os.fdopen(os.open(file_path, os.O_RDONLY, 0o755), "r", encoding='utf8') as f_handle:
        text = f_handle.read()
    return text


def write_2_file(file_path: Union[Path, str], content: str) -> None:
    """
    write content to file if file exists it will be truncated. if file does not exist it wil be created
    """
    makedirs(path.dirname(file_path), exist_ok=True)
    f_descriptor = os.open(file_path, os.O_RDWR | os.O_CREAT | os.O_TRUNC, 0o755)
    with os.fdopen(f_descriptor, mode='w+', encoding="utf-8") as f_handle:
        f_handle.write(content)


def get_opener(mode: int) -> Callable[[Union[str, Path], int], int]:
    def opener(path_name: Union[str, Path], flags: int) -> int:
        return os.open(path_name, os.O_RDWR | os.O_APPEND | os.O_CREAT | flags, mode)
    return opener


def write_list_to_file(list_for_write: list, file_path: Path, mode: int = 0o644) -> None:
    custom_opener = get_opener(mode)
    with open(file_path, mode='w+', encoding='utf-8', opener=custom_opener) as file:
        for entity in list_for_write:
            file.write(f"{entity}\n")


def purify(line: str) -> str:
    return line.strip(" \n").replace(" ", "")


def enum_from_str(item_name: str, enum_cls: Type[EnumT]) -> Optional[EnumT]:
    for enum_value in enum_cls:
        if enum_value.value.lower() == item_name.lower() or str(enum_value).lower() == item_name.lower():
            return enum_cls(enum_value)
    return None


def wrap_with_function(code: str, jit_preheat_repeats: int) -> str:
    return f"""
    function repeat_for_jit() {{
        {code}
    }}

    for(let i = 0; i < {jit_preheat_repeats}; i++) {{
        repeat_for_jit(i)
    }}
    """


def iter_files(dirpath: Union[Path, str], allowed_ext: List[str]) -> Iterator[Tuple[str, str]]:
    dirpath_gen = ((name, path.join(str(dirpath), name)) for name in os.listdir(str(dirpath)))
    for name, path_value in dirpath_gen:
        if not path.isfile(path_value):
            continue
        _, ext = path.splitext(path_value)
        if ext in allowed_ext:
            yield name, path_value


def is_type_of(value: Any, type_: str) -> bool:
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


# from itertools import pairwise when switching to python version >= 3.10
# pylint: disable=invalid-name
def pairwise(iterable: Iterable[Path]) -> Iterator[Tuple[Path, Path]]:
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


def compare_files(files: List[Path]) -> bool:
    for f1, f2 in pairwise(files):
        if not cmp(f1, f2):
            return False
    return True

def unlines(lines: Iterable[str]) -> str:
    return ''.join(line+"\n" for line in lines)
