# Copyright (c) 2024 Huawei Device Co., Ltd.
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

require 'erb'
require_relative 'common_gen_tests_api'

abckit_scripts = File.dirname(__FILE__)
abckit_root = File.expand_path("../", abckit_scripts)
abckit_test = File.join(abckit_root, "/tests/null_args_tests/")

excluded_funcs = [
  "getLastError",
  "moduleAddImportFromArkTsV2ToArkTsV2",
  "moduleAddExportFromArkTsV2ToArkTsV2"
]

implemented_api_map = collect_implemented_api_map(excluded_funcs)
null_args_tests_erb = File.join(abckit_test, "null_args_tests.cpp.erb")
implemented_api_map.each_key do |domain|
  iteration = 0
  index = 0
  slice_size = 100
  api_funcs_arr = implemented_api_map[domain]
  total_domain_api_funcs = api_funcs_arr.length

  puts "#{domain}: #{total_domain_api_funcs}"

  while index < total_domain_api_funcs do
    testfile_fullpath = File.join(abckit_test, "null_args_tests_#{domain}_#{iteration}.cpp")
    res = ERB.new(File.read(null_args_tests_erb), nil, "%").result(binding)
    File.write(testfile_fullpath, res)
    iteration += 1
    index += slice_size
  end
end
