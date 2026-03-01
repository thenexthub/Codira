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

import argparse
import os
import re
import sys
import traceback
from clade import Clade
from clang.cindex import Index
from clang.cindex import TokenKind
from clang.cindex import CursorKind
from clang.cindex import TranslationUnitLoadError
import yamale
import yaml

BINDINGS_FILE_NAME = "bindings.yml"

# File describing structure of bindings.yml files and other restrictions on their content
# This file is used to validate bindings.yml with yamale module
BINDINGS_SCHEMA_FILE = '%s/%s' % (os.path.dirname(os.path.realpath(__file__)), "bindings_schema.yml")

CPP_CODE_TO_GET_TEST_ID_IN_PANDA = "IntrusiveTest::GetId()"
CPP_HEADER_COMPILE_OPTION_LIST = ["-x", "c++-header"]
CLANG_CUR_WORKING_DIR_OPT = "-working-directory"
HEADER_FILE_EXTENSION = ".h"


# Parser of arguments and options
class ArgsParser:

    def __init__(self):
        parser = argparse.ArgumentParser(description="Source code instrumentation for intrusive testing")
        parser.add_argument(
            'src_dir',
            metavar='SRC_DIR',
            nargs=1,
            type=str,
            help="Directory with source code files for instrumentation")

        parser.add_argument(
            'clade_file',
            metavar='CLADE_FILE',
            nargs=1,
            type=str,
            help="File with build commands intercepted by 'clade' tool")

        parser.add_argument(
            'test_suite_dir',
            metavar='TEST_SUITE_DIR',
            nargs=1,
            type=str,
            help="Directory of intrusive test suite")

        self.__args = parser.parse_args()
        self.__src_dir = None
        self.__clade_file = None
        self.__test_suite_dir = None

    # raise: FatalError
    def get_src_dir(self):
        if(self.__src_dir is None):
            self.__src_dir = os.path.abspath(self.__args.src_dir[0])
            if(not os.path.isdir(self.__src_dir)):
                err_msg = "%s argument %s is not an existing directory: %s"\
                    % ("1st", "SRC_DIR", self.__args.src_dir[0])
                raise FatalError(err_msg)
        return self.__src_dir

    # raise: FatalError
    def get_clade_file(self):
        if(self.__clade_file is None):
            self.__clade_file = os.path.abspath(self.__args.clade_file[0])
            if(not os.path.isfile(self.__clade_file)):
                err_msg = "%s argument %s is not an existing file: %s"\
                    % ("2nd", "CLADE_FIlE", self.__args.clade_file[0])
                raise FatalError(err_msg)
        return self.__clade_file

    # raise: FatalError
    def get_test_suite_dir(self):
        if(self.__test_suite_dir is None):
            self.__test_suite_dir = os.path.abspath(self.__args.test_suite_dir[0])
            if(not os.path.isdir(self.__test_suite_dir)):
                err_msg = "%s argument %s is not an existing directory: %s"\
                        % ("3d", "TEST_SUITE_DIR", self.__args.test_suite_dir[0])
                raise FatalError(err_msg)
        return self.__test_suite_dir


# User defined exception class
class FatalError(Exception):

    def __init__(self, msg, *args):
        super().__init__(args)
        self.__msg = msg

    def __str__(self):
        return self.get_err_msg()

    def get_err_msg(self):
        return self.__msg


