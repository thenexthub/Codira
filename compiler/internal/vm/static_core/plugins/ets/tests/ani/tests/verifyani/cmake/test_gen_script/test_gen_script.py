#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

import argparse
import os
from jinja2 import Environment, FileSystemLoader


def main() -> None:
    parser = argparse.ArgumentParser(description='Genereting test files')
    parser.add_argument('--static_core_dir', required=True,
                        type=str, help='static_Core directory')
    parser.add_argument('--j2_cpp_source', required=True,
                        type=str, help='.cpp source')
    parser.add_argument('--j2_ets_source', required=False,
                        type=str, help='.ets source')
    parser.add_argument('--info', required=True, type=str,
                        help='Params for templates and target name')
    parser.add_argument('--output_cpp', required=True,
                        type=str, help='.cpp file path for generated test')
    parser.add_argument('--output_ets', required=False,
                        type=str, help='.ets file path for generated test')

    args = parser.parse_args()

    output_dir = os.path.dirname(args.output_cpp)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    _, params_string = args.info.split(": ")

    params = {}
    for item in params_string.split(','):
        key, value = item.split('=')
        params[key] = value
    env = Environment(loader=FileSystemLoader(os.path.dirname("/")))
    if args.j2_cpp_source:
        template = env.get_template(args.j2_cpp_source)
        output_cpp_data = template.render(params)
        with open(args.output_cpp, "w") as cpp_file:
            cpp_file.write(output_cpp_data + "\n")

    if args.j2_ets_source:
        template = env.get_template(args.j2_ets_source)
        output_ets_data = template.render(params)
        with open(args.output_ets, "w") as ets_file:
            ets_file.write(output_ets_data + "\n")


if __name__ == "__main__":
    main()
