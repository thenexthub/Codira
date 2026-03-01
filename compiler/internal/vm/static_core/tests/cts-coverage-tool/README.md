# Run Spec Coverage Tool

The tool reads ISA spec files and the given set of test files, then computes the coverage and outputs a bunch of coverage reports.

```
spectrac -r path_to_summary_report_yaml_file \
    -d directory_where_tests_located \
    -g glob_to_select_tests_files_in_tests_directory \
    -s paths_to_isa_yaml_files \
    -n path_to_nontestable_yaml_file \
    -u path_to_output_notcovered_yaml_file \
    -U path_to_output_notcovered_markdown_file \
    -o path_to_output_yaml_file_with_irrelevant_tests \
    -O path_to_output_markdown_file_with_irrelevant_tests \
    -f path_to_output_yaml_file_with_coverage_data \
    -F path_to_output_markdown_file_with_coverage_data
```

Options:
```
--testdir (-d) - directory with tests, required
--testglob (-g) - glob for selecting test files in tests directory ("**/*.pa" for example), required
--spec (-s) - ISA spec files, comma-separated, merged into one if multiple specified, required
--non-testable (-n) - yaml file with non-testable assertions
--full (-f) - output the full spec in yaml format with additional fields showing the coverage data (mark assertions as covered/not-covered/non-testable, provide coverage metric for each group)
--full_md (-F) - same as --full, but in markdown format
--uncovered (-u) - output yaml document listing the spec areas not covered by tests
--uncovered_md (-U) - same as --uncovered, but in markdown format
--orphaned (-o) - output list of test files that found not relevant to the current spec (the assertions in test file aren't found in the spec)
--orphaned_md (-O) - same as --orphaned, but in markdown format
--report (-r) - output the test coverage summary report in yaml
```

Example:
```
cd ~/panda
tests/cts-coverage-tool/bin/spectrac.rb \
  -g "**/*.pa" \
  -d tests/cts-generator/cts-generated/ \
  -s isa/isa.yaml,plugins/java/isa/isa.yaml \
  -f full.yaml \
  -u uncovered.yaml \
  -o orphaned.yaml \
  -n tests/cts-coverage-tool/config/non_testable.yaml
```
