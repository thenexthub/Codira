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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

import os
import sys
sys.path.insert(0, os.path.abspath('..'))

import sphinx_common_conf

# -- Project information -----------------------------------------------------

project = u'{p} System ArkTS'.format(p=sphinx_common_conf.project)
author = sphinx_common_conf.author
copyright = sphinx_common_conf.copyright
version = sphinx_common_conf.version
release = sphinx_common_conf.release

rst_prolog = '''
.. role:: kw(code)
   :language: typescript

'''

if tags.has('ispdf'):
    rst_prolog += '''
.. |nbsp| unicode:: U+0020

'''
else:
    rst_prolog += '''
.. |nbsp| unicode:: U+00A0 .. NO-BREAK SPACE
'''

rst_epilog = sphinx_common_conf.rst_epilog

language = sphinx_common_conf.default_language
today_fmt = sphinx_common_conf.default_today_fmt

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = ['sphinx_markdown_builder']

# Add any paths that contain templates here, relative to this directory.
templates_path = []

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
source_suffix = ['.rst']

# The master toctree document.
master_doc = 'index'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = None

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = sphinx_common_conf.default_html_theme
html_css_files = [
    'css/stdlib.css',
]

# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'Documentationdoc'

# -- Options for PDF output --------------------------------------------------

pdf_stylesheets = ['sphinx', 'stdlib']

# To build PDFs, install rst2pdf package:
#     sudo pip3 install rst2pdf
# After that, run:
#     sphinx-build -n -b pdf /path/to/source /path/to/output
pdf_documents = [(u'index', u'{l}-{v}'.format(l=project, v=version), project,
                  author)]
