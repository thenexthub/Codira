#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#

# flake8: noqa
# pylint: skip-file

import sys
import pytest  # type: ignore
from unittest import TestCase
from dataclasses import asdict
from unittest.mock import patch
from vmb.doclet import DocletParser, TemplateVars
from vmb.helpers import get_plugin
from vmb.cli import Args
from vmb.generate import BenchGenerator

ETS_VALID = '''/**
 * @State
 * @Bugs gitee987654321, blah0102030405
 */

export class ArraySort {

    /**
     * @Param 4, 8, 16
     */
    size: int;

    /**
     * Specifics of the array to be sorted.
     * @Param "A", "B"
     */

    code: String;

    ints: int[];
    intsInitial: int[];

    /**
     * Foo
     */ 

    /**
     * Prepare array depending on the distribution variant.
     * @Setup
     */
    public prepareArray(): void {
        /* blah */
    }

    /**
     * @Benchmark -it 3
     * @Tags Ohos
     * @Bugs gitee0123456789
     */

    public sort(): void {
    }

    /**
     * @Benchmark
     * @Tags StdLib, StdLib_String, Ohos
     */
    public baseline(): void {
    }
}
'''

JS_CLASS = '''/**
 * @State
 */
class ArraySort {
    size = 1;
    /**
     * @Setup
     */
    prepareArray() {
        /* blah */
    }
    /**
     * @Benchmark
     * @returns {Int}
     */
    test() {
        return this.size;
    }
}
'''

ETS_DUP = '''
/**
 * @State
 */
export class ArraySort {
  /**
   * @Param 7
   * @Param 4
   */
  size: int;
}
'''

ETS_NOSTATE_BENCH = '''
class X {
  /**
   * @Benchmark
   */
  x(): int {
}
'''

ETS_NOSTATE_SETUP = '''
class X {
  /**
   * @Setup
   */
  x(): int {
}
'''

ETS_NOSTATE_PARAM = '''
class X {
  /**
   * @Param 1
   */
  size: int;
}
'''

ETS_NOLIST_PARAM = '''
/**
 * @State
 */
class X {
  /**
   * @Param
   */
  size: int;
}
'''

ETS_BENCH_LIST = '''
/**
 * @State
 */
class X {
  /**
   * Help me
   * @Benchmark -mi 10 -wi 11 -it 3 -wt 4 -fi 6 -gc 300
   * -mi 999 -wi 999 -it 999 -wt 999 -fi 999 -gc 999
   */
  public testme(): int {
}
'''

ETS_STATE_TAGS = '''

/**
 * @State
 * @Tags StdLib, StdLib_Math
 */
class MathFunctions {
'''

ETS_STATE_COMMENT = '''

/**
 * @State
 */

 // blah
// foo bar
class XXX {
'''

ETS_PARAM_INIT = '''
/**
 * @State
 */
class X {
/**
 * Array size.
 * @Param 1024
 */
  size: int = 1024;
'''

JS_PARAM_INIT = '''
/**
 * @State
 */
function X() {
    /**
     * With initialization.
     * @Param 1024
     */
        this.size = 16;
    /**
     * Without initialization.
     * @Param "one","two"
     */
        this.name;
'''

ETS_MEASURE_OVERRIDES = '''
    /**
    * @State
    * @Benchmark -mi 33 -wi 44 -it 55 -wt 66
    */
    class X {
    /**
    * @Benchmark -mi 1 -wi 2 -wt 4 -fi 5 -gc 6
    */
    public one(): int {
    }
    /**
    * @Benchmark
    */
    public two(): bool {
    }
    '''

ets_mod = get_plugin('langs', 'ets')
ts_mod = get_plugin('langs', 'ts')
js_mod = get_plugin('langs', 'js')


