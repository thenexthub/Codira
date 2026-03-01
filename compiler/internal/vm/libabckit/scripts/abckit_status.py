#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import re
import os
import argparse
import logging

script_dir = os.path.dirname(os.path.realpath(__file__))

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


def get_args():
    parser = argparse.ArgumentParser(description='Abckit status script')
    parser.add_argument('--print-implemented',
                        action='store_true',
                        default=False,
                        help=f'Print list of implemented API and exit')
    parser.add_argument('--cppapi',
                        action='store_true',
                        default=False,
                        help=f'Fill table with tests for cpp api')
    parser.add_argument('--capi',
                        action='store_true',
                        default=False,
                        help=f'Fill table with tests for c api')
    return parser.parse_args()


args = get_args()

API_PATTERN = r'^[\w,\d,_, ,*]+\(\*([\w,\d,_]+)\)\(.*'
CPP_API_PATTERN = r'.+ \**\&*[\w\d]+\('
PUBLIC_API_PATTERN = r'^(?!explicit)(.+) (?!~)[\&\*]*(?!operator)(.+)\(.*'
domain_patterns = [
    r'struct Abckit\S*Api\s\{', r'struct Abckit\S*ApiStatic\s\{',
    r'struct Abckit\S*ApiDynamic\s\{'
]
specs = ['public:', 'private:', 'protected:']

libabckit_dir = script_dir.rsplit('/', 1)[0]
abckit_tests = os.path.join(libabckit_dir, 'tests')

c_sources = {'include/libabckit/c'}

c_tests = {
    'tests/canary', 'tests/helpers', 'tests/internal', 'tests/null_args_tests',
    'tests/sanitizers', 'tests/scenarios', 'tests/scenarios_c_api_clean',
    'tests/stress', 'tests/ut tests/wrong_ctx_tests', 'tests/wrong_mode_tests'
}

cpp_sources = {'include/libabckit/cpp'}

cpp_tests = {
    'tests/cpp/tests',
    'tests/mock',
    'tests/regression',
    'tests/scenarios_cpp_api_clean'
}

TS = 'TS'
JS = 'JS'
ARKTS1 = 'ArkTS1'
ARKTS2 = 'ArkTS2'
NO_ABC = 'NoABC'


def check_test_anno_line(line):
    mul_lines = False
    anno_line = False
    if re.fullmatch(r'^\/\/ Test: test-kind=.*\n', line):
        anno_line = True
        mul_lines = line.strip()[-1] == ','
    return {'is_annotation_line': anno_line, 'mul_annotation_lines': mul_lines}


def get_full_annotation(it, annotation_start):
    annotation = annotation_start.strip()
    next_anno_line = True
    while next_anno_line:
        new_line = next(it)
        annotation += new_line.replace('//', '').strip()
        if not re.fullmatch(r'^\/\/.*,$', new_line):
            next_anno_line = False
    return annotation


class Test:

    def __init__(self, s):
        err = f'Wrong test annotation: "{s}"'

        self.kind = ''
        self.abc_kind = ''
        self.api = ''
        self.category = ''
        self.extension = ''

        check('// Test:' in s, err)
        s = s.replace('// Test:', '')
        entries = [x.strip() for x in s.split(',') if x]
        for entry in entries:
            key, value = entry.strip().split('=')
            if key == 'test-kind':
                check(value in ['api', 'scenario', 'internal', 'mock', 'regression'], err)
                self.kind = value
            elif key == 'abc-kind':
                check(value in [ARKTS1, ARKTS2, JS, TS, NO_ABC], err)
                self.abc_kind = value
            elif key == 'api':
                self.api = value
            elif key == 'category':
                possible_values = [
                    'positive', 'negative-mode', 'negative-nullptr',
                    'negative-file', 'internal', 'negative'
                ]
                check(value in possible_values, err)
                self.category = value
            elif key == 'extension':
                self.extension = value
            else:
                check(False, f'Wrong key: {key}')

        check(self.kind, err)
        check(self.extension, err)
        check(self.abc_kind, err)
        check(self.category, err)
        if self.kind == 'api' or self.kind == 'mock':
            check(self.api, err)


def is_first_test_line(line):
    if re.fullmatch(r'TEST_F\(.*,.*\n', line):
        return True
    return False


def get_test_from_annotation(api, annotation):
    test = Test(annotation)
    if ('api=' in annotation
            or 'mock=' in annotation) and test.api != 'ApiImpl::GetLastError':
        check(test.api in api, f'No such API: {test.api}')
    return test


