#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

#
# Common settings for the entire documentation bundle:
#

project = u'ArkTS'

# CC-OFFNXT(G.NAM.01): project code style
author = u''

copyright = u'2021-2026 Huawei Device Co., Ltd.'

# The short X.Y version
# CC-OFFNXT(G.NAM.01): project code style
version = u'1.2.1'

# The full version, including alpha/beta/rc tags
# CC-OFFNXT(G.NAM.01): project code style
release = u'1.2.1-alpha TECHNICAL PREVIEW 3'

# Common glossary for the entire documentation bundle:
rst_epilog = '''
.. |CB_BAD| replace:: TypeScript
.. |CB_OK| replace:: {lang}
.. |CB_NON_COMPLIANT_CODE| replace:: Non-compliant code
.. |CB_COMPLIANT_CODE| replace:: Compliant code
.. |CB_R| replace:: Recipe:
.. |CB_RULE| replace:: Rule
.. |CB_ERROR| replace:: **Severity: error**
.. |CB_WARNING| replace:: **Severity: warning**
.. |CB_SEE| replace:: See also
.. |JS| replace:: JavaScript
.. |LANG| replace:: {lang}
.. |UIFW| replace:: ArkUI
.. |TS| replace:: TypeScript
.. |C_JOB| replace:: Job
.. |C_JOBS| replace:: Jobs
.. |C_CORO| replace:: Suspendable Job
.. |C_COROS| replace:: Suspendable Jobs
.. |C_WORKER| replace:: Worker Thread
.. |C_WORKERS| replace:: Worker Threads
'''.format(lang=project)

#
# Common defaults (not required, but welcome for usage):
#

default_language = 'en'

default_today_fmt = '%Y.%m.%d'

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
default_html_theme = 'alabaster'
