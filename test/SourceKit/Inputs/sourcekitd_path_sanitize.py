#!/usr/bin/env python3
# sourcekitd_path_sanitize.py - Cleans up paths from sourcekitd-test output
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2019 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import re
import sys

# I'm sorry dear reader, unfortunately my knowledge of regex trumps my knowledge of
# Python. This pattern allows us to clean up file paths by stripping them down to
# just the relevant file name.
RE = re.compile(
    # The key can either be 'filepath' or 'buffer_name'. Also apply this logic to
    # the file name in XML, which is written 'file=\"...\"'.
    r'(key\.(?:filepath|buffer_name): |file=)'

    # Open delimiter with optional escape.
    r'\\?"'

    # Lazily match characters until we hit a slash, then match any non-slash that
    # ends in the right file extension, capturing the result.
    r'.*?[/\\]+([^/\\]*\.(?:languagemodule|language|pcm|h))'

    # For languagemodule bundles, we want to match against the directory name, so
    # optionally match the .codemodule filename here. The lazy matching of the
    # previous logic means we'll prefer to match the previous '.codemodule' as the
    # directory.
    r'(?:[/\\]+[^/\\]*\.codemodule)?'

    # Close delimiter with optional escape.
    r'\\?"'
)

try:
    for line in sys.stdin.readlines():
        # We substitute in both the key and the matched filename.
        line = re.sub(RE, r'\1\2', line)
        sys.stdout.write(line)
except KeyboardInterrupt:
    sys.stdout.flush()
