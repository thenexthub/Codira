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

import os
import argparse
import re
from copy import deepcopy
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--spec", dest="spec", default="", help="path to spec")
parser.add_argument("--rst", dest="rst_file", default="", help="path to .rst file")

args = parser.parse_args()
spec_dir = os.fsencode(args.spec.strip())
rst_ = os.fsencode(args.rst_file.strip())


default_snippet_tags = {
    "marked": False,
    "name": '',
    "first_line_ind": 0,
    "skip": False,
    "not-subset": False,
    "expect-cte": False,
    "expect_output": [],
    "unrecognized tags": [],
    "part": 0,
}


def get_space_len(line):
    length = 0
    for i in line:
        if i.isspace():
            length += 1
        else:
            return length
    return 10**10


def write_snippet_line(line, space_len, ets_file, ts_file=None):
    if not line[0].isspace():
        return
    if line.isspace():
        return
    indent_cropped_line = line[space_len:]

    ets_file.write(indent_cropped_line)
    if ts_file:
        ts_file.write(indent_cropped_line)
    return


def write_code_meta(expect_cte, frontend_status, expect_subset, file):
    file.write("//" + expect_cte + "\n")
    file.write("//" + frontend_status + "\n")
    file.write("//" + expect_subset + "\n")


def write_snippet(rst_lines, snippet_name, snippet_meta, previous_snippet_name, frontend_statuses):
    sm = snippet_meta
    if sm["skip"]:
        return previous_snippet_name

    for ind in frontend_statuses:
        if sm["first_line_ind"] > ind:
            frontend_status = frontend_statuses[ind]["status"]


    if int(snippet_meta.get("part")) >= 1 and previous_snippet_name:
        current_snippet_name = previous_snippet_name
    else:
        current_snippet_name = snippet_name

    expect_cte = "cte" if snippet_meta["expect-cte"] else "sc"
    expect_subset = "ns" if sm["not-subset"] else ""
    snippet_ets = os.fdopen(os.open("snippets/" + current_snippet_name + ".ets", os.O_WRONLY, 0o755), "w+")
    write_code_meta(expect_cte, frontend_status, expect_subset, snippet_ets)

    if not snippet_meta["not-subset"] and not snippet_meta["expect-cte"]:
        snippet_ts = os.fdopen(os.open("snippets/" + current_snippet_name + ".ts", os.O_WRONLY, 0o755), "w+")
        write_code_meta(expect_cte, frontend_status, expect_subset, snippet_ts)

    else:
        snippet_ts = None

    # + 2 because of ":lineos:" (Varvara, do some smarter thing pls)
    space_len = 10**10
    for line in rst_lines[sm["first_line_ind"] + 2 :]:
        if not line[0].isspace():
            break
        space_len = min(get_space_len(line), space_len)
        write_snippet_line(line, space_len, snippet_ets, snippet_ts)

    snippet_ets.close()
    if snippet_ts:
        snippet_ts.close()
    return current_snippet_name


def check_tag(trailed_line):
    if trailed_line in ["skip", "not-subset"]:
        return "flag"
    if re.fullmatch("part\d+", trailed_line):
        return "part"
    if re.match("name: [a-z]+", trailed_line):
        return "name"
    if re.fullmatch("expect-cte:{0,1}", trailed_line):
        return "cte"
    return "unrecognized tag"


def add_tag(trailed_line, snippet_tags):
    tag = check_tag(trailed_line)
    if tag == "flag":
        snippet_tags[trailed_line] = True
    elif tag == "part":
        snippet_tags["part"] = re.findall(r"\d+", trailed_line)[0]
    elif tag == "cte":
        snippet_tags["expect-cte"] = True
    elif tag == "unrecognized tag":
        snippet_tags["unrecognized tags"].append(trailed_line)
    elif tag == "name":
        snippet_tags["name"] = trailed_line.replace('name: ', '')


def parse_snippet_meta(meta_block_ind, rst_lines, snippets_meta):
    snippet_tags = deepcopy(default_snippet_tags)
    snippet_tags["marked"] = True

    for ind, line in enumerate(rst_lines[meta_block_ind + 1 :]):
        if "code-block:: typescript" in line:
            first_line_ind = ind + meta_block_ind + 1
            snippet_tags["first_line_ind"] = first_line_ind
            snippets_meta[first_line_ind] = snippet_tags
            return snippet_tags

        trailed_line = line.strip()
        if trailed_line:
            add_tag(trailed_line, snippet_tags)
    return snippet_tags


