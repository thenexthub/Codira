#!/usr/bin/env bash
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
# Script for building documentation bundle from Sphinx *.rst documents
# TODO(igelhaus): Provision sphinx-build in the main bootstrap script.
#
# shellcheck disable=SC2086

set -e

SCRIPT_DIR="$(realpath "${0}")"
SCRIPT_DIR="$(dirname "${SCRIPT_DIR}")"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE=debug

# Build targets:
BUILD_COOKBOOK=no
BUILD_RUNTIME=no
BUILD_SPEC=no
BUILD_STDLIB=no
BUILD_TUTORIAL=no
BUILD_SYSTEM_ARKTS=no
BUILD_INTEROP_JS=no
BUILD_CONCURRENCY=no

CHECK_CODE=no

function print_help()
{
    local help_message="
ETS documentation builder

Tested and used under Ubuntu 20+ distributions. All dependencies are provisioned
by the main install-deps-ubuntu script, see its help for details.

SYNOPSIS

    [ENVIRONMENT VARIABLES] $0 [OPTIONS] [TARGETS]

ENVIRONMENT VARIABLES

    ARTIFACTS_DIR               See --build-dir command line option.

OPTIONS

    --help, -h                  Show this message and exit.

    --build-dir=[PATH]          Path to build directory. Default: ${BUILD_DIR}
                                If specified, supercedes ARTIFACTS_DIR.

    --build-type=debug|release  Build type, supported values:
                                * debug: Just build specified documents.
                                * release: Run extra checks and build.
                                Default is ${BUILD_TYPE}.

    --check-code                If this option is specified, the scipt verifies
                                correctness of code examples in the specification.

    --panda-path=[PATH]         Path to panda binaries needed to verify correctness
                                of code examples in the specification.


TARGETS

    A white-space separated list of documents to build. Supported stable
    targets (they are included into 'all' alias):

    * cookbook
    * runtime
    * spec
    * stdlib
    * tutorial
    * system ArkTS
    * interop_js
    * concurrency

    Following aliases are supported:

    * guides: cookbook stdlib tutorial system_arkts
    * all: Build all documents

    If no target is specified on the command line, 'all' is built.
    "
    echo "${help_message}"
}

function check_ubuntu_version()
{
    if [[ ! -f /etc/os-release ]]; then
        echo "FATAL: /etc/os-release not found"
        exit 1
    fi

    . /etc/os-release

    if [[ "${NAME}" != "Ubuntu" ]]; then
        echo "WARNING: OS name is ${NAME}, but only Ubuntu is supported at the moment"
    fi

    local major_version=$(echo "${VERSION_ID}" | cut -d. -f1)
    if [[ "${major_version}" -lt 20 ]]; then
        echo "WARNING: OS version is ${VERSION_ID}, but Ubuntu 20+ is required to run this script"
    fi
}

function build_sphinx_document()
{
    # NB! -j is not used intentionally, as rst2pdf.pdfbuilder is reported
    # to be unsafe for parallel writing under some platforms.
    local target="${1}"
    local src_dir="${2}"
    local build_options='-n -W --keep-going'

    echo "${target}: Building HTML"
    sphinx-build ${build_options} -b html "${src_dir}" "${BUILD_DIR}/${target}-html"

    # NB! Markdown for the spec is not skipped (mark-up too complex)
    if [[ "${target}" != "spec" ]] && [[ "${target}" != "concurrency" ]]; then
        echo "${target}: Building Markdown"
        sphinx-build ${build_options} -b markdown "${src_dir}" "${BUILD_DIR}/${target}-md"
        python3 "${SCRIPT_DIR}/merge_markdown.py" "${SCRIPT_DIR}" "${target}" "${BUILD_DIR}"
    fi

    echo "${target}: Building LaTeX and PDF"
    local build_dir_latex="${BUILD_DIR}/${target}-latex"
    sphinx-build ${build_options} -t ispdf -b latex "${src_dir}" "${build_dir_latex}"
    pushd "${build_dir_latex}"
        latexmk -f -silent -pdf -dvi- -ps- *.tex || true
        mv *.pdf "${BUILD_DIR}"
    popd
}

set -e

if [[ -n "${ARTIFACTS_DIR}" ]]; then
    echo "Detected ARTIFACTS_DIR, BUILD_DIR will be set to ${ARTIFACTS_DIR}"
    BUILD_DIR="${ARTIFACTS_DIR}"
fi

