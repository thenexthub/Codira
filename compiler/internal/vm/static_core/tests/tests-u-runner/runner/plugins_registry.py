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
import subprocess
from os import path, listdir, getenv, chdir
from typing import Dict, List, Optional

from runner.logger import Log

_LOGGER = logging.getLogger("runner.plugins_registry")


class PluginsRegistry:
    ENV_PLUGIN_PATH = "PLUGIN_PATH"
    BUILTIN_PLUGINS = "plugins"
    BUILTIN_ROOT = "runner"

    def __init__(self) -> None:
        self.registry: Dict[str, type] = {}
        self.side_plugins: List[str] = []
        self.load_from_env()
        self.load_builtin_plugins()

    @staticmethod
    def filter_builtins(items: List[str]) -> List[str]:
        return [item for item in items if not item.startswith("__")]

    @staticmethod
    def my_dir(obj: str) -> List[str]:
        return PluginsRegistry.filter_builtins(dir(obj))

    def load_plugin(self, plugin_name: str, plugin_path: str) -> None:
        runner_class = [
            cls
            for cls in PluginsRegistry.filter_builtins(listdir(plugin_path))
            if cls.startswith("runner_")
        ]
        runner_class_name = runner_class.pop() if len(runner_class) > 0 else None
        if runner_class_name is not None:
            last_dot = runner_class_name.rfind(".")
            runner_class_name = runner_class_name[:last_dot]
            class_module_name = f"{PluginsRegistry.BUILTIN_ROOT}" \
                                f".{PluginsRegistry.BUILTIN_PLUGINS}" \
                                f".{plugin_name}.{runner_class_name}"
            class_module_root = __import__(class_module_name)
            class_module_plugin = getattr(getattr(class_module_root, PluginsRegistry.BUILTIN_PLUGINS), plugin_name)
            class_module_runner = getattr(class_module_plugin, runner_class_name)
            classes = PluginsRegistry.my_dir(class_module_runner)
            classes = [cls for cls in classes if cls.startswith("Runner") and cls.lower().endswith(plugin_name.lower())]
            class_name = classes.pop() if len(classes) > 0 else None
            if class_name is not None:
                class_obj = getattr(class_module_runner, class_name)
                self.add(plugin_name, class_obj)

    def load_builtin_plugins(self) -> None:
        starting_path = path.join(path.dirname(__file__), PluginsRegistry.BUILTIN_PLUGINS)
        plugin_names = PluginsRegistry.filter_builtins(listdir(starting_path))

        for plugin_name in plugin_names:
            plugin_path = path.join(starting_path, plugin_name)
            if path.isdir(plugin_path):
                self.load_plugin(plugin_name, plugin_path)
            else:
                Log.all(_LOGGER, f"Found extra file '{plugin_path}' at plugins folder")

    def load_from_env(self) -> None:
        builtin_plugins_path = path.join(path.dirname(__file__), PluginsRegistry.BUILTIN_PLUGINS)
        side_plugins = getenv(PluginsRegistry.ENV_PLUGIN_PATH, "").split(path.pathsep)
        if len(side_plugins) == 0:
            return

        chdir(builtin_plugins_path)
        for side_plugin in side_plugins:
            if not path.exists(side_plugin):
                continue
            cmd = ["ln", "-s", side_plugin]
            subprocess.run(cmd, check=True)
            self.side_plugins.append(path.join(
                path.dirname(__file__),
                PluginsRegistry.BUILTIN_PLUGINS,
                path.basename(side_plugin)
            ))

    def add(self, runner_name: str, runner: type) -> None:
        if runner_name not in self.registry:
            self.registry[runner_name] = runner
            Log.all(_LOGGER, f"Registered plugin '{runner_name}' with class '{runner.__name__}'")
        else:
            Log.exception_and_raise(_LOGGER, f"Plugin '{runner_name}' already registered")

    def get_runner(self, name: str) -> Optional[type]:
        return self.registry.get(name)

    def cleanup(self) -> None:
        if len(self.side_plugins) == 0:
            return
        cmd = ["rm"] + self.side_plugins
        subprocess.run(cmd, check=True)