def collect_tests_from_path(path, api):
    with open(path, 'r') as f:
        lines = f.readlines()
        it = iter(lines)
        tests = []

        ano_count, test_count = 0, 0
        annotation = ''
        line = '\n'
        while True:
            line = next(it, None)
            if (line is None):
                break

            if is_first_test_line(line):
                test_count += 1
                check(test_count <= ano_count,
                      f'Test has no annotation:\n{path}\n{line}\n')
                continue

            info = check_test_anno_line(line)
            if not info.get('is_annotation_line'):
                annotation = ''
                continue
            ano_count += 1
            annotation += line.strip()

            if info.get('mul_annotation_lines'):
                annotation = get_full_annotation(it, line)
            else:
                annotation = line.strip()

            if annotation:
                tests.append(get_test_from_annotation(api, annotation))

    check(ano_count == test_count, f'Annotation without test:\n{path}\n')
    return tests


def collect_tests(path, api):
    tests = []
    for dirpath, _, filenames in os.walk(f'{libabckit_dir}/{path}'):
        for name in filenames:
            if name.endswith('.cpp'):
                tests += collect_tests_from_path(os.path.join(dirpath, name),
                                                 api)
    return tests


def get_tests_statistics(api, tests):
    for test in tests:
        if (test.kind != 'api' and test.kind
                != 'mock') or test.api == 'ApiImpl::GetLastError':
            continue
        if test.abc_kind == ARKTS1:
            api[test.api].arkts1_tests += 1
        if test.abc_kind == ARKTS2:
            api[test.api].arkts2_tests += 1
        elif test.abc_kind == JS:
            api[test.api].js_tests += 1
        elif test.abc_kind == TS:
            api[test.api].ts_tests += 1
        elif test.abc_kind == NO_ABC:
            api[test.api].no_abc_tests += 1

        if test.category == 'positive':
            api[test.api].positive_tests += 1
            if test.abc_kind == TS or test.abc_kind == JS or test.abc_kind == ARKTS1:
                api[test.api].dynamic_positive_tests += 1
            if test.abc_kind == ARKTS2:
                api[test.api].static_positive_tests += 1
        elif test.category == 'negative':
            api[test.api].negative_tests += 1
        elif test.category == 'negative-nullptr':
            api[test.api].negative_nullptr_tests += 1
        elif test.category == 'negative-file':
            api[test.api].negative_ctx_tests += 1
        elif test.category == 'negative-mode':
            api[test.api].negative_mode_tests += 1
        else:
            api[test.api].other_tests += 1
    return api


def check(cond, msg=''):
    if not cond:
        raise Exception(msg)


class API:

    def __init__(self, name, domain, sig='', extension=''):
        self.name = name
        self.domain = domain
        self.sig = sig
        self.extension = extension
        self.dynamic_positive_tests = 0
        self.static_positive_tests = 0
        self.arkts1_tests = 0
        self.arkts2_tests = 0
        self.js_tests = 0
        self.ts_tests = 0
        self.no_abc_tests = 0
        self.positive_tests = 0
        self.negative_tests = 0
        self.negative_nullptr_tests = 0
        self.negative_ctx_tests = 0
        self.negative_mode_tests = 0
        self.other_tests = 0


def next_line(it):
    return next(it).strip()


def determine_doclet(it, line):
    if not line or line.strip() != "/**":
        return ''

    line = next_line(it)
    funс_declaration = ''

    while not funс_declaration:
        if line and line == '*/':
            l = next_line(it)
            if 'template <' in l:
                return next_line(it)
            return l
        line = next_line(it)


def check_oop(sign):
    if ' = default' in sign or ' = delete' in sign:
        if any([x in sign for x in [' = default', ' = delete', ' : ', 'operator']]):
            return False
    return True


def cut_cpp_sign(sign):
    name_with_return = re.search(r'[^\s] [^\(]+', sign.strip()).group(0)
    name = re.search(r' [^\s]([\w\d]+)$', name_with_return).group(0).strip()
    return re.search(r'[^\**\&*]([\w\d]+)$', name).group(0).strip()


