#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

import os
import sys

from sphinx.directives.code import CodeBlock
from docutils import nodes
from docutils.parsers.rst import directives

sys.path.insert(0, os.path.abspath(".."))

import sphinx_common_conf

# -- Project information -----------------------------------------------------

project = "{p} Concurrency Specification".format(p=sphinx_common_conf.project)
author = sphinx_common_conf.author
copyright = sphinx_common_conf.copyright
version = sphinx_common_conf.version
release = sphinx_common_conf.release

rst_epilog = sphinx_common_conf.rst_epilog

language = sphinx_common_conf.default_language
today_fmt = sphinx_common_conf.default_today_fmt

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = ["sphinx_markdown_builder", "sphinx.ext.todo", "sphinxcontrib.plantuml"]

# Add any paths that contain templates here, relative to this directory.
templates_path = []

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
source_suffix = [".rst"]

# The master toctree document.
# CC-OFFNXT(G.NAM.01): project code style
master_doc = "index"

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

# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
# CC-OFFNXT(G.NAM.01): project code style
htmlhelp_basename = "Documentationdoc"

# include refs to todos
# CC-OFFNXT(G.NAM.01): project code style
todo_include_todos = True

# -- Directive options extension ---------------------------------------------


class CustomCodeBlock(CodeBlock):
    option_spec = CodeBlock.option_spec.copy()
    option_spec["language-to-compile"] = lambda arg: arg
    option_spec["donotcompile"] = directives.flag

    def run(self):
        language_value = self.options.get("language-to-compile")
        no_compile = "donotcompile" in self.options
        result = super().run()

        for node in result:
            if isinstance(node, nodes.literal_block):
                node["language-to-compile"] = language_value
                node["donotcompile"] = no_compile

        return result


def setup(app):
    app.add_directive("code-block", CustomCodeBlock, override=True)