def parse_frontend_meta(rst_lines):
    theme_indices = [
        i for i, x in enumerate(rst_lines) if re.fullmatch(r"[=]+", x.strip())
            or re.fullmatch(r"[\*]+", x.strip())
            or re.fullmatch(r"[\#]+", x.strip())
            or re.fullmatch(r"[-]+", x.strip())
    ]
    theme_indices = [0] + theme_indices + [len(rst_lines)]

    frontend_statuses = {}
    for i in range(len(theme_indices) - 1):
        frontend_status_line = [
            rst_lines[j].split(":")[1].strip()
            for j in range(theme_indices[i], theme_indices[i + 1])
            if re.match(r"\s*frontend_status:.*", rst_lines[j])
        ]
        frontend_statuses[theme_indices[i]] = {"end_theme" : theme_indices[i + 1],
                                               "status" : frontend_status_line[0]
            if frontend_status_line else "Partly"}

    return frontend_statuses


def print_error(mark, filename, line_idx):
    print("{mark} {fname}::{line_idx} {mark}".format(
            fname=filename, line_idx=line_idx, mark=mark), end=' ')


def check_name(names, name, filename, i, correct_tags):
    if name in names:
        print_error("***", filename, i)
        print("Snippet name in not unique")
        return False
    else:
        names[name] = True
        return correct_tags


def check_snippets_meta(filename, snippets_meta):
    correct_tags = True
    names = {}
    for i in snippets_meta:
        sm = snippets_meta[i]
        if not sm['marked']:
            print_error("!!!", filename, i)
            print("No testing options for snippet")
            correct_tags = False
            continue

        if sm["expect-cte"] and sm["not-subset"]:
            print_error("???", filename, i)
            print("Incorrect options set. Please check meta docs")
            correct_tags = False

        if sm["unrecognized tags"]:
            print_error("!!!", filename, i)
            print("Unrecognized tags:", ' ,'.join(sm["unrecognized tags"]))
            correct_tags = False

        if sm["name"]:
            correct_tags = check_name(names, sm['name'],
                filename, i, correct_tags)
    return correct_tags


def write_snippets_from_rst(rst_lines, filename, skiplist):
    frontend_statuses = parse_frontend_meta(rst_lines)

    code_block_indices = [
        i for i, x in enumerate(rst_lines) if x.strip() == ".. code-block:: typescript"
    ]
    meta_block_indices = [i for i, x in enumerate(rst_lines) if x.strip() == ".. code-block-meta:"]
    snippets_meta = {int(i): deepcopy(default_snippet_tags) for i in code_block_indices}
    for i in snippets_meta:
        snippets_meta[i]["first_line_ind"] = i

    for i in meta_block_indices:
        parse_snippet_meta(i, rst_lines, snippets_meta)
    previous_snippet_name = ""

    if not check_snippets_meta(filename, snippets_meta):
        return False

    for i in code_block_indices:
        if snippets_meta[i]['name'] in skiplist:
            continue
        snippet_name = "{fname}_{first_line_idx}".format(
            fname=filename, first_line_idx=i
        )
        previous_snippet_name = write_snippet(
            rst_lines, snippet_name, snippets_meta.get(i),
            previous_snippet_name, frontend_statuses
        )
    return True


def parse_skiplist():
    with open('skiplist', 'r') as skiplist:
        global SKIP_NAMES
        SKIP_NAMES = skiplist.readlines()
    return SKIP_NAMES


def parse_dir(skiped_names):
    result = True
    for file in os.listdir(spec_dir):
        if not parse_file(skiped_names, file, os.path.join(spec_dir, file)):
            result = False
            continue
        parse_file(skiped_names, file, os.path.join(spec_dir, file))
    return result


def parse_file(skiped_names, file, path=""):
    filename = os.fsdecode(file)
    result = True
    if filename.endswith(".rst"):
        with open(path) as rst_file:
            if not write_snippets_from_rst(rst_file.readlines(), os.path.splitext(filename)[0], skiped_names):
                result = False
            write_snippets_from_rst(rst_file.readlines(), os.path.splitext(filename)[0], skiped_names)
    return result

SKIP_NAMES = parse_skiplist()

if args.rst_file:
    if not parse_file(SKIP_NAMES, os.path.basename(args.rst_file), rst_):
        raise NameError("incorrect snippets meta")
    parse_file(SKIP_NAMES, os.path.basename(args.rst_file), rst_)
if args.spec:
    if not parse_dir(SKIP_NAMES):
        raise NameError("incorrect snippets meta")
    parse_dir(SKIP_NAMES)

if args.rst_file:
    parse_file(SKIP_NAMES, os.path.basename(args.rst_file), rst_)
if args.spec:
    parse_dir(SKIP_NAMES)
