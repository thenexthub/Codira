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
def collect_implemented_api_map(excluded_funcs)
    implemented_api_raw = nil
    Dir.chdir(File.dirname(__FILE__)) do
        implemented_api_raw = `python abckit_status.py --capi --print-implemented 2>&1`.split(/\n/).sort
    end

    implemented_api_map = {}

    implemented_api_raw.each do |api_func_raw|
        domain, api_func = *api_func_raw.split(/::/)

        if excluded_funcs.include?(api_func)
            next
        end

        if implemented_api_map[domain].nil?
            implemented_api_map[domain] = [api_func]
        else
            implemented_api_map[domain].append(api_func)
        end
    end

    return implemented_api_map
end
