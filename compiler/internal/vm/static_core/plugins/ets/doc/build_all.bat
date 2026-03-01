@rem Copyright (c) 2021-2024 Huawei Device Co., Ltd.
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem 
@rem http://www.apache.org/licenses/LICENSE-2.0
@rem 
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.

@echo off
echo Building all ArkTS docs ...
call build_annotations.bat
call build_cookbook.bat
call build_stdlib.bat
call build_tutorial.bat
call build_system.bat
call build_spec.bat
call build_runtime.bat
echo all pdf files are in appropriate 'build' sub-folder for every document