# Class for auxillary functions
class Util:

    # raise: OSError
    @staticmethod
    def find_files_by_name(directory, file_name, file_set):
        for name in os.listdir(directory):
            path = os.path.join(directory, name)
            if(os.path.isfile(path) and (name == file_name)):
                file_set.add(path)
            elif(os.path.isdir(path) and (name != "..") and (name != ".")):
                Util.find_files_by_name(path, file_name, file_set)

    # raise: OSError
    @staticmethod
    def find_files_by_extension(directory, extension, is_recursive, file_set):
        for name in os.listdir(directory):
            path = os.path.join(directory, name)
            if(os.path.isfile(path)):
                root, ext = os.path.splitext(path)
                if(ext == extension):
                    file_set.add(path)
            elif(is_recursive \
                and os.path.isdir(path) \
                and (name != "..") \
                and (name != ".")):
                Util.find_files_by_extension(
                    path,
                    extension,
                    is_recursive,
                    file_set)

    # raise: OSError
    @staticmethod
    def read_file_lines(file_path):
        with open(file_path, "r") as fd:
            lines = fd.readlines()
        return lines

    # raise: OSError
    @staticmethod
    def write_lines_to_file(file_path, lines):
        fd = os.open(file_path, os.O_RDWR, 0o777)
        fo = os.fdopen(fd, "w+")
        try:
            fo.writelines(lines)
        finally:
            fo.close()

    @staticmethod
    def blank_string_to_none(s):
        if(s is not None):
            match = re.match(r'^\s*$', s)
            if(match):
                return None
        return s


# Class describing location of a synchronization point
class SyncPoint:

    def __init__(self, file, index, cl=None, method=None, source=None):
        self._file = file
        self._index = index
        # In case the key is defined but with empty value, we set 'None' constant to encode that the key is not provided
        self._cl = Util.blank_string_to_none(cl)
        self._method = Util.blank_string_to_none(method)
        self._source = Util.blank_string_to_none(source)

    def __hash__(self):
        return hash((self.file, self.index, self.cl, self.method, self.source))

    def __eq__(self, other):
        if not isinstance(other, SyncPoint):
            return False
        return (self.file == other.file) \
            and (self.index == other.index) \
            and (self.cl == other.cl) \
            and (self.method == other.method) \
            and (self.source == other.source)

    @property
    def file(self):
        return self._file

    @property
    def index(self):
        return self._index

    @property
    def cl(self):
        return self._cl

    @property
    def method(self):
        return self._method

    @property
    def source(self):
        return self._source

    def has_class(self):
        return self._cl is not None

    def has_method(self):
        return self._method is not None

    def has_source(self):
        return self._source is not None


    def to_string(self):
        s = ["file: %s" % (self.file), "index: %s" % (str(self.index))]
        if(self.has_class()):
            s.append("class: %s" % (str(self.cl)))
        if(self.has_method()):
            s.append("method: %s" % (str(self.method)))
        if(self.has_source()):
            s.append("source: %s" % (str(self.source)))
        return ", ".join(s)


# Class describing a synchronization action (code)
# of a single test at one synchronization point
class SyncAction:

    ID = 0

    def __init__(self, code):
        self._code = code
        self._id = SyncAction.ID
        SyncAction.ID += 1

    def __hash__(self):
        return hash(self._id)

    def __eq__(self, other):
        if not isinstance(other, SyncAction):
            return False
        return self._id == other.id

    @property
    def code(self):
        return self._code

    @property
    def id(self):
        return self._id


# Abstraction for a source code file
class SourceFile:

    def __init__(self, path):
        self._path = path

    def __hash__(self):
        return hash(self._path)

    def __eq__(self, other):
        if not isinstance(other, SourceFile):
            return False
        return self.path == other.path

    @property
    def path(self):
        return self._path


# Abstraction for a header file, i.e.
# file included by another source or header file
class HeaderFile(SourceFile):
    def __init__(self, path, including_source_file_path):
        self._including_source_file_path = including_source_file_path
        super().__init__(path)

    def __hash__(self):
        return hash((self.path, self._including_source_file_path))

    def __eq__(self, other):
        if not isinstance(other, HeaderFile):
            return False
        return super().__eq__(other) \
            and (self.including_source_file_path == other.including_source_file_path)

    @property
    def including_source_file_path(self):
        return self._including_source_file_path


