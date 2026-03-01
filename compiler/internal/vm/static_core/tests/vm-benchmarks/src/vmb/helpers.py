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

import os
import json
import logging
import sys
import random
import string
import shutil
from pathlib import Path
from string import Template
from typing import Union, Dict, Any, Callable, List, Iterable, Optional
from time import time
from enum import Enum
from datetime import datetime, timezone, timedelta
from importlib.util import spec_from_file_location, module_from_spec

PASS_LOG_LEVEL = logging.ERROR + 1
TRACE_LOG_LEVEL = logging.DEBUG - 1
log = logging.getLogger('vmb')


def ensure_env_var(var: str) -> str:
    """Ensure that env variable is set."""
    val = os.environ.get(var, '')
    die(not val, 'Please set "%s" env var!', var)
    return val


def log_pass(self, message, *args, **kws):
    """Inject new log level above info."""
    if self.isEnabledFor(PASS_LOG_LEVEL):
        # pylint: disable-next=protected-access
        self._log(PASS_LOG_LEVEL, message, args, **kws)


def log_trace(self, message, *args, **kws):
    """Inject new log level above info."""
    if self.isEnabledFor(TRACE_LOG_LEVEL):
        # pylint: disable-next=protected-access
        self._log(TRACE_LOG_LEVEL, message, args, **kws)


def rnd_string(size=8):
    """Random string of fixed size."""
    return ''.join(
        random.choice(
            string.ascii_uppercase + string.digits) for _ in range(size))


def pad_left(s: str, ln: int, limit: int = 80) -> str:
    """Reurn string left-padded with spaces."""
    ln = min(ln, limit)
    return f'{s[:ln]:<{ln}}'


def remove_prefix(s: str, prefix: str) -> str:
    """Strip prefix from string."""
    return s[len(prefix):] if s.startswith(prefix) else s


def split_params(line: str) -> List[str]:
    """Split comma-separated string into list."""
    return [t.strip() for t in line.split(',') if t.strip()]


def norm_list(it: Optional[Iterable[str]]) -> List[str]:
    """Remove duplicates and cast to lower case."""
    return sorted([t.lower() for t in set(it)]) if it else []


def log_time(f: Callable[..., Any]) -> Callable[..., Any]:
    """Annotation for debug performance."""
    def f1(*args: Any, **kwargs: Any) -> Any:
        log.trace('%s started', f.__name__)
        start = time()
        ret = f(*args, **kwargs)
        log.trace('%s finished in %s',
                  f.__name__,
                  str(timedelta(seconds=time() - start)))
        return ret
    return f1


def read_list_file(list_file: Union[str, Path]) -> List[str]:
    """List file to array."""
    path = Path(list_file)
    if not path.is_file():
        log.error('List file not found: %s', path)
        return []
    with open(path, 'r', encoding="utf-8") as f:
        lst = list(filter(
            lambda x: x and not x.startswith('#'),
            f.read().splitlines()))
    return lst


def import_module(module_name, path):
    """Import py file as a module."""
    spec = spec_from_file_location(module_name, path)
    if spec is None:
        raise RuntimeError(f'Import module from "{path}" failed')
    module = module_from_spec(spec)
    loader = spec.loader
    if loader:
        loader.exec_module(module)
    return module


def get_plugin(plug_type: str,
               plug_name: str,
               extra: Optional[Path] = None) -> Any:
    """Return plugin."""
    # try extra dir first
    if extra:
        die(not extra.is_dir(),
            'Extra plugins dir "%s" does not exist!', extra)
        py = extra.joinpath(plug_type, f'{plug_name}.py')
        if py.is_file():
            return import_module(plug_name, str(py))
    # load default in case there is no extra one
    py = Path(__file__).parent.resolve().joinpath(
        'plugins', plug_type, f'{plug_name}.py')
    if py.is_file():
        return import_module(plug_name, str(py))
    log.fatal('No such plugin: "%s"\n'
              'Searching here: "%s"\n'
              'To see available plugins: `vmb list`', plug_name, py)
    sys.exit(1)


def get_plugins(plug_type: str,
                plugins: List[str],
                extra: Optional[Path]) -> Dict[str, Any]:
    """Return dict of plugins."""
    plugs = {}
    for plug_name in plugins:
        plug = get_plugin(plug_type, plug_name, extra)
        plugs[plug_name] = plug
    return plugs


class Timer:
    """Simple struct for begin-end."""

    tz = datetime.now(timezone.utc).astimezone().tzinfo
    tm_format = "%Y-%m-%dT%H:%M:%S.00000%z"

    def __init__(self) -> None:
        self.begin = datetime.now(timezone.utc)
        self.end = self.begin

    @staticmethod
    def format(t) -> str:
        if not isinstance(t, datetime):
            return 'unknown'
        return t.astimezone(Timer.tz).strftime(Timer.tm_format)

    def start(self) -> None:
        self.begin = datetime.now(timezone.utc)
        self.end = self.begin

    def finish(self) -> None:
        self.end = datetime.now(timezone.utc)

    def elapsed(self) -> timedelta:
        return self.end - self.begin


