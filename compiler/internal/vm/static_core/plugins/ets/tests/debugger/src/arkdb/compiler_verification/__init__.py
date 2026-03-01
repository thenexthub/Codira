#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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


class ExpressionEvaluationNames:
    NON_DIFF_MARKS = set([" ", "?"])
    EVAL_PATCH_FUNCTION_NAME = "evalPatchFunction"
    EVAL_PATCH_RETURN_VALUE = "evalGeneratedResult"
    EVAL_METHOD_GENERATION_PATTERN = r"^eval_[a-zA-Z\-_\d]*_[\d]+_eval"
    EVAL_METHOD_GENERATED_NAME_RE = re.compile(EVAL_METHOD_GENERATION_PATTERN + r"$")
    EVAL_METHOD_RETURN_VALUE_RE = re.compile(EVAL_METHOD_GENERATION_PATTERN + r"_generated_var$")
