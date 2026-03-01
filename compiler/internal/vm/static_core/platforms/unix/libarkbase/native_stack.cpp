/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "platforms/unix/libarkbase/native_stack.h"

#include "libarkbase/os/file.h"
#include <regex>
#include <dirent.h>
#include <libarkbase/mem/mem.h>

namespace ark::os::unix::native_stack {

static constexpr int MOVE_2 = 2;      // delete kernel stack's prefix
static constexpr int STACK_TYPE = 2;  // for calling Application Not Responding stack type
static constexpr int FIND_TID = 10;   // decimal number

std::string GetNativeThreadNameForFile(pid_t tid)
{
    std::string result;
    std::ostringstream commFile;
    commFile << "/proc/self/task/" << tid << "/comm";
    if (native_stack::ReadOsFile(commFile.str(), &result)) {
        result.resize(result.size() - 1);
    } else {
        result = "<unknown>";
    }
    return result;
}

std::string BuildNumber(int count)
{
    std::ostringstream ostr;
    constexpr int STACK_FORMAT = 10;
    if (0 <= count && count < STACK_FORMAT) {
        ostr << "#0" << count;
    }
    if (count >= STACK_FORMAT) {
        ostr << "#" << count;
    }
    return ostr.str();
}

void DumpKernelStack(std::ostream &os, pid_t tid, const char *tag, bool count)
{
    if (tid == static_cast<pid_t>(thread::GetCurrentThreadId())) {
        return;
    }
    std::string kernelStack;
    std::ostringstream stackFile;
    stackFile << "/proc/self/task/" << tid << "/stack";
    if (!native_stack::ReadOsFile(stackFile.str(), &kernelStack)) {
        os << tag << "(couldn't read " << stackFile.str() << ")\n";
        return;
    }

    std::regex split("\n");
    std::vector<std::string> kernelStackFrames(
        std::sregex_token_iterator(kernelStack.begin(), kernelStack.end(), split, -1), std::sregex_token_iterator());

    if (!kernelStackFrames.empty()) {
        kernelStackFrames.pop_back();
    }

    for (size_t i = 0; i < kernelStackFrames.size(); ++i) {
        const char *kernelStackBuild = kernelStackFrames[i].c_str();
        // change the stack string, case:
        // kernel stack in linux file is : "[<0>] do_syscall_64+0x73/0x130"
        // kernel stack in Application Not Responding file is : "do_syscall_64+0x73/0x130"

        const char *removeBracket = strchr(kernelStackBuild, ']');

        if (removeBracket != nullptr) {
            kernelStackBuild = removeBracket + MOVE_2;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        // if need count, case: "do_syscall_64+0x73/0x130" --> "#00 do_syscall_64+0x73/0x130"
        os << tag;
        if (count) {
            os << BuildNumber(static_cast<int>(i));
        }
        os << kernelStackBuild << std::endl;
    }
}

void DumpUnattachedThread::AddTid(pid_t tidThread)
{
    threadManagerTids_.insert(tidThread);
}

bool DumpUnattachedThread::InitKernelTidLists()
{
    kernelTid_.clear();
    DIR *task = opendir("/proc/self/task");

    if (task == nullptr) {
        return false;
    }
    dirent *dir = nullptr;
    while ((dir = readdir(task)) != nullptr) {
        char *dirEnd = nullptr;
        auto tid = static_cast<pid_t>(strtol(dir->d_name, &dirEnd, FIND_TID));
        if (*dirEnd == 0) {
            kernelTid_.insert(tid);
        }
    }
    closedir(task);
    return true;
}

void DumpUnattachedThread::Dump(std::ostream &os, bool dumpNativeCrash, FuncUnwindstack callUnwindstack)
{
    std::set<pid_t> dumpTid;
    set_difference(kernelTid_.begin(), kernelTid_.end(), threadManagerTids_.begin(), threadManagerTids_.end(),
                   inserter(dumpTid, dumpTid.begin()));
    std::set<int>::iterator tid;
    for (tid = dumpTid.begin(); tid != dumpTid.end(); ++tid) {
        // thread_manager tid may have wrong,check again
        if (kernelTid_.count(*tid) == 0) {
            continue;
        }
        int priority = thread::GetPriority(*tid);
        os << '"' << GetNativeThreadNameForFile(*tid) << '"' << " prio=" << priority << " (not attached)\n";
        os << "  | sysTid=" << *tid << " nice=" << priority << "\n";
        DumpKernelStack(os, *tid, "  kernel: ", false);

        if (dumpNativeCrash && (callUnwindstack != nullptr)) {
            callUnwindstack(*tid, os, STACK_TYPE);
        }
        os << "\n";
    }
}

bool ReadOsFile(const std::string &fileName, std::string *result)
{
    ark::os::unix::file::File cmdfile = ark::os::file::Open(fileName, ark::os::file::Mode::READONLY);
    ark::os::file::FileHolder fholder(cmdfile);
    constexpr size_t BUFF_SIZE = 8_KB;
    std::vector<char> buffer(BUFF_SIZE);
    auto res = cmdfile.Read(&buffer[0], buffer.size());
    if (res) {
        result->append(&buffer[0], res.Value());
        return true;
    }
    return false;
}

bool WriterOsFile(const void *buffer, size_t count, int fd)
{
    ark::os::unix::file::File myfile(fd);
    ark::os::file::FileHolder fholder(myfile);
    return myfile.WriteAll(buffer, count);
}

std::string GetPrimitiveName(char descriptor)
{
    const char *primitiveName = "";
    switch (descriptor) {
        case 'Z':
            primitiveName = "boolean";
            break;
        case 'B':
            primitiveName = "byte";
            break;
        case 'C':
            primitiveName = "char";
            break;
        case 'S':
            primitiveName = "short";
            break;
        case 'I':
            primitiveName = "int";
            break;
        case 'J':
            primitiveName = "long";
            break;
        case 'F':
            primitiveName = "float";
            break;
        case 'D':
            primitiveName = "double";
            break;
        case 'V':
            primitiveName = "void";
            break;
        default:
            break;
    }

    return primitiveName;
}

std::string ChangeJaveStackFormat(const char *descriptor)
{
    if (descriptor == nullptr || strlen(descriptor) < 1) {
        LOG(ERROR, RUNTIME) << "Invalid descriptor";
        return "";
    }

    if (descriptor[0] == 'L') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::string str(descriptor);
        size_t end = str.find_last_of(';');
        if (end == std::string::npos) {
            LOG(ERROR, RUNTIME) << "Invalid descriptor: no scln at end";
            return "";
        }
        std::string javaName = str.substr(1, end - 1);  // Remove 'L' and ';'
        std::replace(javaName.begin(), javaName.end(), '/', '.');
        return javaName;
    }

    if (descriptor[0] == '[') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::string javaName(descriptor);
        std::replace(javaName.begin(), javaName.end(), '/', '.');
        return javaName;
    }

    return GetPrimitiveName(descriptor[0]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

}  // namespace ark::os::unix::native_stack