def collect_api(path, extension):
    apis = {}
    domain = ''
    with open(path, 'r') as f:
        accsess = 'public:'
        lines = f.readlines()
    it = iter(lines)

    while True:
        if (line := next(it, None)) is None:
            break
        line = line.strip()

        if line in specs:
            accsess = line

        signature = determine_doclet(it, line)
        api_name = ''
        if not signature:
            continue

        if re.search(r'struct Abckit(.*)Api(.*)\s\{', signature):
            domain = re.search(r'struct Abckit(.*)\s\{',
                               signature.strip()).group(1)
            domain = f'{domain}Impl'

        elif re.search(r'struct (.+) Abckit(.*)Api(.*)\s\{', signature):
            domain = re.search(r'struct (.+) Abckit(.*)\s\{', signature.strip()).group(2)
            domain = f'{domain}Impl'

        elif re.match(r'class .+ {', signature):
            if match := re.search(r'class (.+) .+ : .+ {', signature,
                                  re.IGNORECASE):
                domain = match.group(1)
            elif match := re.search(r'class (.+) : .+ {', signature,
                                    re.IGNORECASE):
                domain = match.group(1)
            elif match := re.search(r'class (.+) (.+){', signature,
                                    re.IGNORECASE):
                domain = match.group(1)
            elif match := re.search(r'class (.+) {', signature, re.IGNORECASE):
                domain = match.group(1)

        elif (re.match(PUBLIC_API_PATTERN, signature)) and accsess == "public:":
            if re.match(API_PATTERN, signature.strip()) and check_oop(signature):
                api_name = re.search(r'^[\w,\d,_, ,*]+\(\*([\w,\d,_]+)\)\(.*', signature.strip()).group(1)
            elif re.match(CPP_API_PATTERN, signature.strip()) and check_oop(signature):
                api_name = cut_cpp_sign(signature)
            else:
                continue
            apis[f'{domain}::{api_name}'] = API(api_name, domain, signature, extension)
    return apis


def collect_api_from_sources(sources, extension):
    apis = {}
    for src in sources:
        for (dirpath, _, filenames) in os.walk(f'{libabckit_dir}/{src}'):
            headers = list(
                filter(lambda f: re.fullmatch(r'(.+)(?<!_impl).h$', f),
                       filenames))
            for file in headers:
                apis = dict(apis.items() | collect_api(
                    os.path.join(dirpath, file), extension).items())
    return apis


def api_test_category(tests, name):
    return len(
        list(
            filter(lambda t: t.kind == 'api' and t.category == name,
                   tests)))


def api_lang(tests, name):
    return len(
        list(
            filter(lambda t: t.kind == 'api' and t.abc_kind == name,
                   tests)))


def scenario_lang(tests, name):
    return len(
        list(
            filter(lambda t: t.kind == 'scenario' and t.abc_kind == name,
                   tests)))


def print_cppapi_stat(tests_pathes, api, expected=0):
    tests = []
    for p in tests_pathes:
        tests += list(
            filter(lambda t: t.extension == 'cpp', collect_tests(p, api)))
    api = get_tests_statistics(api, tests)

    def api_tests_kind(kind):
        return list(filter(lambda t: t.kind == kind, tests))

    def get_element(array, i):
        if len(array) <= i:
            return '-'
        return array[i]

    mock_tests_apis = list(dict.fromkeys(t.api
                                         for t in api_tests_kind("mock")))
    api_tests_apis = list(dict.fromkeys(t.api for t in api_tests_kind("api")))
    internal_tests = list(filter(lambda t: t.kind == 'internal', tests))
    regression_tests = list(filter(lambda t: t.kind == 'regression', tests))

    csv = ''
    for i in range(max(len(mock_tests_apis), len(api_tests_apis))):
        csv += f'{get_element(mock_tests_apis, i)},{get_element(api_tests_apis, i)}\n'
    with os.fdopen(os.open(os.path.join(libabckit_dir, 'scripts/abckit_cppapi_status.csv'),
                           os.O_CREAT | os.O_WRONLY | os.O_TRUNC, 0o755), 'w+') as f:
        f.write(('mock_tests_apis,api_tests_apis\n'))
        f.write(csv)

    logging.debug(f'>>> CPP EXTENSION <<<\n')

    logging.debug(
        'Total API:                                              %s/%s',
        len(api), expected)
    logging.debug('')
    logging.debug('Total Tests:                                            %s',
                  len(tests))
    logging.debug('')
    logging.debug('Total API\'S with api tests:                             %s/%s',
                  len(api_tests_apis), expected)
    logging.debug('Total API\'S with mock tests:                            %s/%s',
                  len(mock_tests_apis), expected)
    logging.debug('Total internal tests:                                     %s',
                  len(internal_tests))
    logging.debug('Total regression tests:                                   %s',
                  len(regression_tests))
    logging.debug('Total scenario tests:                                     %s',
                  len(list(filter(lambda t: t.kind == 'scenario', tests))))
    logging.debug(
        'ArkTS1/ArkTS2/JS/TS scenario tests:                     %s/%s/%s/%s',
        scenario_lang(tests, ARKTS1), scenario_lang(tests, ARKTS2), scenario_lang(tests, JS),
        scenario_lang(tests, TS))
    logging.debug(f'\n------------------------------------------------------------------\n')


