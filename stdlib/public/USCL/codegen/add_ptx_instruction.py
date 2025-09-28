#!/usr/bin/env python3
import argparse
from pathlib import Path

docs = Path("docs/libcudacxx/ptx/instructions")
test = Path("libcudacxx/test/libcudacxx/cuda/ptx")
src = Path("libcudacxx/include/cuda/__ptx/instructions")
ptx_header = Path("libcudacxx/include/cuda/ptx")
instr_docs = Path("docs/libcudacxx/ptx/instructions.rst")


def add_docs(ptx_instr, url):
    cpp_instr = ptx_instr.replace(".", "_")
    underbar = "=" * len(ptx_instr)

    (docs / f"{cpp_instr}.rst").write_text(
        f""".. _libcudacxx-ptx-instructions-{ptx_instr.replace(".", "-")}:

{ptx_instr}
{underbar}

-  PTX ISA:
   `{ptx_instr} <{url}>`__

.. include:: generated/{cpp_instr}.rst
"""
    )


def add_test(ptx_instr):
    cpp_instr = ptx_instr.replace(".", "_")
    dst = test / f"ptx.{ptx_instr}.compile.pass.cpp"
    dst.write_text(
        f"""/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */
// UNSUPPORTED: libcpp-has-no-threads

// <cuda/ptx>

#include <uscl/ptx>
#include <uscl/std/utility>

#include "generated/{cpp_instr}.h"

int main(int, char**)
{{
  return 0;
}}
"""
    )


def add_src(ptx_instr):
    cpp_instr = ptx_instr.replace(".", "_")
    (src / f"{cpp_instr}.h").write_text(
        f"""/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef _CUDA_PTX_{cpp_instr.upper()}_H_
#define _CUDA_PTX_{cpp_instr.upper()}_H_

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__ptx/ptx_dot_variants.h>
#include <uscl/__ptx/ptx_helper_functions.h>
#include <uscl/std/cstdint>

#include <nv/target> // __CUDA_MINIMUM_ARCH__ and friends

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_PTX

#include <uscl/__ptx/instructions/generated/{cpp_instr}.h>

_CCCL_END_NAMESPACE_CUDA_PTX

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_PTX_{cpp_instr.upper()}_H_
"""
    )


def add_ptx_header_include(ptx_instr):
    cpp_instr = ptx_instr.replace(".", "_")
    txt = ptx_header.read_text()
    # just add as first new include. clang-format will sort it in
    idx = txt.index("#include <uscl/__ptx/instructions")
    txt = (
        txt[:idx]
        + f"""#include <uscl/__ptx/instructions/{cpp_instr}.h>\n"""
        + txt[idx:]
    )
    ptx_header.write_text(txt)


def add_docs_include(ptx_instr):
    cpp_instr = ptx_instr.replace(".", "_")
    txt = instr_docs.read_text()
    # just add as first new include
    idx = txt.index("   instructions/")
    txt = txt[:idx] + f"   instructions/{cpp_instr}\n" + txt[idx:]
    instr_docs.write_text(txt)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("ptx_instruction", type=str)
    parser.add_argument("url", type=str)

    args = parser.parse_args()

    ptx_instr = args.ptx_instruction
    url = args.url

    # Enable using internal urls in the command-line, to be automatically converted to public URLs.
    if url.startswith("index.html"):
        url = url.replace(
            "index.html",
            "https://docs.nvidia.com/cuda/parallel-thread-execution/index.html",
        )

    add_test(ptx_instr)
    add_docs(ptx_instr, url)
    add_src(ptx_instr)
    add_ptx_header_include(ptx_instr)
    add_docs_include(ptx_instr)
