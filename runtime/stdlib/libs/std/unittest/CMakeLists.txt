# Copyright © 2025 Huawei Technologies Co., Ltd. All Rights Reserved.
# This source file is part of the Codira project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

set(UNITTEST_SRCS
    api.code
    assert.code
    assert_exception.code
    bench_apis.code
    bench_executor.code
    bench_table.code
    bench_test_case.code
    check_result.code
    csv_strategy.code
    custom_assertion.code
    default_configuration.code
    entry_main.code
    execution.code
    fail.code
    filtration.code
    framework.code
    id.code
    json_strategy.code
    options.code
    parallel.code
    parameter_test_case.code
    platform_specific.code
    power_assert_diagram_builder.code
    pretty_console.code
    pretty_table.code
    process.code
    reexport.code
    render_exception.code
    report_bench_csv.code
    reporter_api.code
    reporter_progress.code
    reporter_progress_tests.code
    report_stats.code
    report_unittest_xml.code
    result.code
    result_serialize.code
    statistics.code
    strategy_processor.code
    suite_executor.code
    tempdir.code
    test_case_executor.code
    test_code.code
    text_report.code
    timeout.code
    transport.code
    unittest_exception.code
    util.code
    CACHE INTERNAL "")
add_subdirectory(native)