def print_capi_stat(tests_pathes, api):
    csv = ''
    tests = []
    for p in tests_pathes:
        tests += list(
            filter(lambda t: t.extension == 'c', collect_tests(p, api)))
    api = get_tests_statistics(api, tests)
    for name in api:
        csv += (
            f'{name},{api[name].extension},{api[name].dynamic_positive_tests},{api[name].static_positive_tests},'
            f'{api[name].arkts1_tests},{api[name].arkts2_tests},{api[name].js_tests},{api[name].ts_tests},'
            f'{api[name].no_abc_tests},{api[name].positive_tests},{api[name].negative_tests},'
            f'{api[name].negative_nullptr_tests},{api[name].negative_ctx_tests},{api[name].other_tests}\n'
        )

    with os.fdopen(os.open(os.path.join(libabckit_dir, 'scripts/abckit_capi_status.csv'),
                           os.O_CREAT | os.O_WRONLY | os.O_TRUNC, 0o755), 'w+') as f:
        f.write(('api,extension,dynamic_positive_tests,static_positive_tests,'
                 'arkts1_tests,arkts2_tests,js_tests,ts_tests,no_abc_tests,positive_tests,'
                 'negative_tests,negative_nullptr_tests,negative_ctx_tests,other_tests\n'))
        f.write(csv)

    log_capi_stat(api, tests)


def log_capi_stat(api, tests):

    logging.debug('>>> C <<<\n')

    logging.debug('Total API:                                              %s',
                  len(api))
    logging.debug('')
    logging.debug('Total Tests:                                            %s',
                  len(tests))
    logging.debug('')
    logging.debug('Total API tests:                                        %s',
                  len(list(filter(lambda t: t.kind == 'api', tests))))
    logging.debug(
        'Positive/Negative/NullArg/WrongCtx/WrongMode API tests: %s/%s/%s/%s/%s',
        api_test_category(tests, 'positive'), api_test_category(tests, 'negative'),
        api_test_category(tests, 'negative-nullptr'),
        api_test_category(tests, 'negative-file'), api_test_category(tests, 'negative-mode'))
    logging.debug(
        'ArkTS1/ArkTS2/JS/TS/NoABC API tests:                    %s/%s/%s/%s/%s',
        api_lang(tests, ARKTS1), api_lang(tests, ARKTS2), api_lang(tests, JS), api_lang(tests, TS),
        api_lang(tests, NO_ABC))
    logging.debug('')
    logging.debug('Total scenario tests:                                   %s',
                  len(list(filter(lambda t: t.kind == 'scenario', tests))))
    logging.debug(
        'ArkTS1/ArkTS2/JS/TS scenario tests:                     %s/%s/%s/%s',
        scenario_lang(tests, ARKTS1), scenario_lang(tests, ARKTS2), scenario_lang(tests, JS),
        scenario_lang(tests, TS))
    logging.debug('')
    logging.debug('Internal tests:                                         %s',
                  len(list(filter(lambda t: t.kind == 'internal', tests))))
    logging.debug(
        f'\n------------------------------------------------------------------\n'
    )


cpp_api, c_api = {}, {}
if args.cppapi:
    cpp_api = dict(cpp_api.items()
                   | collect_api_from_sources(cpp_sources, 'cpp').items())
c_api = dict(c_api.items() | collect_api_from_sources(c_sources, 'c').items())

if args.print_implemented:
    if args.cppapi:
        logging.debug('\n'.join(cpp_api))
    if args.capi:
        logging.debug('\n'.join(c_api))
else:
    if args.cppapi:
        print_cppapi_stat(cpp_tests, cpp_api, len(c_api))
    if args.capi:
        print_capi_stat(c_tests, c_api)