BUILD_SOMETHING=no
for i in "$@"; do
    case "$i" in
    -h|--help)
        print_help
        exit 0
        ;;
    --build-dir=*)
        BUILD_DIR=${i//[-a-z]*=/}
        ;;
    --build-type=*)
        BUILD_TYPE=${i//[-a-z]*=/}
        if ! [[ "${BUILD_TYPE}" =~ ^(debug|release)$ ]]; then
            echo "FATAL: Unknown build type ${BUILD_TYPE}. Supported values: debug, release"
            exit 1
        fi
        ;;

    --check-code)
        CHECK_CODE=yes
        ;;
    --panda-path=*)
        PANDA_ROOT="${i//[-a-z]*=/}/bin"
        export PANDA_ROOT
        ;;

    # Main build targets:

    cookbook)
        BUILD_SOMETHING=yes

        BUILD_COOKBOOK=yes
        ;;
    runtime)
        BUILD_SOMETHING=yes

        BUILD_RUNTIME=yes
        ;;
    spec)
        BUILD_SOMETHING=yes

        BUILD_SPEC=yes
        ;;
    stdlib)
        BUILD_SOMETHING=yes

        BUILD_STDLIB=yes
        ;;
    tutorial)
        BUILD_SOMETHING=yes

        BUILD_TUTORIAL=yes
        ;;
    system_arkts)
        BUILD_SOMETHING=yes

        BUILD_SYSTEM_ARKTS=yes
        ;;
    interop_js)
        BUILD_SOMETHING=yes

        BUILD_INTEROP_JS=yes
        ;;
    concurrency)
        BUILD_SOMETHING=yes

        BUILD_CONCURRENCY=yes
        ;;

    # Alias build targets:

    all)
        BUILD_SOMETHING=yes

        BUILD_COOKBOOK=yes
        BUILD_RUNTIME=yes
        BUILD_SPEC=yes
        BUILD_STDLIB=yes
        BUILD_TUTORIAL=yes
        BUILD_SYSTEM_ARKTS=yes
        BUILD_INTEROP_JS=yes
        BUILD_CONCURRENCY=yes
        ;;
    guides)
        BUILD_SOMETHING=yes

        BUILD_COOKBOOK=yes
        BUILD_STDLIB=yes
        BUILD_TUTORIAL=yes
        BUILD_SYSTEM_ARKTS=yes
        BUILD_INTEROP_JS=yes
        ;;

    *)
        echo "Error: Unknown option $i" >&2
        exit 1
        ;;
    esac
done

if [[ "${BUILD_SOMETHING}" == "no" ]] ; then
    BUILD_SOMETHING=yes

    BUILD_COOKBOOK=yes
    BUILD_RUNTIME=yes
    BUILD_SPEC=yes
    BUILD_STDLIB=yes
    BUILD_TUTORIAL=yes
    BUILD_SYSTEM_ARKTS=yes
    BUILD_INTEROP_JS=yes
    BUILD_CONCURRENCY=yes
fi

check_ubuntu_version

mkdir -p "${BUILD_DIR}"

if [[ "${CHECK_CODE}" == "yes" ]]; then
    echo "Checking code examples validity"
    if [[ ! -v PANDA_ROOT ]]; then
        echo "Error: Panda directory is not set"
        exit 1
    fi

    if [[ "${BUILD_CONCURRENCY}" == "yes" ]]; then
        python3 $(dirname "$SCRIPT_DIR")/tools/specification_checker.py --spec-folder "${SCRIPT_DIR}/concurrency"
    fi
fi

if [[ "${BUILD_SPEC}" == "yes" ]]; then
    echo "spec: Validating ${SCRIPT_DIR}/spec"
    cp ${SCRIPT_DIR}/concurrency/*.plantuml ${SCRIPT_DIR}/spec/
    python3 "${SCRIPT_DIR}/validate_spec.py" "${SCRIPT_DIR}/spec"
    build_sphinx_document spec "${SCRIPT_DIR}/spec"
    rm -f ${SCRIPT_DIR}/spec/*.plantuml
fi

if [[ "${BUILD_COOKBOOK}" == "yes" ]]; then
    echo "cookbook: Validating ${SCRIPT_DIR}/cookbook/recipes.rst"
    VALIDATE_RECIPES_OPTIONS=""
    if [[ "${BUILD_TYPE}" == "release" ]]; then
        VALIDATE_RECIPES_OPTIONS="${VALIDATE_RECIPES_OPTIONS} --check-code"
    fi

    "${SCRIPT_DIR}/validate-recipes" \
        ${VALIDATE_RECIPES_OPTIONS} \
        --recipes="${SCRIPT_DIR}/cookbook/recipes.rst"

    build_sphinx_document cookbook "${SCRIPT_DIR}/cookbook"
fi

if [[ "${BUILD_RUNTIME}" == "yes" ]]; then
    build_sphinx_document runtime "${SCRIPT_DIR}/runtime"
fi

if [[ "${BUILD_STDLIB}" == "yes" ]]; then
    build_sphinx_document stdlib "${SCRIPT_DIR}/stdlib"
fi

if [[ "${BUILD_TUTORIAL}" == "yes" ]]; then
    build_sphinx_document tutorial "${SCRIPT_DIR}/tutorial"
fi

if [[ "${BUILD_SYSTEM_ARKTS}" == "yes" ]]; then
    build_sphinx_document system_arkts "${SCRIPT_DIR}/system_arkts"
fi

if [[ "${BUILD_INTEROP_JS}" == "yes" ]]; then
    build_sphinx_document interop_js "${SCRIPT_DIR}/interop_js"
fi

if [[ "${BUILD_CONCURRENCY}" == "yes" ]]; then
    build_sphinx_document concurrency "${SCRIPT_DIR}/concurrency"
fi


echo "Build succeeded, please find documents in ${BUILD_DIR}"
