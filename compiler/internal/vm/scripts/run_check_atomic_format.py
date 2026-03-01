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

"""
A tool to check atomic memory order format.
"""

import re
import sys

if len(sys.argv) < 2:
    sys.exit("File name to be checked was not specified")
FILE = sys.argv[1]

MAX_OFFSET = 3
EMPTY_MEMORY_ORDER = "empty"
PARAMETERIZED_MEMORY_ORDER = "parameterized"
IGNORED_PREFIXES = ["ATOMIC_STORE", "ATOMIC_LOAD", "ATOMIC_FETCH_ADD",
                    "ATOMIC_FETCH_SUB", "ATOMIC_CAS_WEAK"]


def obtain_memory_order(parsed_line: str) -> []:
    memory_orders = list()
    while True:
        parsed_res = re.search(r"memory_order_(\w+)", parsed_line)
        if parsed_res:
            memory_order = parsed_res.group(1)
            memory_orders.append(memory_order)
            parsed_line = parsed_line.replace(memory_order, "", 1)
        else:
            break
    return memory_orders


def parse_file() -> ():
    atomic_comments = dict()
    atomic_operations = dict()
    cur_parser_label = 0
    with open(FILE) as parsed_file:
        for index, line in enumerate(parsed_file.readlines(), 1):
            res = re.search(r"// Atomic with (\w+) order reason: (.+)", line)
            if res:
                memory_order = res.group(1)
                atomic_comments[index] = memory_order

            # TODO: Support compare_exchange_strong|compare_exchange_weak
            res = re.search(r"(.*)(\.|->)(store|load|fetch_add|fetch_sub|"
                            r"fetch_or|fetch_xor|fetch_and)\((.+)", line)
            if res:
                prefix = res.group(1)
                is_ignored = False
                for ignored_prefix in IGNORED_PREFIXES:
                    if ignored_prefix in prefix:
                        is_ignored = True
                        continue
                if is_ignored:
                    continue
                if cur_parser_label:
                    atomic_operations[cur_parser_label] = [EMPTY_MEMORY_ORDER]
                    cur_parser_label = 0
                rest_str = res.group(4)
                memory_order = obtain_memory_order(rest_str)
                if memory_order:
                    atomic_operations[index] = memory_order
                else:
                    cur_parser_label = index
            else:
                if cur_parser_label:
                    memory_order = obtain_memory_order(line)
                    if memory_order:
                        atomic_operations[cur_parser_label] = memory_order
                        cur_parser_label = 0
                    else:
                        if index - cur_parser_label >= MAX_OFFSET:
                            atomic_operations[cur_parser_label] = \
                                [EMPTY_MEMORY_ORDER]
                            cur_parser_label = 0
    if cur_parser_label:
        atomic_operations[cur_parser_label] = [EMPTY_MEMORY_ORDER]
    return atomic_comments, atomic_operations


def process_results(atomic_comments: dict, atomic_operations: dict) -> int:
    exit_code = 0
    for index, memory_orders in atomic_operations.items():
        for memory_order in memory_orders:
            is_commented = False
            for i in reversed(range(index - MAX_OFFSET, index)):
                commented_memory_order = atomic_comments.get(i)
                if memory_order == commented_memory_order or \
                        memory_order == EMPTY_MEMORY_ORDER and \
                        commented_memory_order == PARAMETERIZED_MEMORY_ORDER:
                    is_commented = True
                    if commented_memory_order == PARAMETERIZED_MEMORY_ORDER:
                        memory_order = PARAMETERIZED_MEMORY_ORDER
                    break
            if not is_commented:
                print("File {}:{}: reason for specific memory order was not "
                      "specified for atomic operation".format(FILE, index))
                exit_code = 1
            else:
                if memory_order == EMPTY_MEMORY_ORDER:
                    print("File {}:{}: memory order was not specified for "
                          "atomic operation".format(FILE, index))
                    exit_code = 1
    return exit_code


if __name__ == "__main__":
    COMMENTS, OPERATIONS = parse_file()
    sys.exit(process_results(COMMENTS, OPERATIONS))
