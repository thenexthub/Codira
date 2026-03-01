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

set -e

args=$(getopt -a -o s:b:r --long spec:,build:,rst: -- "$@")

if [ $? -ne 0 ]; then
    help
    exit 1
fi
eval set -- "${args}"

while :; do
    case "$1" in
    -b | --build)
        export BUILD=$2
        shift 2
        ;;

    -s | --spec)
        export SPEC=$2
        shift 2
        ;;

    -r | --rst)
        export RST_FILE=$2
        shift 2
        ;;

    -h | --help)
        help
        shift
        ;;

    - | --)
        shift
        break
        ;;
    esac

done

OK_COLOR=107
WARNING_COLOR=11



function echo_color_text() {
    COLOR_CODE=$1
    if [[ "$3" ]]; then
        tabs="\t";
    else
        tabs="\n"; fi

    TEXT=$2
    C="\033[38;5;${COLOR_CODE}m"
    NC='\033[0m' # No Color
    printf "${C}${TEXT}${NC}${tabs}"
}

es2panda=$BUILD/bin/es2panda
# echo $es2panda
if ! test -f "$BUILD"; then
    echo_color_text $WARNING_COLOR "please check your path to es2panda: \n $es2panda"
    exit; fi

if [ "$SPEC" ] && [ "$RST_FILE" ]; then
    echo_color_text $WARNING_COLOR "please specify correctly"
    exit
fi

rm -rf snippets; rm -rf main_results; rm -rf results
mkdir snippets; mkdir snippets/abc; mkdir results
touch snippets/_output; touch results/main_results
chmod a+rwx -R snippets

if [[ "$SPEC" ]]; then
    python3 verify.py --spec="$SPEC" 2> ./.verifier_error; fi
if [[ "$RST_FILE" ]]; then
    python3 verify.py --rst="$RST_FILE" 2> ./.verifier_error; fi

if [[ $(cat ./.verifier_error) ]]; then
    echo_color_text $WARNING_COLOR "please fix the snippets meta or check error in .verifier_error"
    exit
fi
rm -rf ./.verifier_error

snippets=$(pwd)/snippets


function check_results_md_file() {
    if test -f results/"$1".md; then return; fi
    touch results/"$1".md
    echo "<table><tr><th>snippet</th><th>frontend_status</th><th>expect_compile</th> \
        <th>actual_compile</th><th>expect_subset</th><th>actual_subset</th></tr><tr>" >> results/"$1".md
}

function write_md_results() {
    check_results_md_file "$theme"

    echo "<td> <details><summary>$2</summary><pre><code class=typescript>">> results/"$1".md
    cat snippets/"$2".ets >> results/"$1".md
    echo "</td></code></pre></details>" >> results/"$1".md
    echo "<td> $3 </td><td> $4 </td><td> $5 </td><td> $6 </td><td> $7 </th></tr>" >> results/"$1".md
}

function write_results() {
    name=$1
    if [ "$2" = 1 ]; then
        expected_subset="subset"
    else expected_subset="not_subset"; fi

    if [ "$3" = 1 ]; then
        actual_subset="subset"
    else actual_subset="not_subset"; fi

    if [ "$4" = 1 ]; then
        expected_compile="compile"
    else expected_compile="not_compile"; fi

    if [ "$5" = 1 ]; then
        actual_compile="compile"
    else actual_compile="not_compile"; fi

    frontend_status=$6

    if [ "$4" = 0 ]; then
        expected_subset="na"
        actual_subset="na";
    fi

    theme=${name%_*}

    echo "$theme,$name,$frontend_status,$expected_compile,$actual_compile,$expected_subset,$actual_subset" >> results/main_results
    write_md_results "$theme" "$name" "$frontend_status" "$expected_compile" "$actual_compile" "$expected_subset" "$actual_subset"
}

function check() {
    ets_count=$(ls "$snippets"/*.ets 2> /dev/null | wc -l)
    if [ "$ets_count" = 0 ]; then
        echo_color_text $OK_COLOR "There is no snippets in $RST_FILE $SPEC :)"
        exit
    fi
    chmod a+x "$snippets"
    for snippet in $snippets/*.ets; do
        snippet_name=$(echo "${snippet##*/}" | cut -d "." -f 1) # "/../../../test.ets" -> "test"

        # echo "$snippet_name"
        snippet_ets=$snippets/$snippet_name.ets
        snippet_ts=$snippets/$snippet_name.ts

        expect_cte=$(sed -n '1p' "$snippet_ets")
        frontend_status_comment=$(sed -n '2p' "$snippet_ets")
        expect_subset=$(sed -n '3p' "$snippet_ets")

        frontend_status_formated=${frontend_status_comment##* }
        expect_status=1
        expect_subset_status=1

        if [[ "$expect_cte" = "// cte" ]]; then
            expect_status=0
        fi
        if [[ "$expect_subset" = "// ns" ]]; then
            expect_subset_status=0
        fi

        ets_compile_return=$($es2panda "$snippet_ets" --output="$snippets"/abc/"$snippet_name".abc)
        echo -e "\n\n$snippet_name\n" >> "snippets/_output"
        echo "$ets_compile_return" >> snippets/_output

        if test -f "$snippet_ts"; then
            ts_compile_return=$(tsc "$snippet_ts" --outFile "$snippets"/abc/"$snippet_name".json)
            echo "$ts_compile_return" >> snippets/_output
        fi

        ets_compile_status=1; ts_compile_status=1
        if [[ "$ets_compile_return" != '' ]]; then
            ets_compile_status=0; fi
        if [[ "$ts_compile_return" != '' ]]; then
            ts_compile_status=0; fi

        ts_subset=$((! $ets_compile_status ^ $ts_compile_status))
        # ets_compile=$((! $ets_compile_status))

        write_results "$snippet_name" $expect_subset_status $ts_subset $expect_status $ets_compile_status "$frontend_status_formated"

    done
    for md_result in results/*.md; do
        echo "</table>" >> "$md_result"
    done
}
check
