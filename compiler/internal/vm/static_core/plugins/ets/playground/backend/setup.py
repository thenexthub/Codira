#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

try:
    from setuptools import setup, find_packages
except ImportError:
    try:
        from setuptools.core import setup
    except ImportError:
        from distutils.core import setup


def format_local(version):
    """Format a local version"""
    dist = version.distance or 0
    loc = "dirty." if version.dirty else ""
    return "" if dist == 0 and not version.dirty else f"+{loc}{version.node}"


if __name__ == "__main__":
    setup(
        use_scm_version={
            'local_scheme': format_local
        }
    )
