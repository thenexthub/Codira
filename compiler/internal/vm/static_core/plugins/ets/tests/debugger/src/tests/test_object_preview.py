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
#

import cdp
from arkdb.debug import StopOnPausedType


TEST_OBJECT_PREVIEW = """\
class A {
    x: FixedArray<int> = [1, 2, 3, 4, 5]
};

function main(): void {
    let obj1 = new A()
    let obj2 = new A()
    let obj3 = new A()
    let arr: FixedArray<A> = [obj1, obj2, obj3]

    let primitiveArr: FixedArray<double> = [5.0, 4.0, 3.0, 2.0, 1.0]

    console.log("array"); // #BP
    console.log("array");
}\
"""


def get_expected_preview_for_array(class_name_prefix: str):
    property_preview = []
    for idx in range(0, 3):
        property_preview.append(
            cdp.runtime.PropertyPreview(
                name="{i}".format(i=idx),
                type_="object",
                value=f"{class_name_prefix}.A",
                value_preview=None,
                subtype=None,
            )
        )

    return cdp.runtime.ObjectPreview(
        type_="object",
        overflow=False,
        properties=property_preview,
        subtype="array",
        description=None,
        entries=None,
    )


def get_expected_preview_for_object():
    property_preview = cdp.runtime.PropertyPreview(
        name="x", type_="object", value="i32[](5)", value_preview=None, subtype="array"
    )

    return cdp.runtime.ObjectPreview(
        type_="object",
        overflow=False,
        properties=[property_preview],
        subtype=None,
        description=None,
        entries=None,
    )


def get_expected_preview_for_primitive_array():
    property_preview = []
    for idx in range(0, 5):
        property_preview.append(
            cdp.runtime.PropertyPreview(
                name="{i}".format(i=idx),
                type_="number",
                value=str(5 - idx),
                value_preview=None,
                subtype=None,
            )
        )

    return cdp.runtime.ObjectPreview(
        type_="object",
        overflow=False,
        properties=property_preview,
        subtype="array",
        description=None,
        entries=None,
    )


async def test_object_preview(
    run_and_stop_on_breakpoint: StopOnPausedType,
):
    async with run_and_stop_on_breakpoint(TEST_OBJECT_PREVIEW) as paused:
        # In current test TEST_OBJECT_PREVIEW there is only one local scope.
        local_scope_object_id = paused.local_scope().object_().data.object_id

        properties = await paused.client.get_properties(
            local_scope_object_id, own_properties=False, accessor_properties_only=False, generate_preview=True
        )
        # Obtained property descriptors for all objects in local scope in main.
        property_descriptors = properties[0]

        for prop_descriptor in property_descriptors:
            expected_preview: cdp.runtime.ObjectPreview
            generated_preview = prop_descriptor.value.preview
            if prop_descriptor.name == "arr":
                # Expect fully-qualified names of objects
                expected_preview = get_expected_preview_for_array("test_string")
            elif prop_descriptor.name in ["obj1", "obj2", "obj3"]:
                expected_preview = get_expected_preview_for_object()
            elif prop_descriptor.name == "primitiveArr":
                expected_preview = get_expected_preview_for_primitive_array()

            assert str(expected_preview) == str(generated_preview)