class Singleton(type):
    """Singleton."""

    __instances: Dict[Any, Any] = {}

    def __call__(cls, *args: Any, **kwargs: Any) -> Any:
        """Instantiante singleton."""
        if cls not in cls.__instances:
            cls.__instances[cls] = \
                super(Singleton, cls).__call__(*args, **kwargs)
        return cls.__instances[cls]


class StringEnum(Enum):
    """String Enum."""

    def __lt__(self, other):
        return self.value < other.value

    @classmethod
    def getall(cls) -> List[str]:
        return [str(x.value) for x in cls]


class Jsonable:
    """Base class (abstract) for json-serialiazation."""

    @staticmethod
    def get_props(obj):
        """Search for properties."""
        # add all 'public' fields
        props = {k: v for k, v in obj.__dict__.items()
                 if not str(k).startswith('_')}
        # add all @property-decorated fields
        props.update(
            {name: value.fget(obj) for name, value
             in vars(obj.__class__).items()
             if isinstance(value, property)})
        return props

    def js(self, sort_keys=False, indent=4) -> str:
        """Serialize object to json."""
        return json.dumps(
            self,
            default=Jsonable.get_props,
            sort_keys=sort_keys,
            indent=indent)

    def save(self, json_file: Union[Path, str]) -> None:
        with create_file(json_file) as f:
            f.write(self.js())


class ColorFormatter(logging.Formatter):
    """Colorful log."""

    red = "\x1b[31;20m"
    green = "\x1b[32;20m"
    grey = "\x1b[38;20m"
    magenta = "\x1b[35;20m"
    cyan = "\x1b[36;1m"
    yellow = "\x1b[33;20m"
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"
    orange = "\x1b[33;20m"
    bold_blue = "\x1b[34;1m"
    fmt = '%(message)s'
    ts = ''

    FORMATS = {
        TRACE_LOG_LEVEL: magenta + fmt + reset,
        logging.DEBUG: bold_blue + fmt + reset,
        logging.INFO: cyan + fmt + reset,
        PASS_LOG_LEVEL: green + fmt + reset,
        logging.WARNING: yellow + fmt + reset,
        logging.ERROR: red + fmt + reset,
        logging.CRITICAL: bold_red + fmt + reset
    }

    def __init__(self, timestamps: bool = False):
        super().__init__()
        if timestamps:
            ColorFormatter.ts = '[%(asctime)s.000Z] '

    def format(self, record):
        """Format."""
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(ColorFormatter.ts + log_fmt, '%Y-%m-%dT%H:%M:%S')
        return formatter.format(record)


def create_file(path: Union[str, Path]):
    """Create file in `safe` manner."""
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    return os.fdopen(
        os.open(path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o664),
        mode='w', encoding='utf-8')


def check_file_exists(file_path: Union[str, Path]) -> None:
    path = Path(file_path)
    if not path.exists():
        error_text = 'File not found: "' + str(path) + '"'
        log.error(error_text)
        sys.exit(1)


def copy_file(src: Union[str, Path], dst: Union[str, Path]) -> None:
    s = Path(src)
    if not s.exists():
        raise RuntimeError(f'File not found: {src}')
    d = Path(dst)
    if d.is_dir():
        # copy to existing dir dst
        d = d.joinpath(s.name)
    else:
        # copy to dst as full path
        d.parent.mkdir(parents=True, exist_ok=True)
    log.trace('Copy: %s -> %s', str(s), str(d))
    shutil.copy(s, d)


def copy_files(src: Union[str, Path], dst: Union[str, Path], pattern: str = '*') -> None:
    log.trace('Copy: %s%s -> %s', str(src), pattern, str(dst))
    s = Path(src)
    d = Path(dst)
    if not s.exists():
        raise RuntimeError(f'File not found: {src}')
    d.mkdir(parents=True, exist_ok=True)
    for f in s.glob(pattern):
        shutil.copy(f, d.joinpath(f.name))


def create_file_from_template(tpl: Union[str, Path], dst: Union[str, Path], **kwargs) -> None:
    with open(tpl, 'r', encoding="utf-8") as src:
        template = Template(src.read())
        with create_file(dst) as f:
            f.write(template.substitute(**kwargs))


def load_file(path: Union[str, Path]) -> str:
    """Read file to string."""
    fd = os.fdopen(
        os.open(path, os.O_RDONLY), mode="r", encoding='utf-8', buffering=1)
    fd.seek(0)
    return fd.read()


def load_json(path: Union[str, Path]) -> Any:
    json_path = Path(path)
    if not json_path.exists():
        raise RuntimeError(f'File not found: {path}')
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            j = json.load(f)
    except json.JSONDecodeError as e:
        log.error('Bad json: %s\n%s', str(path), str(e))
        raise RuntimeError from e
    return j


def force_link(link: Path, dest: Path) -> None:
    log.trace('Force link: %s -> %s', str(link), str(dest))
    if link.exists():
        link.unlink()
    if 'nt' == os.name:
        copy_file(dest, link)
        return
    link.symlink_to(dest)


def die(condition: bool, *msg) -> None:
    if condition:
        log.fatal(*msg)
        sys.exit(1)