# Parser of bindings.yml files
class BindingsFileParser:

    # raise: OSError
    def __init__(self, file):
        self.__file = file
        with open(file, "r") as f:
            self.__obj = yaml.safe_load(f)

    #raise: yamale.YamaleError
    def validate(self, schema_file):
        schema = yamale.make_schema(schema_file)
        data = yamale.make_data(self.__file)
        yamale.validate(schema, data)

    # Set of header files with declarations referenced in the code
    # of synchronization actions, e.g. function calls, constants
    # raise: FatalError
    def get_declaration_set(self, test_dir):
        l = self.__obj["declaration"].split(",")
        i = 0
        while(i < len(l)):
            l[i] = l[i].lstrip().rstrip()
            abs_path = os.path.join(test_dir, l[i])
            if(not os.path.isfile(abs_path)):
                err_msg = "[%s] in declaration list from [%s] is not an existing file"\
                    % (l[i], self.__file)
                raise FatalError(err_msg)
            l[i] = abs_path
            i += 1
        return set(l)

    # raise: FatalError
    def get_mapping(self, src_dir):
        m = {}
        l = self.__obj["mapping"]
        for e in l:
            f = e.get("file")
            abs_f = os.path.join(src_dir, f)
            if(not os.path.isfile(abs_f)):
                err_msg = "file attribute [%s] from [%s] is not an existing file"\
                    % (f, self.__file)
                raise FatalError(err_msg)

            s = e.get("source")
            if(s is None):
                abs_s = None
            else:
                abs_s = os.path.join(src_dir, s)
                if(not os.path.isfile(abs_s)):
                    err_msg = "source attribute [%s] from [%s] is not an existing file"\
                        % (s, self.__file)
                    raise FatalError(err_msg)

            ref = SyncPoint(
                file=abs_f,
                index=e.get("index"),
                cl=e.get("class"),
                method=e.get("method"),
                source=abs_s)
            m[ref] = SyncAction(e.get("code"))
        return m


