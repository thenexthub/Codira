# arkdb-tests

Integration tests for the Ark Debugger

#### Install environment

```shell
./setup.sh
```

#### Init environment

Activate venv environment

```shell
. .venv/bin/activate
```

Set path to arkcompiler build directory (cmake build directory).

```shell
# .env file

ARK_BUILD=$HOME/projects/arkcompiler/build
```

#### Run

```shell
pytest
```
or
```shell
src/main.py
```
or
```shell
python3 src/main.py
```

In Visual Studio Code press `Ctrl-Shift-T` (Menu / Terminal / Run Tasks...) and select task for run.

#### Format code

```shell
black src
```

#### Linters

```shell
flake8 src
mypy src
```

#### Update license

```shell
scripts/update-license.sh src/**/*.py
```
