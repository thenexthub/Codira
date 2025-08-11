#!/usr/bin/env python3

import sys
import os

# Fix Python import path manually to avoid circular imports
repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "utils"))
sys.path.insert(0, repo_root)

# Import the build script cleanly
build_script_path = os.path.join(repo_root, "build-script")
exec(compile(open(build_script_path, "rb").read(), build_script_path, 'exec'))