# Class implementing all instrumentation logic (algorithm)
class Instrumentator:

    def __init__(self, args_parser):
        # arguments parser
        self.__args_parser = args_parser

        # set of bindings.yml files
        self.__bindings_file_set = set()

        # dictionary from synchronization action
        # to set of header files declaring functions,
        # constants and etc. used in that synchronization
        # action
        self.__sync_action_to_declaration_set = {}

        # dictionary from synchronization action
        # to test identifier
        self.__sync_action_to_test_id = {}

        # dictionary from synchronization point
        # to list of synchronization actions for that point
        self.__sync_point_to_sync_action_list = {}

        # dictionary from synchronization point to
        # line number in which the synchronization comment
        # for that synchronization point finishes
        self.__sync_point_to_line = {}

        # dictionary from file to set of synchronization
        # points from that file
        self.__file_to_sync_point_set = {}

        # dictionary from file to list of compile options
        # used to parse that file
        self.__file_to_compile_option_list = {}


    def run(self):
        Util.find_files_by_name(
            self.__args_parser.get_test_suite_dir(),
            BINDINGS_FILE_NAME,
            self.__bindings_file_set)
        # go over all bindings.yml files, parse them and fill data structures
        self.__bindings_file_set_work()
        self.__lookup_compile_opts_in_clade_db()
        framework_header_file_set = set()
        Util.find_files_by_extension(
            self.__args_parser.get_test_suite_dir(),
            HEADER_FILE_EXTENSION,
            False,
            framework_header_file_set)

        # Instrumentator includes all headers from runtime/tests/intrusive-tests directory
        # in all places where code is needed to be synced (according to bindings.yml).
        # That is why it includes intrusive_test_option.h file, which contains RuntimeOptions,
        # so framework cannot be used in libarkbase, compiler and etc, but actually
        # intrusive_test_option.h is only needed to initialize testsuite (see Runtime::Create)
        for header in framework_header_file_set:
            if "intrusive_test_option.h" in header:
                framework_header_file_set.remove(header)
                break

        # go over map elements (from instrumented file to sync points in that file)
        for file in self.__file_to_sync_point_set:
            # find line numbers of synchronization points
            # in the source code and header files
            self.__lookup_sync_points_with_libclang(file)
            lines = Util.read_file_lines(file.path)
            try:
                sync_point_list = list(self.__file_to_sync_point_set[file])
            except KeyError:
                print(f"no key {file}")
                sys.exit(-1)
            try:
                sync_point_list.sort(key=lambda sp_ref : self.__sync_point_to_line[sp_ref])
            except KeyError:
                print(f"no keys in {self.__sync_point_to_line}")
                sys.exit(-1)

            # Header files that should be '#included' in the instrumented file
            header_file_set = set(framework_header_file_set)
            self.__add_changes_to_files(header_file_set, sync_point_list, lines)
            incl_text_list = []
            for header in header_file_set:
                incl_text_list.append("#include \"")
                incl_text_list.append(header)
                incl_text_list.append("\"\n")
            lines.insert(0, ''.join(incl_text_list))
            Util.write_lines_to_file(file.path, lines)

    def __add_changes_to_files(self, header_file_set, sync_point_list, lines):
        sync_point_idx = 0
        # go over sync points in a file
        while(sync_point_idx < len(sync_point_list)):
            try:
                sp_ref = sync_point_list[sync_point_idx]
            except KeyError:
                print(f"no key {sync_point_idx}")
                sys.exit(-1)
            sync_point_code = []
            # go over synchronization actions for a synchronization point
            # and insert source code of the synchronization actions into
            # file lines

            try:
                sync_action_list = self.__sync_point_to_sync_action_list[sp_ref]
            except KeyError:
                print(f"no key {sp_ref}")
                sys.exit(-1)

            for sync_action in sync_action_list:
                self.__rep_sync_actions_process(sp_ref, sync_point_code, sync_action, header_file_set)
            sync_point_code.insert(0, "#if defined(INTRUSIVE_TESTING)\n")
            sync_point_code.append("#endif\n")

            try:
                lines.insert(
                    self.__sync_point_to_line[sp_ref] - 1 + sync_point_idx,
                    ''.join(sync_point_code))
            except KeyError:
                print(f"no key {sp_ref}")
                sys.exit(-1)

            sync_point_idx += 1

    def __rep_sync_actions_process(self, sp_ref, sync_point_code, sync_action, header_file_set):
        if(sp_ref.has_method()):
            if(len(sync_point_code) > 0):
                sync_point_code.append("\nelse ")
            sync_point_code.append("if(")
            sync_point_code.append(CPP_CODE_TO_GET_TEST_ID_IN_PANDA)
            sync_point_code.append(" == (uint32_t)")
            try:
                sync_point_code.append(self.__sync_action_to_test_id[sync_action])
            except KeyError:
                print(f"no key {sync_action}")
                sys.exit(-1)
            sync_point_code.append("){\n")
        sync_point_code.append(sync_action.code)
        if(sp_ref.has_method()):
            sync_point_code.append("\n}")
        sync_point_code.append("\n")
        try:
            header_file_set.update(self.__sync_action_to_declaration_set[sync_action])
        except KeyError:
            print(f"no key {sync_action}")
            sys.exit(-1)

    def __bindings_file_set_work(self):
        for bindings_file in self.__bindings_file_set:
            bindings_parser = BindingsFileParser(bindings_file)
            bindings_parser.validate(BINDINGS_SCHEMA_FILE)
            test_dir = self.__get_test_dir(bindings_file)
            test_id = self.__get_test_id(test_dir)
            declaration_set = bindings_parser.get_declaration_set(test_dir)
            mapping = bindings_parser.get_mapping(self.__args_parser.get_src_dir())
            # go over mapping from sync point to sync action in a single bindings.yml
            for r in mapping:
                a = mapping[r]
                self.__sync_action_to_declaration_set[a] = declaration_set
                self.__sync_action_to_test_id[a] = test_id
                if(r not in self.__sync_point_to_sync_action_list):
                    self.__sync_point_to_sync_action_list[r] = []
                self.__sync_point_to_sync_action_list[r].append(a)
                if(r.has_source()):
                    f = HeaderFile(r.file, r.source)
                else:
                    f = SourceFile(r.file)
                if(f not in self.__file_to_sync_point_set):
                    self.__file_to_sync_point_set[f] = set()
                self.__file_to_sync_point_set[f].add(r)

    def __get_test_dir(self, bindings_file):
        return os.path.dirname(bindings_file)

    def __get_test_id(self, test_dir):
        return os.path.basename(test_dir).upper()

    # parse clade database file and find compilation options
    # for source code modules
    def __lookup_compile_opts_in_clade_db(self):
        clade_cmds_file = self.__args_parser.get_clade_file()
        c = Clade(work_dir=os.path.dirname(clade_cmds_file), cmds_file=clade_cmds_file)
        f_list = list(self.__file_to_sync_point_set.keys())
        f_list.sort(key=lambda f : f.including_source_file_path if isinstance(f, HeaderFile) else f.path)
        # go over types of compilation commands
        # (emitted by C and C++ compilers, 2 items in the list)
        for cmd_type in ["CXX", "CC"]:
            for cmd in c.get_all_cmds_by_type(cmd_type):
                # go over input files of compilation commands
                # i.e. parsed files (usually 1 item in the list)
                for compiled_file_path in cmd["in"]:
                    self.__filter_file(f_list, compiled_file_path, c, cmd)
        if(len(f_list) > 0):
            err_msg = ["Failed to find compilation command for source code modules:"]
            for f in f_list:
                if isinstance(f, HeaderFile):
                    src_file_path = f.including_source_file_path
                else:
                    src_file_path = f.path
                err_msg.append("\n")
                err_msg.append(src_file_path)
            raise FatalError(''.join(err_msg))

    def __is_kind_of_libclang_function(self, kind):
        return kind == CursorKind.FUNCTION_DECL \
            or kind == CursorKind.CXX_METHOD \
            or kind == CursorKind.FUNCTION_TEMPLATE \
            or kind == CursorKind.CONSTRUCTOR \
            or kind == CursorKind.DESTRUCTOR \
            or kind == CursorKind.CONVERSION_FUNCTION

    def __is_kind_of_libclang_class(self, kind):
        return kind == CursorKind.CLASS_DECL \
            or kind == CursorKind.CLASS_TEMPLATE \
            or kind == CursorKind.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION \
            or kind == CursorKind.STRUCT_DECL

    def __filter_file(self, f_list, compiled_file_path, c, cmd):
        i = 0
        # go over sorted file List
        while(i < len(f_list)):
            f = f_list[i]
            if isinstance(f, HeaderFile):
                src_file_path = f.including_source_file_path
            else:
                src_file_path = f.path
            if(compiled_file_path == src_file_path):
                compile_option_list = c.get_cmd_opts(cmd["id"])
                compile_option_list.append('%s=%s' % (CLANG_CUR_WORKING_DIR_OPT, cmd["cwd"]))
                if isinstance(f, HeaderFile):
                    compile_option_list.extend(CPP_HEADER_COMPILE_OPTION_LIST)
                self.__file_to_compile_option_list[f] = compile_option_list
                f_list.pop(i)
            elif(compiled_file_path < src_file_path):
                break
            else:
                i += 1

    # Find line numbers of synchronization points
    # in source code modules and header files
    def __lookup_sync_points_with_libclang(self, file):
        try:
            not_found = list(self.__file_to_sync_point_set[file])
        except KeyError:
            print(f"no key {file}")
            sys.exit(-1)

        candidates = []

        clang_index = Index.create()
        try:
            tu = clang_index.parse(file.path, args=self.__file_to_compile_option_list[file])
        except KeyError:
            print(f"no key {file}")
            sys.exit(-1)
        for token in tu.get_tokens(extent=tu.cursor.extent):
            if(token.kind != TokenKind.COMMENT):
                continue
            match = re.match(r'\s*/\*\s*@sync\s+([0-9]+).*', token.spelling)
            if(not match):
                continue

            candidates.clear()
            candidates.extend(not_found)

            self.__clear_candidates_by_match(candidates, match)
            self.__clear_candidates_by_token(candidates, token)

            for sync_point in candidates:
                self.__sync_point_to_line[sync_point] = token.extent.end.line + 1
                not_found.remove(sync_point)

        if(len(not_found) > 0):
            err_msg = ["Failed to find synchronization points:"]
            for sync_point in not_found:
                err_msg.append("\n")
                err_msg.append(sync_point.to_string())
            raise FatalError(''.join(err_msg))

    def __clear_candidates_by_match(self, candidates, match):
        i = 0
        while(i < len(candidates)):
            if(int(match.group(1)) != candidates[i].index):
                candidates.pop(i)
            else:
                i += 1

    def __clear_candidates_by_token(self, candidates, token):
        i = 0
        while(i < len(candidates)):
            need_continue = False
            if(candidates[i].has_method()):
                need_continue = self.__check_for_method(candidates, token, i)

            elif(candidates[i].has_class()):
                if(not(self.__is_kind_of_libclang_class(token.cursor.kind)) \
                    or (token.cursor.spelling != candidates[i].cl)):
                    candidates.pop(i)
                    continue
            if(need_continue):
                continue
            i += 1

    def __check_for_method(self, candidates, token, i):
        if(not(hasattr(token.cursor, "semantic_parent"))):
            candidates.pop(i)
            return True
        sem_parent = token.cursor.semantic_parent
        if(not(self.__is_kind_of_libclang_function(sem_parent.kind)) \
            or (sem_parent.spelling != candidates[i].method)):
            candidates.pop(i)
            return True

        if(candidates[i].has_class()):
            if(not(hasattr(sem_parent, "semantic_parent"))):
                candidates.pop(i)
                return True
            sem_grand_parent = sem_parent.semantic_parent
            if(not(self.__is_kind_of_libclang_class(sem_grand_parent.kind)) \
                or (sem_grand_parent.spelling != candidates[i].cl)):
                candidates.pop(i)
                return True
        return False


