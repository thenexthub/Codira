##  Prerequisites
- All paths below are relative paths regarding root of arkcompiler_runtime_core repository (<https://gitee.com/openharmony/arkcompiler_runtime_core.git>)
- The following libraries (Jinja2==3.1.2  MarkupSafe==2.1.1  PyYAML==6.0) are required for test code generator
- These libraries can be installed by running scripts/install-deps-ubuntu. This script creates the python virtual environment (.venv-panda in the user's home directory) and installs required libraries there.
  - Go to root of arkcompiler runtime core repository
  - Run command `sudo ./scripts/install-deps-ubuntu -i=dev -i=test`

## Quick run
- Go to the folder `$panda_source/tests/tests-u-runner`
- Run tests `./runner.sh --ets-func-tests --build-dir $panda_build <other options>`. For other options see documentation for runner at `$panda_source/tests/tests-u-runner/readme.md`

## Only generate tests without running
- Go to the folder `$panda_source/tests/tests-u-runner`
- Run generator: `./generate.sh -t $panda_source/plugins/ets/tests/stdlib-templates/ -o /tmp/gen`. You can setup any other path for output of the generator if you wish.

## Template structure
For generating tests against ArkTS stdlib the Python library Jinja2 is used. For detail information about Jinja2 template, see :<https://jinja.palletsprojects.com/en/3.1.x/>
The common structure of a template is:

- The template itself. It contains a mix of Jinja2 statements and ArkTS code. The content will be rendered into clear ArkTS source code. 
- The configuration file in the yaml format is a set of objects which will be used for rendering Jinja2 template. 

There are two levels of templates:

  - common level with code that can be reused in different cases
  - specific level that usually is constructed from common level templates via **include** command of Jinja2. This level can contain additional code for the task.
  
  
The specific level template contains the loop over all objects in the configuration file. Then the generator takes an object from the configuration file and replaces corresponded variables in the template. Separated test object will be generated for each object. So number of tests created by generator will be equal number of objects in the configuration file. 


For example:

#### Specific level template
```
{% for item in std_math %}

/*---
desc:  {function: {{.item.method_name}}}
---*/

{% include 'utils/test_verifier_lib.j2' with context %}
{% include 'utils/test_function.j2' with context  %}

{% endfor %}
```
Where test_`function.j2` is common level template (see example below) and `test_verifier_lib.j2` - the file that contains function for verification test result.
#### Common level template (`test_function.j2`)
```
{%- for number, param in item.param_list.items() %}
const TEST_DATA_{{.number|upper}} : {{.item.param_init_data_type}} = {{.item.expected_data}}{{.param}};
{%- endfor %}
const TEST_DATA_EXPECTED : {{.item.expected_data_type}} = {{.item.expected_test_data}};

function main(): int {
    let max_test : int = TEST_DATA_EXPECTED.length;
    let passed_counter : int = 0;

    for (let i = 0; i < max_test; i++) {
       {%- for number, param in item.param_list.items() %}
       let {{.number}} = (TEST_DATA_{{.number|upper}}[i]);
       {%- endfor %}	 
       let expected = TEST_DATA_EXPECTED[i]
       let actual : {{.item.method_return_type}} =  {{.item.method_name}}({{.item.param_list.keys()|list|join(', ')}});
       {%- for number, param in item.param_list.items() %}
       console.print("PARAM:" + {{.number}} + ";");
       {%- endfor %}	 
       console.print("ACTUAL:" + actual + ";");
       console.print("EXPECTED:" + expected + ";");
       if ({{.item.verify_test_result_function}}(actual, expected)) {
         console.print("PASSED");
         passed_counter++;
       } else {
         console.print("FAILED")
       }
       console.println();
    }

    arktest.assertEQ(passed_counter, max_test)
    return 0;
}

```
#### Fragment of YAML configuration file
This example contains two YAML objects fragments, each of them will be used for creating a separated test
```
- {
    method_name: abs,
    param_list: {"param1":"[ 0.0, PI, -PI, 123.0, -123.0,]"},
    param_types: {"param1":double},
    method_return_type: double,
    param_init_data_types: {"param1":"double[]"},
    expected_data_type: "double[]",
    expected_test_data: "[ 0.0, PI, PI, 123.0, 123.0,]",
    verify_test_result_function: compare_float_point_value
  }
- {
    method_name: scalbn,
    param_list: {"param1": "[e, e, e, e, e, -e, -e, -e, -e, -e ]", "param2" :"[0, 1, 2, -1, -2, 0, 1, 2, -1, -2 ]"},
    method_return_type: double,
    param_types: {"param1":double, "param2":int},
    param_init_data_types: {"param1":"double[]", "param2":"int[]"},
    expected_data_type: "double[]",
    expected_test_data: "[ e, 5.43656366, 10.87312731, 1.35914091, 0.67957046, -e, -5.43656366,  -10.87312731, -1.35914091, -0.67957046]",
    verify_test_result_function: compare_float_point_value
  }

```
### Main keywords that are used in the configuration YAML file
- object_type - type of object that contains the tested method
- init_object_type - type of data that will be used for object initialization
- init_object_data_type - type of container that contains test data, usually this is an array
- init_object_data - dictionary with data that used for object initialization
- method_name - define name of the tested method
- method_return_type - type that returned by the tested method
- param_list - dictionary that contains test data for each parameter
- param_types - dictionary that contains type for each parameter
- param_init_data_types - dictionary that contains data type that used for parameters initialization
- expected_data_type - type of a container with expected data
- expected_test_data - actual data that should be used for tests verification
- verify_test_result_function - name of a function that used for test result verification. Usually it contains two parameters, actual data and expected data. Type of these parameters are described in expected_data_type keyword
