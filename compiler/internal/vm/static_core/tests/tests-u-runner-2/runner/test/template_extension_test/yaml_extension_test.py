#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import unittest
from pathlib import Path

from jinja2 import Environment, select_autoescape

from runner.extensions.generators.ets_cts.yaml_extension import YamlExtension


class YamlExtensionTest(unittest.TestCase):

    def __init__(self, method_name: str = "runTest") -> None:
        self.env: Environment = Environment(
            autoescape=select_autoescape(),
            extensions=[YamlExtension],
        )
        current_data_dir = Path(__file__).parent / "data"
        self.env.__setattr__('current_test_path_dir', current_data_dir)
        super().__init__(method_name)

    def test_one_yaml(self) -> None:
        template_text = (
            "{% yaml \"data_0.yaml\" as testData %}" 
            + "{% for c in testData.cases %}"
            + "{{c.a}}|{{c.b}}||{{c.desc}}\n"
            + "{% endfor %}"
        )
        expected_result = (
            "10|20||num from testData_0\n"
            + "testA2|testB2||str from testData_0\n"
            + "True|False||bool from testData_0"
        )
        self.__check(template_text, expected_result)

    def test_only_str_yaml(self) -> None:
        template_text = (
            "{% yaml \"data_str.yaml\" as y_str %}"
            + "/*{{y_str}}*/"
        )
        expected_result = "/*test string from testData 3*/"
        self.__check(template_text, expected_result)

    def test_only_simple_list_yaml_for(self) -> None:
        template_text = (
            "{% yaml \"data_list.yaml\" as testData %}" 
            + "{% for c in testData %}"
            + "{{c}}/"
            + "{% endfor %}"
        )
        expected_result = "1/2/33/test 1/True/0/"
        self.__check(template_text, expected_result)

    def test_only_simple_list_yaml_by_index(self) -> None:
        template_text = (
            "{% yaml \"data_list.yaml\" as testData %}"  
            + "{{testData[1]}} + {{testData[2]}} = 35"
            + "\n new: {{testData[0] + testData[1]}}"
        )
        expected_result = (
            "2 + 33 = 35"
            + "\n new: 3"
        )
        self.__check(template_text, expected_result)

    def test_empty_yaml(self) -> None:
        template_text = (
            "{% yaml \"data_empty.yaml\" as e_y %}"
            + "#{{e_y}}#"
        )
        expected_result = "#None#"
        self.__check(template_text, expected_result)

    def test_only_complex_list_yaml(self) -> None:
        template_text = (
            "{% yaml \"data_complex_list.yaml\" as testData %}" 
            + "{% for c in testData %}"
            + "{{c.a}}|{{c.b}}->{{c.fl.ca if c.fl is not none else 'n/a'}}"
            + " {{c.fl.cb if c.fl is not none else 'n/a'}}\n"
            + "{% endfor %}"
        )
        expected_result = (
            "a|bb->1 one\n"
            + "b|cc->10 ten\n"
            + "dd|bd->n\\n 12\n"
            + "ss|SSS->True test"
        )
        self.__check(template_text, expected_result)

    def test_complex_yaml(self) -> None:
        template_text = (
            "{% yaml \"data_1.yaml\" as td %}"
            + "#{{td.testName}}\n"
            + "{% for c in td.testData.cases %}"
            + "{{c.a}}|{{c.b}}//{{c.desc}}\n"
            + "{% endfor %}"
            + "field2={{td.testData.field2}}\n"
            + "field1={{td.testData.field1}}"
        )
        expected_result = (
            "#tst_10\n"
            + "1|2//num from testData_1\n"
            + "testA|testB//str from testData_1\n"
            + "False|True//bool from testData_1\n"
            + "field2=test2\n"
            + "field1=test1"
        )
        self.__check(template_text, expected_result)

    def test_many_yaml(self) -> None:
        template_text = (
            "{% yaml \"data_0.yaml\" as testData0 %}"
            + "{% yaml \"data_1.yaml\" as testData1 %}"
            + "{% for c in testData0.cases + testData1.testData.cases %}"
            + "{{c.a}}|{{c.b}}||{{c.desc}}\n"
            + "{% endfor %}"
        )
        expected_result = (
            "10|20||num from testData_0\n"
            + "testA2|testB2||str from testData_0\n"
            + "True|False||bool from testData_0\n"
            + "1|2||num from testData_1\n"
            + "testA|testB||str from testData_1\n"
            + "False|True||bool from testData_1"
        )
        self.__check(template_text, expected_result)

    def __check(self, template_text: str, expected_result: str) -> None:
        result = self.__render(template_text)
        self.assertEqual(result, expected_result)

    def __render(self, template_text: str) -> str:
        template = self.env.from_string(template_text)
        return template.render().strip()
