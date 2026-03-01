# coding=utf-8
#
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

from taihe.utils.resources import DeploymentMode, ResourceContext


def test_dev():
    p = "/tmp/repo_root/compiler/taihe/utils/resources.py"
    loc = ResourceContext.from_path(p)
    pass
    assert loc.base_dir.name == "repo_root"


def test_pkg():
    p = ".venv/lib/python3.12/site-packages/taihe/utils/resources.py"
    loc = ResourceContext.from_path(p)
    pass
    assert loc.base_dir.name == "data"


def test_bundle():
    p = "taihe-pkg/lib/pyrt/lib/python3.11/site-packages/taihe/utils/resources.py"
    loc = ResourceContext.from_path(p)
    pass
    assert loc.base_dir.name == "taihe-pkg"