# Templates

This directory contains template files for generating benchmark code in different languages.

Below is the description for parameters used.
Note: full actual list is in `src/vmb/doclet.py::TemplateVars`

- `src` - Full bench class source
- `state_name` - Name of main class
- `state_setup` - Setup method call, f.e: `bench.SomeMethod();`
- `state_params` - '\n'-joined list of 'bench.param1=5;'
- `fixture` - ';'-joined param list of 'param1=5'
- `fix_id` - fixture identifier
- `method_name` - Name of test method
- `method_rettype`
- `method_call`
- `bench_name`
- `bench_path`
- `common` - code block shared between tests (added via `common` dirs)
- `imports` - import statement (added via `@Import` doclet)
- `includes` - shared code block (added via `@Include` doclet)
- `mi` - Warm up Iterations
- `wi` - Measure Iterations
- `it` - Iteration Time (sec)
- `wt` - Warmup Time (sec)
- `fi` - Fast Iterations
- `gc` - GC pause (ms)
- `tags`
- `bugs`


Note that `bench` variable name is 'reserved' (i.e. hard-coded into test generator).