if __name__ == '__main__':
    err_msg_suffix = "Error! Source code instrumentation failed. Reason:"
    try:
        exit_code = 0
        argument_parser = ArgsParser()
        instrumentator = Instrumentator(argument_parser)
        instrumentator.run()
    except yaml.YAMLError as yaml_err:
        exit_code = 1
        print(err_msg_suffix)
        traceback.print_exc(file=sys.stdout, limit=0)
    except yamale.YamaleError as yamale_err:
        exit_code = 2
        print(err_msg_suffix)
        for res in yamale_err.results:
            print("Error validating '%s' against schema '%s'\n\t" % (res.data, res.schema))
            for er in res.errors:
                print('\t%s' % er)
    except OSError as os_err:
        exit_code = 3
        err_message = [os_err.strerror]
        if(hasattr(os_err, 'filename') and (os_err.filename is not None)):
            err_message.append(": ")
            err_message.append(os_err.filename)
        if(hasattr(os_err, 'filename2') and (os_err.filename2 is not None)):
            err_message.append(", ")
            err_message.append(os_err.filename2)
        print(''.join(err_message))
    except FatalError as ferr:
        exit_code = 4
        print(err_msg_suffix, ferr.get_err_msg())
    except Exception as err:
        exit_code = 5
        print(err_msg_suffix)
        traceback.print_exc(file=sys.stdout)
    sys.exit(exit_code)
