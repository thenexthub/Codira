# language_build_support/products/stdlib_docs.py -------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
#
# ----------------------------------------------------------------------------

import os

from . import product
from . import languagedocc
from . import languagedoccrender
from .. import shell


class StdlibDocs(product.Product):
    @classmethod
    def is_build_script_impl_product(cls):
        return False

    @classmethod
    def is_before_build_script_impl_product(cls):
        return False

    @classmethod
    def is_languagepm_unified_build_product(cls):
        return False

    def should_build(self, host_target):
        return self.args.build_stdlib_docs

    def build(self, host_target):
        toolchain_path = self.install_toolchain_path(host_target)
        docc_path = os.path.join(toolchain_path, "bin", "docc")

        language_build_dir = os.path.join(
            os.path.dirname(self.build_dir),
            f'language-{host_target}'
        )
        symbol_graph_dir = os.path.join(language_build_dir, "lib", "symbol-graph")
        output_path = os.path.join(language_build_dir, "Swift.doccarchive")

        docc_action = 'preview' if self.args.preview_stdlib_docs else 'convert'

        docc_cmd = [
            docc_path,
            docc_action,
            "--additional-symbol-graph-dir",
            symbol_graph_dir,
            "--output-path",
            output_path,
            "--default-code-listing-language",
            "language",
            "--fallback-display-name",
            "Swift",
            "--fallback-bundle-identifier",
            "org.language.language",
        ]

        shell.call(docc_cmd)

    def should_test(self, host_target):
        return False

    def should_install(self, host_target):
        return False

    @classmethod
    def get_dependencies(cls):
        """Return a list of products that this product depends upon"""
        return [
            languagedocc.SwiftDocC,
            languagedoccrender.SwiftDocCRender
        ]
