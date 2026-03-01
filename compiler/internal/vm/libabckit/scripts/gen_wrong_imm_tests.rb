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

abckit_scripts = File.dirname(__FILE__)
abckit_root = File.expand_path("../", abckit_scripts)
abckit_test = File.join(abckit_root, "/tests/wrong_imm_tests/")

funcs_with_imm_bitsize_8 = [
  "iCreateCreateobjectwithexcludedkeys",
  "iCreateNewlexenv",
  "iCreateNewlexenvwithname",
  "iCreateCallruntimeLdsendableexternalmodulevar",
  "iCreateCallruntimeNewsendableenv",
  "iCreateCallruntimeStsendablevar",
  "iCreateCallruntimeLdsendablevar",
  "iCreateDefinefunc",
  "iCreateDefinemethod",
  "iCreateCopyrestargs",
  "iCreateLdlexvar",
  "iCreateStlexvar",
  "iCreateSetgeneratorstate",
  "iCreateNewobjrange",
  "iCreateCallrange",
  "iCreateCallthisrange",
  "iCreateSupercallarrowrange"
]

funcs_with_imm_bitsize_16 = [
  "iCreateLdprivateproperty",
  "iCreateStprivateproperty",
  "iCreateTestin",
  "iCreateWideCreateobjectwithexcludedkeys",
  "iCreateWideNewlexenv",
  "iCreateWideNewlexenvwithname",
  "iCreateCallruntimeCreateprivateproperty",
  "iCreateCallruntimeDefineprivateproperty",
  "iCreateCallruntimeDefinesendableclass",
  "iCreateCallruntimeLdsendableclass",
  "iCreateCallruntimeWideldsendableexternalmodulevar",
  "iCreateCallruntimeWidenewsendableenv",
  "iCreateCallruntimeWidestsendablevar",
  "iCreateCallruntimeWideldsendablevar",
  "iCreateThrowIfsupernotcorrectcall",
  "iCreateDefineclasswithbuffer",
  "iCreateLdobjbyindex",
  "iCreateStobjbyindex",
  "iCreateStownbyindex",
  "iCreateWideCopyrestargs",
  "iCreateWideLdlexvar",
  "iCreateWideStlexvar",
  "iCreateWideLdpatchvar",
  "iCreateWideStpatchvar",
  "iCreateWideNewobjrange",
  "iCreateWideCallrange",
  "iCreateWideCallthisrange",
  "iCreateSupercallarrowrange"
]

funcs_with_imm_bitsize_32 = [
  "iCreateCallruntimeDefinefieldbyindex",
  "iCreateWideStownbyindex",
  "iCreateWideStobjbyindex",
  "iCreateWideLdobjbyindex"
]

implemented_api_raw = nil
Dir.chdir(abckit_scripts) do
  implemented_api_raw = `python abckit_status.py --capi --print-implemented 2>&1`.split(/\n/).sort
end

implemented_api_map = {}
included_funcs = funcs_with_imm_bitsize_8 + funcs_with_imm_bitsize_16 + funcs_with_imm_bitsize_32

implemented_api_raw.each do |api_func_raw|
  domain, api_func = *api_func_raw.split(/::/)

  if api_func.include? "iCreate" and domain.include? "IsaApiDynamicImpl" and included_funcs.include?(api_func)
    if implemented_api_map[domain].nil?
      implemented_api_map[domain] = [api_func]
    else
      implemented_api_map[domain].append(api_func)
    end
    next
  end
end

wrong_imm_tests_erb = File.join(abckit_test, "wrong_imm_tests.cpp.erb")
implemented_api_map.each_key do |domain|
  iteration = 0
  index = 0
  slice_size = 100
  api_funcs_arr = implemented_api_map[domain]
  total_domain_api_funcs = api_funcs_arr.length

  puts "#{domain}: #{total_domain_api_funcs}"

  while index < total_domain_api_funcs do
    testfile_fullpath = File.join(abckit_test, "wrong_imm_tests_#{domain}_#{iteration}.cpp")
    res = ERB.new(File.read(wrong_imm_tests_erb), nil, "%").result(binding)
    File.write(testfile_fullpath, res)
    iteration += 1
    index += slice_size
  end
end
