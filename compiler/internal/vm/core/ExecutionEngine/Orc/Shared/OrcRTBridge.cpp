//===------ OrcRTBridge.cpp - Executor functions for bootstrap -----===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/Orc/Shared/OrcRTBridge.h"

namespace vm::core {
namespace orc {
namespace rt {

const char *SimpleExecutorDylibManagerInstanceName =
    "__llvm_orc_SimpleExecutorDylibManager_Instance";
const char *SimpleExecutorDylibManagerOpenWrapperName =
    "__llvm_orc_SimpleExecutorDylibManager_open_wrapper";
const char *SimpleExecutorDylibManagerResolveWrapperName =
    "__llvm_orc_SimpleExecutorDylibManager_resolve_wrapper";

const char *SimpleExecutorMemoryManagerInstanceName =
    "__llvm_orc_SimpleExecutorMemoryManager_Instance";
const char *SimpleExecutorMemoryManagerReserveWrapperName =
    "__llvm_orc_SimpleExecutorMemoryManager_reserve_wrapper";
const char *SimpleExecutorMemoryManagerInitializeWrapperName =
    "__llvm_orc_SimpleExecutorMemoryManager_initialize_wrapper";
const char *SimpleExecutorMemoryManagerDeinitializeWrapperName =
    "__llvm_orc_SimpleExecutorMemoryManager_deinitialize_wrapper";
const char *SimpleExecutorMemoryManagerReleaseWrapperName =
    "__llvm_orc_SimpleExecutorMemoryManager_release_wrapper";

const char *ExecutorSharedMemoryMapperServiceInstanceName =
    "__llvm_orc_ExecutorSharedMemoryMapperService_Instance";
const char *ExecutorSharedMemoryMapperServiceReserveWrapperName =
    "__llvm_orc_ExecutorSharedMemoryMapperService_Reserve";
const char *ExecutorSharedMemoryMapperServiceInitializeWrapperName =
    "__llvm_orc_ExecutorSharedMemoryMapperService_Initialize";
const char *ExecutorSharedMemoryMapperServiceDeinitializeWrapperName =
    "__llvm_orc_ExecutorSharedMemoryMapperService_Deinitialize";
const char *ExecutorSharedMemoryMapperServiceReleaseWrapperName =
    "__llvm_orc_ExecutorSharedMemoryMapperService_Release";

const char *MemoryWriteUInt8sWrapperName =
    "__llvm_orc_bootstrap_mem_write_uint8s_wrapper";
const char *MemoryWriteUInt16sWrapperName =
    "__llvm_orc_bootstrap_mem_write_uint16s_wrapper";
const char *MemoryWriteUInt32sWrapperName =
    "__llvm_orc_bootstrap_mem_write_uint32s_wrapper";
const char *MemoryWriteUInt64sWrapperName =
    "__llvm_orc_bootstrap_mem_write_uint64s_wrapper";
const char *MemoryWritePointersWrapperName =
    "__llvm_orc_bootstrap_mem_write_pointers_wrapper";
const char *MemoryWriteBuffersWrapperName =
    "__llvm_orc_bootstrap_mem_write_buffers_wrapper";

const char *MemoryReadUInt8sWrapperName =
    "__llvm_orc_bootstrap_mem_read_uint8s_wrapper";
const char *MemoryReadUInt16sWrapperName =
    "__llvm_orc_bootstrap_mem_read_uint16s_wrapper";
const char *MemoryReadUInt32sWrapperName =
    "__llvm_orc_bootstrap_mem_read_uint32s_wrapper";
const char *MemoryReadUInt64sWrapperName =
    "__llvm_orc_bootstrap_mem_read_uint64s_wrapper";
const char *MemoryReadPointersWrapperName =
    "__llvm_orc_bootstrap_mem_read_pointers_wrapper";
const char *MemoryReadBuffersWrapperName =
    "__llvm_orc_bootstrap_mem_read_buffers_wrapper";
const char *MemoryReadStringsWrapperName =
    "__llvm_orc_bootstrap_mem_read_strings_wrapper";

const char *RegisterEHFrameSectionAllocActionName =
    "llvm_orc_registerEHFrameAllocAction";
const char *DeregisterEHFrameSectionAllocActionName =
    "llvm_orc_deregisterEHFrameAllocAction";

const char *RegisterJITLoaderGDBAllocActionName =
    "llvm_orc_registerJITLoaderGDBAllocAction";

const char *RunAsMainWrapperName = "__llvm_orc_bootstrap_run_as_main_wrapper";
const char *RunAsVoidFunctionWrapperName =
    "__llvm_orc_bootstrap_run_as_void_function_wrapper";
const char *RunAsIntFunctionWrapperName =
    "__llvm_orc_bootstrap_run_as_int_function_wrapper";

} // end namespace rt
namespace rt_alt {
const char *UnwindInfoManagerRegisterActionName =
    "orc_rt_alt_UnwindInfoManager_register";
const char *UnwindInfoManagerDeregisterActionName =
    "orc_rt_alt_UnwindInfoManager_deregister";
} // end namespace rt_alt
} // end namespace orc
} // end namespace vm::core
