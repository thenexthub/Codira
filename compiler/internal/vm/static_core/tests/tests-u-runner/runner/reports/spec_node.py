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

from __future__ import annotations
from typing import List, Optional


class SpecNode:
    def __init__(self, title: str, prefix: str, status: str, parent: Optional[SpecNode]):
        self.title = title
        self.prefix = prefix
        self.status = status
        self.parent: Optional[SpecNode] = parent
        self.dir: str = ""
        self.children: List[SpecNode] = []
        if parent:
            parent.children.append(self)

        # Counters with recursive accumulation from children:
        self.total_acc: int = 0
        self.passed_acc: int = 0
        self.failed_acc: int = 0
        self.ignored_acc: int = 0
        self.ignored_but_passed_acc: int = 0
        self.excluded_acc: int = 0
        self.excluded_after_acc: int = 0

        # Counters without accumulation:
        self.total: int = 0
        self.passed: int = 0
        self.failed: int = 0
        self.ignored: int = 0
        self.ignored_but_passed: int = 0
        self.excluded: int = 0
        self.excluded_after: int = 0

    def to_dict(self) -> dict:
        ret: dict = {
            'title': self.title
        }

        if len(self.prefix) > 0:
            ret['prefix'] = self.prefix

        if len(self.status) > 0:
            ret['status'] = self.status

        if len(self.dir) > 0:
            ret['test_dir'] = self.dir

        if self.total:
            ret['total'] = self.total
            ret['passed'] = self.passed
            ret['failed'] = self.failed
            ret['ignored'] = self.ignored
            ret['ignored_but_passed'] = self.ignored_but_passed
            ret['excluded'] = self.excluded
            ret['excluded_after'] = self.excluded_after

        if self.total_acc:
            ret['total_acc'] = self.total_acc
            ret['passed_acc'] = self.passed_acc
            ret['failed_acc'] = self.failed_acc
            ret['ignored_acc'] = self.ignored_acc
            ret['ignored_but_passed_acc'] = self.ignored_but_passed_acc
            ret['excluded_acc'] = self.excluded_acc
            ret['excluded_after_acc'] = self.excluded_after_acc

        if len(self.children) > 0:
            ret['subsections'] = self.children

        return ret
