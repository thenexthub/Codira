# VMB module development notes

## Development

### Code Style

VMB code is assumed to be compatible with `python3.7` and higher,
although usually 3.8 and 3.10 is used.
Please make sure to not add 3.8+ features,
(f.e. using built-in type annotations as `list` instead `typing.List`).
If such a feature addition could be justified, please add it with usage of `__future__`
and checks that code actually works on `python3.7`

For the reason of compatibility with internal code checkers maximum line width is set to 120,
though [PEP-8](https://peps.python.org/pep-0008/#maximum-line-length) recommends 80.

To quick-check your changes, please do `make tox` - this will run linters, and unit-tests.

`pytest` is used as a unit test framework,
though its features have not been used fully due to internal code checkers limitations.
F.e. powerful rewritten `assert` is banned in favor of `unittest.TestCase.assert*`

### Versioning
There is neither rules on module version update nor automatic CI-based version incrementation.
To bump version these files should be updated:
- `pyproject.toml` (`version = "1.0.0"`)
- `src/vmb/vmb.py` (`VERSION = '1.0.0'`)

## CLI
As many python tools (`pip`, `virtualenv` etc)
`vmb` module provides small cli entry point,
which is basically a `sys.exit(vmb.main())`

This is controlled by `pyproject.toml`:
```
[project.scripts]
vmb = "vmb:main"
```

It is placed to `$HOME/.local/bin` (or similar place) during module installation.
Note that `templates` and other `resources` are placed inside module directory.
To find out where your modules reside, issue something like this:

```shell
python3 -c 'import vmb; print(vmb.__file__)'
# /home/user/.local/lib/python3.10/site-packages/vmb/__init__.py
```

## Workflow

Normal usage involves these stages:
1. Generation (`vmb gen ...`)
2. Run (`vmb run ...`)
3. Report (`vmb report ...`)

All three stages combined in `vmb all ...`

## Code structure

VMB is built around the idea of pluggable sub-modules.
It shipped with ready-to-use (aka 'default') plugins in `src/vmb/plugins`.
See [readme on extra plugins](./readme.md#extra-custom-plugins) on adding user-defined plugins.
There are 4 kinds of plugins:

#### 1) Platform plugins

Platform is main 'runner' for particular VM (product to test)
and 'target' (machine to run on).
It defines which languages it supports and how to process each `Bench Unit`.
It is a subclass of `src/vmb/platform.py::PlatformBase`

#### 2) Tool plugins

Tool is wrapper for executable/command (compiler or vm)
and should implement `src/vmb/tool.py::ToolBase`
and should expose `exec(BenchUnit)` method.

#### 3) Language plugins

Lang should override `src/vmb/lang.py::LangBase`.
Generally it contains regular expressions to parse particular source.

#### 4) Hooks plugins

Hook subclasses `src/vmb/hook.py::HookBase` and can define some
additional actions before/after suite/test.

### Bench Unit

`BU` (bench unit) is an entity for holding info on particular test:
its paths, sources, generated sources, compiled binaries etc.
It also accumulates results of tests.

## Report Structure
TODO

## FAQ: Why `Popen(shell=True)` is used in `shell.py::ShellHost`?
Although this is not recommended from security point of view,
this `shell=True` is needed to use unix **shell builtin** command `time`
(to get some process info, `RSS` particularly)
in **predictable cross-platform** manner. Unfortunately implementations
of `/bin/time` differ between devices significantly.

Also note that on Ubuntu `time` builtin still depends on `/usr/bin/time`
(not on `/bin/time`) so corresponding package should be installed
to measure host command's times and RSS.

## FAQ: Is there strict one-to-one correspondence b/w language and file extension?
No. Generally `*.ets` file means `ets` language, but `*.ts` file could be treated
as `ts` or `ets` depending on `--langs` `--src-langs` options.
(This was a requested feature of _Using same sources for TS and ArkTS_)

Also `*.js` doclet sources produce `*.mjs` files on test generation.

## FAQ: How to run tests and linters for all available python versions?

```sh
# install needed versions, f.e. build them from sources:
./configure --prefix=$HOME/bin/py$VERSION && make && make install
cd $vmb_src
PATH=$PATH:$HOME/bin/py313/bin:$HOME/bin/py38/bin:$HOME/bin/py37/bin make tox
```

## FAQ: Why paths is not fully cross-platform?

`pathlib::Path` is cross-platform, but running device platforms (which is unix)
from Windows host we need to make sure paths on device will be
expanded using `/` (unix separators).
Optimal solution would be make such fields/variables of `pathlib::PosixPath` type,
but by some reason it could not be instantiated on Windows,
so `as_posix()` is massively used throughout code.

This should be refactored and generalized.

## FAQ: How to explore module structure without source repo?

After module installed locally it can be explored f.e. in python interactive shell:
```python
>>> import vmb
>>> vmb.  # press TAB after any '.' to get hint and autocomplete:
# vmb.VERSION   vmb.helpers          vmb.main()    vmb.shell   vmb.vmb
# vmb.cli       vmb.helpers_numbers  vmb.platform  vmb.sys     vmb.x_shell

>>> print(vmb.gensettings.GenSettings.__doc__)
# Template overrides class.
#    In most cases template name, source and bench file extensions
#    are set by selected lang,
#    but for some platforms these defaults needs to be overridden.

>>> vmb.x_shell.ShellBase.__annotations__
# {'_Singleton__instances': typing.Dict[typing.Any, typing.Any]}

```

In case of python code performance issues run it like that:
```shell
python3 -m cProfile --sort=cumtime -m vmb gen -l ets ./examples/benchmarks/ets
```