def test_valid_ets():
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(ETS_VALID, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('ArraySort' == parser.state.name)
    test.assertTrue(2 == len(parser.state.params))
    test.assertTrue(2 == len(parser.state.benches))


def test_js_class():
    js = js_mod.Lang()
    test = TestCase()
    test.assertIsNotNone(js)
    parser = DocletParser.create(JS_CLASS, js).parse()
    test.assertIsNotNone(parser.state)
    test.assertEqual('ArraySort', parser.state.name)
    test.assertEqual(1, len(parser.state.benches))


def test_duplicate_doclets():
    ets = ets_mod.Lang()
    TestCase().assertTrue(ets is not None)
    with pytest.raises(ValueError):
        DocletParser.create(ETS_DUP, ets).parse()


def test_no_state():
    ets = ets_mod.Lang()
    TestCase().assertTrue(ets is not None)
    for src in (ETS_NOSTATE_BENCH, ETS_NOSTATE_SETUP, ETS_NOSTATE_PARAM):
        with pytest.raises(ValueError):
            DocletParser.create(src, ets).parse()


def test_bench_list():
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(ETS_BENCH_LIST, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('X' == parser.state.name)
    b = parser.state.benches
    test.assertTrue(1 == len(b))
    test.assertTrue('testme' == b[0].name)
    args = vars(b[0].args)
    # -mi 10 -wi 11 -it 3 -wt 4 -fi 6 -gc 300
    test.assertTrue(10 == args['measure_iters'])
    test.assertTrue(11 == args['warmup_iters'])
    test.assertTrue(3 == args['iter_time'])
    test.assertTrue(4 == args['warmup_time'])
    test.assertTrue(6 == args['fast_iters'])
    test.assertTrue(300 == args['sys_gc_pause'])


def test_bugs():
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(ETS_VALID, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('ArraySort' == parser.state.name)
    b = parser.state.benches
    test.assertTrue(2 == len(b))
    test.assertTrue('sort' == b[0].name)
    test.assertTrue(['gitee0123456789'] == b[0].bugs)
    test.assertTrue('gitee987654321' in parser.state.bugs)
    test.assertTrue('blah0102030405' in parser.state.bugs)


def test_state_tags():
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(ETS_STATE_TAGS, ets).parse()
    test.assertTrue(parser.state is not None)
    tags = parser.state.tags
    test.assertTrue(2 == len(tags))
    test.assertTrue('StdLib' in tags)
    test.assertTrue('StdLib_Math' in tags)


def test_tags_before_state():
    src = '''
    /**
     * @Tags Before, More
     */

    /**
     * @State
     * @Tags StdLib
     */
    class MathFunctions {
    '''
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(src, ets).parse()
    test.assertTrue(parser.state is not None)
    tags = parser.state.tags
    test.assertTrue(3 == len(tags))
    for t in ('Before', 'More', 'StdLib'):
        test.assertTrue(t in tags)


def test_state_comment():
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(ETS_STATE_COMMENT, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('XXX' in parser.state.name)


def test_param_init():
    fixture = {
        'ts':  {'module': ts_mod,  'src': ETS_PARAM_INIT},
        'ets': {'module': ets_mod, 'src': ETS_PARAM_INIT},
        'js':  {'module': js_mod,  'src': JS_PARAM_INIT}
    }
    for _, v in fixture.items():
        lang = v['module'].Lang()
        test = TestCase()
        test.assertTrue(lang is not None)
        parser = DocletParser.create(v['src'], lang).parse()
        test.assertTrue(parser.state is not None)


def test_measure_overrides():
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(ETS_MEASURE_OVERRIDES, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('X' == parser.state.name)
    b = parser.state.benches
    test.assertTrue(2 == len(b))
    test.assertTrue('one' == b[0].name)
    args = vars(b[0].args)
    test.assertTrue(1 == args['measure_iters'])
    test.assertTrue(2 == args['warmup_iters'])
    # Should be default
    test.assertTrue(args['iter_time'] is None)
    test.assertTrue(4 == args['warmup_time'])
    test.assertTrue(5 == args['fast_iters'])
    test.assertTrue(6 == args['sys_gc_pause'])
    test.assertTrue(2 == len(b))
    test.assertTrue('two' == b[1].name)
    args = vars(parser.state.bench_args)
    test.assertTrue(33 == args['measure_iters'])
    test.assertTrue(44 == args['warmup_iters'])
    test.assertTrue(55 == args['iter_time'])
    test.assertTrue(66 == args['warmup_time'])
    # Should be default
    test.assertTrue(args['fast_iters'] is None)
    # Should be default
    test.assertTrue(args['sys_gc_pause'] is None)
    # Test cmdline overrides
    with patch.object(sys, 'argv', 'vmb gen --lang ets -fi 123 blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed('', parser.state, args))
        test.assertTrue(2 == len(tpl_vars))
        var1, var2 = tpl_vars
        # Bench one
        test.assertTrue(var1.fix_id == 0)
        test.assertTrue(var1.method_name == 'one')
        test.assertTrue(var1.method_rettype == 'int')
        test.assertTrue(var1.bench_name == 'X_one')
        test.assertTrue(var1.mi == 1)
        test.assertTrue(var1.wi == 2)
        # Should use class level
        test.assertTrue(var1.it == 55)
        test.assertTrue(var1.wt == 4)
        test.assertTrue(var1.fi == 5)
        test.assertTrue(var1.gc == 6)
        # Bench two
        test.assertTrue(var2.fix_id == 0)
        test.assertTrue(var2.method_name == 'two')
        test.assertTrue(var2.method_rettype == 'bool')
        test.assertTrue(var2.bench_name == 'X_two')
        test.assertTrue(var2.mi == 33)
        test.assertTrue(var2.wi == 44)
        test.assertTrue(var2.it == 55)
        test.assertTrue(var2.wt == 66)
        # Should use sys.cmdline
        test.assertTrue(var2.fi == 123)
        # Should be default
        test.assertTrue(var2.gc == -1)


def test_tags():
    src = '''
    /**
    * @State
    * @Tags Cool, Fast
    */
    class X {
    /**
    * @Benchmark
    * @Tags Fast, One
    */
    public one(): int {
    }
    /**
    * @Benchmark
    */
    public two(): bool {
    }
    '''
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(src, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('X' == parser.state.name)
    test.assertTrue(set(parser.state.tags) == {'Cool', 'Fast'})
    b = parser.state.benches
    test.assertTrue(2 == len(b))
    test.assertTrue(set(b[0].tags) == {'Fast', 'One'})
    test.assertTrue(set(b[1].tags) == set())
    # w/o --tags
    with patch.object(sys, 'argv', 'vmb gen --lang ets blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed(
            '', parser.state, args))
        test.assertTrue(2 == len(tpl_vars))
        # bench + state params merged
        test.assertTrue(tpl_vars[0].tags == {'Cool', 'Fast', 'One'})
        test.assertTrue(tpl_vars[1].tags == {'Cool', 'Fast'})
    # with --tags filter
    with patch.object(sys, 'argv',
                      'vmb gen --lang ets --tags=One blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed(
            '', parser.state, args))
        test.assertTrue(1 == len(tpl_vars))
        # bench + state params merged
        test.assertTrue(tpl_vars[0].tags == {'Cool', 'Fast', 'One'})
    # filter all
    with patch.object(
            sys,
            'argv', 'vmb gen --lang ets --tags=Unexistent blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed(
            '', parser.state, args))
        test.assertTrue(0 == len(tpl_vars))


def test_import():
    src = '''
    /**
     * @Import hello from world
     */

    /**
     * @Import x from y
     * @State
     * @Import a from b
     */
    class Blah {
    }
    /**
     * @Benchmark
     * @Import m from n
     */
     public x(): int {
     }
    '''
    ets = ets_mod.Lang()
    test = TestCase()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(src, ets).parse()
    test.assertTrue(parser.state is not None)
    imports = parser.state.imports
    test.assertTrue(4 == len(imports))
    for i in ('hello from world', 'x from y', 'a from b', 'm from n'):
        test.assertTrue(i in imports)


def test_generic_type():
    src = '''
    /**
     * @State
     */
    class X {
      /**
       * @Benchmark
       */
      public testme(): Set<int> {
      }
    }
    '''
    test = TestCase()
    ets = ets_mod.Lang()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(src, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('X' == parser.state.name)
    b = parser.state.benches
    test.assertTrue(1 == len(b))
    test.assertTrue(b[0].name == 'testme')
    test.assertTrue(b[0].return_type == 'Set<int>')


def test_generator():
    src = '''
    /**
     * @State
     * @Generator gen_classes.py
     * @Rank 10
     */
    export class X {
        /**
         * @Teardown
         */
         teardown() {
         }
    }'''
    test = TestCase()
    ets = ets_mod.Lang()
    test.assertTrue(ets is not None)
    parser = DocletParser.create(src, ets).parse()
    test.assertTrue(parser.state is not None)
    test.assertTrue('X' == parser.state.name)
    test.assertTrue(parser.state.generator == 'gen_classes.py')


def test_templates():
    test = TestCase()
    ets = ets_mod.Lang()
    test.assertIsNotNone(ets)
    parser = DocletParser.create(ETS_VALID, ets).parse()
    tpl_vars = list(TemplateVars.params_from_parsed('fake src', parser.state))
    test.assertGreater(len(tpl_vars), 0)
    with patch.object(sys, 'argv', 'vmb gen -l ets blah'.split()):
        generator = BenchGenerator(Args())
        for tpl in ('ets.j2', 'js.j2', 'ts.j2', 'swift.j2'):
            for v in tpl_vars:
                template = generator.get_template(tpl)
                tpl_values = asdict(v)
                template.render(**tpl_values)


def test_tag_len_limit():
    src = '''
    /**
    * @State
    */
    class X {
    /**
    * @Benchmark
    * @Tags ShortTag, Max_______Len____Tag
    */
    public one(): int {
    }
    /**
    * @Benchmark
    * @Tags OtherShortTag, Too_Long____Len____Tag
    */
    public two(): bool {
    }
    /**
    * @Benchmark
    * @Tags LastShortTag
    */
    public three(): bool {
    }
    /**
    * @Benchmark
    * @Tags Last_Too_Long____Len____Tag
    */
    public four(): bool {
    }
    '''
    test = TestCase()
    ets = ets_mod.Lang()
    parser = DocletParser.create(src, ets).parse()
    
    with patch.object(sys, 'argv', 'vmb gen --lang ets blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed('', parser.state, args))
        test.assertEqual(len(tpl_vars), 2)
        test.assertEqual(tpl_vars[0].tags, {"ShortTag", "Max_______Len____Tag"})
        test.assertEqual(tpl_vars[1].tags, {"LastShortTag"})
