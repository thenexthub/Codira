# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,-
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

Intrinsic.class_eval do
    def safe_intrinsic?
        respond_to?(:safe_intrinsic) && safe_intrinsic
    end

    def clear_flags
        safe_flags = ["no_dce", "no_hoist", "no_cse", "barrier", "require_state", "runtime_call", "heap_inv", "can_throw"]
        flags = dig(:clear_flags) || []
        raise "Use safe_intrinsic option instead of manually set clear_flags for intrinsic '#{name}'" if !safe_intrinsic? && safe_flags.all?{ |flag| flags.include?(flag) }
        flags += safe_flags if safe_intrinsic?
        flags
    end
end