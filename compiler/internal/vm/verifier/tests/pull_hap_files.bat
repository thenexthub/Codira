@rem Copyright (c) 2024 Huawei Device Co., Ltd.
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem
@rem  http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.

@echo off
setlocal enabledelayedexpansion

@rem ##########################################################################
@rem
@rem  Verifier-Pull-Haps startup script for Windows
@rem
@rem ##########################################################################

@rem Initialize variables
set "device="
set "device_count=0"

@rem Check if a device is connected
echo Checking for connected devices...
for /f "delims=" %%i in ('hdc list targets') do (
    if not "%%i"=="" (
        set "device=%%i"
        set /a device_count+=1
    )
)

@rem Verify if device information is valid
if "%device%"=="[Empty]" (
    echo No devices connected. Please connect a device first.
    pause
    exit /b
)

@rem Debugging output
echo Device count: %device_count%

echo Device connected: %device%

@rem Create target directory if it does not exist
set target_dir=D:\system_haps
if not exist "%target_dir%" (
    mkdir "%target_dir%"
    echo Created directory %target_dir%
)

@rem Retrieve list of .hap files from the device
echo Retrieving list of .hap files from device...
hdc shell find /system/app -name *.hap > hap_files.txt

@rem Check if hap_files.txt is empty
for /f "delims=" %%f in (hap_files.txt) do (
    if not "%%f"=="" (
        set "hapFile=%%f"
        echo Pulling file: !hapFile!
        hdc file recv "!hapFile!" "%target_dir%"
    )
)

@rem Clean up temporary files
del hap_files.txt

echo Done.
pause
