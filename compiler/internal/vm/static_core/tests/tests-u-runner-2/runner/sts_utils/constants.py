#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import re

# Meta
META_START_STRING = "/*---"
META_START_PATTERN = re.compile(r"\/\*---")
META_END_STRING = "---*/"
META_END_PATTERN = re.compile(r"---\*\/")
META_START_COMMENT = "/*"
META_END_COMMENT = "*/"
META_COPYRIGHT = "Copyright (c)"

# Extensions
YAML_EXTENSIONS = [".yaml", ".yml"]
TEMPLATE_EXTENSION = ".ets"
OUT_EXTENSION = ".ets"
JAR_EXTENSION = ".jar"

# Prefixes
LIST_PREFIX = "list."
NEGATIVE_PREFIX = "n."
NEGATIVE_EXECUTION_PREFIX = "ne."
SKIP_PREFIX = "tbd."

# Jinja
VARIABLE_START_STRING = "{{."

# Spec
SPEC_SECTION_TITLE_FIELD_NAME = "name"
SPEC_SUBSECTIONS_FIELD_NAME = "children"
