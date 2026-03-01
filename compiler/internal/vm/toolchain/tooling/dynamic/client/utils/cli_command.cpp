/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tooling/dynamic/client/utils/cli_command.h"

namespace OHOS::ArkCompiler::Toolchain {
const std::string HELP_MSG = "usage: <command> <options>\n"
    " These are common commands list:\n"
    "  allocationtrack(at)                           allocation-track-start with options\n"
    "  allocationtrack-stop(at-stop)                 allocation-track-stop\n"
    "  heapdump(hd)                                  heapdump with options\n"
    "  heapprofiler-enable(hp-enable)                heapdump enable\n"
    "  heapprofiler-disable(hp-disable)              heapdump disable\n"
    "  sampling(sampling)                            heapprofiler sampling\n"
    "  sampling-stop(sampling-stop)                  heapprofiler sampling stop\n"
    "  collectgarbage(gc)                            heapprofiler collectgarbage\n"
    "  cpuprofile(cp)                                cpuprofile start\n"
    "  cpuprofile-stop(cp-stop)                      cpuprofile stop\n"
    "  cpuprofile-enable(cp-enable)                  cpuprofile enable\n"
    "  cpuprofile-disable(cp-disable)                cpuprofile disable\n"
    "  cpuprofile-show(cp-show)                      cpuprofile show\n"
    "  cpuprofile-setSamplingInterval(cp-ssi)        cpuprofile setSamplingInterval\n"
    "  runtime-enable(rt-enable)                     runtime enable\n"
    "  heapusage(hu)                                 runtime getHeapUsage\n"
    "  break(b)                                      break with options\n"
    "  setSymbolicBreakpoints                        setSymbolicBreakpoints\n"
    "  removeSymbolicBreakpoints                     removeSymbolicBreakpoints\n"
    "  removeBreakpointsByUrl                        removeBreakpointsByUrl\n"
    "  backtrack(bt)                                 backtrace\n"
    "  continue(c)                                   continue\n"
    "  delete(d)                                     delete with options\n"
    "  disable                                       disable\n"
    "  display                                       display\n"
    "  enable                                        debugger enable\n"
    "  finish(fin)                                   finish\n"
    "  frame(f)                                      frame\n"
    "  help(h)                                       list available commands\n"
    "  ignore(ig)                                    ignore\n"
    "  infobreakpoints(infob)                        info-breakpoints\n"
    "  infosource(infos)                             info-source\n"
    "  jump(j)                                       jump\n"
    "  list(l)                                       list\n"
    "  next(n)                                       next\n"
    "  print(p)                                      print with options\n"
    "  ptype                                         ptype\n"
    "  quit(q)                                       quit\n"
    "  run(r)                                        run\n"
    "  setvar(sv)                                    set value with options\n"
    "  step(s)                                       step\n"
    "  undisplay                                     undisplay\n"
    "  watch(wa)                                     watch\n"
    "  resume                                        resume\n"
    "  showstack(ss)                                 showstack\n"
    "  step-into(si)                                 step-into\n"
    "  step-out(so)                                  step-out\n"
    "  step-over(sov)                                step-over\n"
    "  runtime-disable                               rt-disable\n"
    "  setAsyncStackDepth                            setAsyncStackDepth\n"
    "  session-new                                   add new session\n"
    "  session-remove                                del a session\n"
    "  session-list                                  list all sessions\n"
    "  session                                       switch session\n"
    "  forall                                        command for all sessions\n"
    "  success                                       test success\n"
    "  fail                                          test fail\n";

const std::vector<std::string> cmdList = {
    "allocationtrack",
    "allocationtrack-stop",
    "heapdump",
    "heapprofiler-enable",
    "heapprofiler-disable",
    "sampling",
    "sampling-stop",
    "collectgarbage",
    "cpuprofile",
    "cpuprofile-stop",
    "cpuprofile-enable",
    "cpuprofile-disable",
    "cpuprofile-show",
    "cpuprofile-setSamplingInterval",
    "runtime-enable",
    "heapusage",
    "break",
    "setSymbolicBreakpoints",
    "removeSymbolicBreakpoints",
    "removeBreakpointsByUrl",
    "backtrack",
    "continue",
    "delete",
    "disable",
    "display",
    "enable",
    "finish",
    "frame",
    "help",
    "ignore",
    "infobreakpoints",
    "infosource",
    "jump",
    "list",
    "next",
    "print",
    "ptype",
    "run",
    "setvar",
    "step",
    "undisplay",
    "watch",
    "resume",
    "step-into",
    "step-out",
    "step-over",
    "runtime-disable",
    "setAsyncStackDepth",
    "session-new",
    "session-remove",
    "session-list",
    "session",
    "success",
    "fail"
};

ErrCode CliCommand::ExecCommand()
{
    CreateCommandMap();

    ErrCode result = OnCommand();
    return result;
}

void CliCommand::CreateCommandMap()
{
    commandMap_ = {
        {std::make_pair("allocationtrack", "at"), std::bind(&CliCommand::HeapProfilerCommand, this, "allocationtrack")},
        {std::make_pair("allocationtrack-stop", "at-stop"),
            std::bind(&CliCommand::HeapProfilerCommand, this, "allocationtrack-stop")},
        {std::make_pair("heapdump", "hd"), std::bind(&CliCommand::HeapProfilerCommand, this, "heapdump")},
        {std::make_pair("heapprofiler-enable", "hp-enable"),
            std::bind(&CliCommand::HeapProfilerCommand, this, "heapprofiler-enable")},
        {std::make_pair("heapprofiler-disable", "hp-disable"),
            std::bind(&CliCommand::HeapProfilerCommand, this, "heapprofiler-disable")},
        {std::make_pair("sampling", "sampling"), std::bind(&CliCommand::HeapProfilerCommand, this, "sampling")},
        {std::make_pair("sampling-stop", "sampling-stop"),
            std::bind(&CliCommand::HeapProfilerCommand, this, "sampling-stop")},
        {std::make_pair("collectgarbage", "gc"), std::bind(&CliCommand::HeapProfilerCommand, this, "collectgarbage")},
        {std::make_pair("cpuprofile", "cp"), std::bind(&CliCommand::CpuProfileCommand, this, "cpuprofile")},
        {std::make_pair("cpuprofile-stop", "cp-stop"),
            std::bind(&CliCommand::CpuProfileCommand, this, "cpuprofile-stop")},
        {std::make_pair("cpuprofile-enable", "cp-enable"),
            std::bind(&CliCommand::CpuProfileCommand, this, "cpuprofile-enable")},
        {std::make_pair("cpuprofile-disable", "cp-disable"),
            std::bind(&CliCommand::CpuProfileCommand, this, "cpuprofile-disable")},
        {std::make_pair("cpuprofile-show", "cp-show"),
            std::bind(&CliCommand::CpuProfileCommand, this, "cpuprofile-show")},
        {std::make_pair("cpuprofile-setSamplingInterval", "cp-ssi"),
            std::bind(&CliCommand::CpuProfileCommand, this, "cpuprofile-setSamplingInterval")},
        {std::make_pair("runtime-enable", "rt-enable"), std::bind(&CliCommand::RuntimeCommand, this, "runtime-enable")},
        {std::make_pair("heapusage", "hu"), std::bind(&CliCommand::RuntimeCommand, this, "heapusage")},
        {std::make_pair("break", "b"), std::bind(&CliCommand::BreakCommand, this, "break")},
        {std::make_pair("backtrack", "bt"), std::bind(&CliCommand::DebuggerCommand, this, "backtrack")},
        {std::make_pair("continue", "c"), std::bind(&CliCommand::DebuggerCommand, this, "continue")},
        {std::make_pair("delete", "d"), std::bind(&CliCommand::DeleteCommand, this, "delete")},
        {std::make_pair("disable", "disable"), std::bind(&CliCommand::DebuggerCommand, this, "disable")},
        {std::make_pair("display", "display"), std::bind(&CliCommand::DisplayCommand, this, "display")},
        {std::make_pair("enable", "enable"), std::bind(&CliCommand::DebuggerCommand, this, "enable")},
        {std::make_pair("finish", "fin"), std::bind(&CliCommand::DebuggerCommand, this, "finish")},
        {std::make_pair("frame", "f"), std::bind(&CliCommand::DebuggerCommand, this, "frame")},
        {std::make_pair("enable-launch-accelerate", "enable-acc"),
            std::bind(&CliCommand::DebuggerCommand, this, "enable-launch-accelerate")},
        {std::make_pair("saveAllPossibleBreakpoints", "b-new"),
            std::bind(&CliCommand::SaveAllPossibleBreakpointsCommand, this, "saveAllPossibleBreakpoints")},
        {std::make_pair("setSymbolicBreakpoints", "setSymbolicBreakpoints"),
            std::bind(&CliCommand::SetSymbolicBreakpointsCommand, this, "setSymbolicBreakpoints")},
        {std::make_pair("removeSymbolicBreakpoints", "removeSymbolicBreakpoints"),
            std::bind(&CliCommand::RemoveSymbolicBreakpointsCommand, this, "removeSymbolicBreakpoints")},
        {std::make_pair("removeBreakpointsByUrl", "removeBreakpointsByUrl"),
            std::bind(&CliCommand::RemoveBreakpointsByUrlCommand, this, "removeBreakpointsByUrl")},
    };
    CreateOtherCommandMap();
}
void CliCommand::CreateOtherCommandMap()
{
    commandMap_.insert({
        {std::make_pair("help", "h"), std::bind(&CliCommand::ExecHelpCommand, this)},
        {std::make_pair("ignore", "ig"), std::bind(&CliCommand::DebuggerCommand, this, "ignore")},
        {std::make_pair("infobreakpoints", "infob"), std::bind(&CliCommand::DebuggerCommand, this, "infobreakpoints")},
        {std::make_pair("infosource", "infos"), std::bind(&CliCommand::InfosourceCommand, this, "infosource")},
        {std::make_pair("jump", "j"), std::bind(&CliCommand::DebuggerCommand, this, "jump")},
        {std::make_pair("list", "l"), std::bind(&CliCommand::ListCommand, this, "list")},
        {std::make_pair("next", "n"), std::bind(&CliCommand::DebuggerCommand, this, "next")},
        {std::make_pair("print", "p"), std::bind(&CliCommand::PrintCommand, this, "print")},
        {std::make_pair("ptype", "ptype"), std::bind(&CliCommand::DebuggerCommand, this, "ptype")},
        {std::make_pair("run", "r"), std::bind(&CliCommand::RuntimeCommand, this, "run")},
        {std::make_pair("setvar", "sv"), std::bind(&CliCommand::DebuggerCommand, this, "setvar")},
        {std::make_pair("step", "s"), std::bind(&CliCommand::DebuggerCommand, this, "step")},
        {std::make_pair("undisplay", "undisplay"), std::bind(&CliCommand::DebuggerCommand, this, "undisplay")},
        {std::make_pair("watch", "wa"), std::bind(&CliCommand::WatchCommand, this, "watch")},
        {std::make_pair("resume", "resume"), std::bind(&CliCommand::DebuggerCommand, this, "resume")},
        {std::make_pair("showstack", "ss"), std::bind(&CliCommand::ShowstackCommand, this, "showstack")},
        {std::make_pair("step-into", "si"), std::bind(&CliCommand::StepCommand, this, "step-into")},
        {std::make_pair("step-out", "so"), std::bind(&CliCommand::StepCommand, this, "step-out")},
        {std::make_pair("step-over", "sov"), std::bind(&CliCommand::StepCommand, this, "step-over")},
        {std::make_pair("runtime-disable", "rt-disable"),
            std::bind(&CliCommand::RuntimeCommand, this, "runtime-disable")},
        {std::make_pair("setAsyncStackDepth", "setAsyncStackDepth"),
            std::bind(&CliCommand::DebuggerCommand, this, "setAsyncStackDepth")},
        {std::make_pair("session-new", "session-new"),
            std::bind(&CliCommand::SessionAddCommand, this, "session-new")},
        {std::make_pair("session-remove", "session-remove"),
            std::bind(&CliCommand::SessionDelCommand, this, "session-remove")},
        {std::make_pair("session-list", "session-list"),
            std::bind(&CliCommand::SessionListCommand, this, "session-list")},
        {std::make_pair("session", "session"),
            std::bind(&CliCommand::SessionSwitchCommand, this, "session")},
        {std::make_pair("success", "success"),
            std::bind(&CliCommand::TestCommand, this, "success")},
        {std::make_pair("fail", "fail"),
            std::bind(&CliCommand::TestCommand, this, "fail")},
    });
}

ErrCode CliCommand::HeapProfilerCommand(const std::string &cmd)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DomainManager &domainManager = session->GetDomainManager();
    HeapProfilerClient &heapProfilerClient = domainManager.GetHeapProfilerClient();
    VecStr argList = GetArgList();
    if ((cmd == "allocationtrack" || cmd == "sampling") && !argList.empty()) {
        std::cout << "This command does not need to follow a suffix" << std::endl;
        return ErrCode::ERR_FAIL;
    } else {
        if (argList.empty()) {
            argList.push_back("/data/");
            std::cout << "exe success, cmd is " << cmd << std::endl;
        } else {
            const std::string &arg = argList[0];
            std::string pathDump = arg;
            std::ifstream fileExit(pathDump.c_str());
            if (fileExit.good() && (pathDump[0] == pathDump[pathDump.size()-1]) && (GetArgList().size() == 1)) {
                std::cout << "exe success, cmd is " << cmd << std::endl;
            } else if (GetArgList().size() > 1) {
                std::cout << "The folder path may contains spaces" << std::endl;
                return ErrCode::ERR_FAIL;
            } else if (pathDump[0] != pathDump[pathDump.size()-1]) {
                std::cout << "The folder path format is incorrect :" << pathDump << std::endl;
                std::cout << "Attention: Check for symbols /" << std::endl;
                return ErrCode::ERR_FAIL;
            } else {
                std::cout << "The folder path does not exist :" << pathDump << std::endl;
                return ErrCode::ERR_FAIL;
            }
        }
    }

    bool result = heapProfilerClient.DispatcherCmd(cmd, argList[0]);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::CpuProfileCommand(const std::string &cmd)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DomainManager &domainManager = session->GetDomainManager();
    ProfilerClient &profilerClient = domainManager.GetProfilerClient();
    ProfilerSingleton &pro = session->GetProfilerSingleton();
    VecStr argList = GetArgList();
    if (cmd == "cpuprofile") {
        if (argList.empty()) {
            std::cout << "exe success, cmd is " << cmd << std::endl;
        } else {
            std::cout << "This command does not need to follow a suffix" << std::endl;
            return ErrCode::ERR_FAIL;
        }
    }
    if (cmd == "cpuprofile-show") {
        std::cout << "exe success, cmd is " << cmd << std::endl;
        pro.ShowCpuFile();
        return ErrCode::ERR_OK;
    }
    if (cmd == "cpuprofile-setSamplingInterval") {
        std::cout << "exe success, cmd is " << cmd << std::endl;
        profilerClient.SetSamplingInterval(std::atoi(GetArgList()[0].c_str()));
    }
    if (cmd == "cpuprofile-stop" && GetArgList().size() == 1) {
        pro.SetAddress(GetArgList()[0]);
        const std::string &arg = argList[0];
        std::string pathCpuPro = arg;
        std::ifstream fileExit(pathCpuPro.c_str());
        if (fileExit.good() && (pathCpuPro[0] == pathCpuPro[pathCpuPro.size()-1])) {
            std::cout << "exe success, cmd is " << cmd << std::endl;
        } else if (pathCpuPro[0] != pathCpuPro[pathCpuPro.size()-1]) {
            std::cout << "The folder path format is incorrect :" << pathCpuPro << std::endl;
            std::cout << "Attention: Check for symbols /" << std::endl;
            return ErrCode::ERR_FAIL;
        } else {
            std::cout << "The folder path does not exist :" << pathCpuPro << std::endl;
            return ErrCode::ERR_FAIL;
        }
    }
    bool result = profilerClient.DispatcherCmd(cmd);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::DebuggerCommand(const std::string &cmd)
{
    if (GetArgList().size()) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }

    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::RuntimeCommand(const std::string &cmd)
{
    if (GetArgList().size()) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    RuntimeClient &runtimeClient = session->GetDomainManager().GetRuntimeClient();
    
    result = runtimeClient.DispatcherCmd(cmd);
    if (result) {
        runtimeClient.SetObjectId("0");
    } else {
        return ErrCode::ERR_FAIL;
    }
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::BreakCommand(const std::string &cmd)
{
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    BreakPointManager &breakpointManager = session->GetBreakPointManager();
    std::vector<Breaklocation> breaklist_ = breakpointManager.Getbreaklist();
    if (GetArgList().size() == 2) { //2: two arguments
        if (!Utils::IsNumber(GetArgList()[1])) {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
        for (auto breakpoint : breaklist_) {
            if (breakpoint.url == GetArgList()[0] &&
                std::stoi(breakpoint.lineNumber) + 1 == std::stoi(GetArgList()[1])) {
                std::cout << "the breakpoint is exist" << std::endl;
                return ErrCode::ERR_FAIL;
            }
        }
        debuggerCli.AddBreakPointInfo(GetArgList()[0], std::stoi(GetArgList()[1]));
    } else {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::SetSymbolicBreakpointsCommand(const std::string &cmd)
{
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    BreakPointManager &breakpointManager = session->GetBreakPointManager();
    std::vector<Breaklocation> breaklist_ = breakpointManager.Getbreaklist();
    if (GetArgList().size() == 1) { //1: one arguments
        if (Utils::IsNumber(GetArgList()[0])) {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
        debuggerCli.AddSymbolicBreakpointInfo(GetArgList()[0]);
    } else {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::RemoveSymbolicBreakpointsCommand(const std::string &cmd)
{
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    BreakPointManager &breakpointManager = session->GetBreakPointManager();
    std::vector<Breaklocation> breaklist_ = breakpointManager.Getbreaklist();
    if (GetArgList().size() == 1) { //1: one arguments
        if (Utils::IsNumber(GetArgList()[0])) {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
        debuggerCli.AddSymbolicBreakpointInfo(GetArgList()[0]);
    } else {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::RemoveBreakpointsByUrlCommand(const std::string &cmd)
{
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    if (GetArgList().size() == 1) { //1: one arguments
        if (Utils::IsNumber(GetArgList()[0])) {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
        debuggerCli.AddUrl(GetArgList()[0]);
    } else {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::DeleteCommand(const std::string &cmd)
{
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    BreakPointManager &breakpoint = session->GetBreakPointManager();
    if (GetArgList().size() == 1) {
        if (!Utils::IsNumber(GetArgList()[0])) {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
        int tmpNum = std::stoi(GetArgList()[0]);
        size_t num = static_cast<size_t>(tmpNum);
        if (breakpoint.Getbreaklist().size() >= num && num > 0) {
            debuggerCli.AddBreakPointInfo(breakpoint.Getbreaklist()[num - 1].breakpointId, 0); // 1: breakpoinId
            breakpoint.Deletebreaklist(num);
        } else {
            std::cout << "the breakpoint is not exist" << std::endl;
            return ErrCode::ERR_FAIL;
        }
    } else {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::DisplayCommand(const std::string &cmd)
{
    if (GetArgList().size()) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    BreakPointManager &breakpointManager = session->GetBreakPointManager();
    breakpointManager.Show();
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::InfosourceCommand(const std::string &cmd)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    SourceManager &sourceManager = session->GetSourceManager();
    if (GetArgList().size() > 1) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    } else if (GetArgList().size() == 1) {
        if (!Utils::IsNumber(GetArgList()[0])) {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
        sourceManager.GetFileSource(std::stoi(GetArgList()[0]));
    } else {
        sourceManager.GetFileName();
    }
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::ListCommand(const std::string &cmd)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    SourceManager &sourceManager = session->GetSourceManager();
    WatchManager &watchManager = session->GetWatchManager();
    if (!watchManager.GetDebugState()) {
        std::cout << "Start debugging your code before using the list command" << std::endl;
        return ErrCode::ERR_FAIL;
    }
    if (GetArgList().size() > 2) { //2: two arguments
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    } else if (GetArgList().size() == 2) { //2: two arguments
        if (Utils::IsNumber(GetArgList()[0]) && Utils::IsNumber(GetArgList()[1])) {
            sourceManager.GetListSource(GetArgList()[0], GetArgList()[1]);
        } else {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
    } else if (GetArgList().size() == 1) {
        if (Utils::IsNumber(GetArgList()[0])) {
            sourceManager.GetListSource(GetArgList()[0], "");
        } else {
            OutputCommand(cmd, false);
            return ErrCode::ERR_FAIL;
        }
    } else {
        sourceManager.GetListSource("", "");
    }
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::StepCommand(const std::string &cmd)
{
    if (GetArgList().size()) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    DebuggerClient &debuggerCli = session->GetDomainManager().GetDebuggerClient();
    RuntimeClient &runtimeClient = session->GetDomainManager().GetRuntimeClient();
    runtimeClient.SetIsInitializeTree(true);
    result = debuggerCli.DispatcherCmd(cmd);
    OutputCommand(cmd, true);
    return result ? ErrCode::ERR_OK : ErrCode::ERR_FAIL;
}

ErrCode CliCommand::ShowstackCommand(const std::string &cmd)
{
    if (GetArgList().size()) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    StackManager &stackManager = session->GetStackManager();
    stackManager.ShowCallFrames();
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::PrintCommand(const std::string &cmd)
{
    size_t maxArgs = 3;
    if (GetArgList().size() > maxArgs) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    bool result = false;
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    RuntimeClient &runtimeClient = session->GetDomainManager().GetRuntimeClient();
    if (GetArgList().size() == 1) {
        if (!Utils::IsNumber(GetArgList()[0])) {
            return ErrCode::ERR_FAIL;
        }
        runtimeClient.SetIsInitializeTree(false);
        VariableManager &variableManager = session->GetVariableManager();
        int32_t objectId = variableManager.FindObjectIdWithIndex(std::stoi(GetArgList()[0]));
        runtimeClient.SetObjectId(std::to_string(objectId));
    } else if (GetArgList().size() == maxArgs - 1) {
        if (!Utils::IsNumber(GetArgList()[1])) {
            return ErrCode::ERR_FAIL;
        }
        runtimeClient.SetIsInitializeTree(false);
        VariableManager &variableManager = session->GetVariableManager();
        int32_t objectId = std::stoi(GetArgList()[1]);
        runtimeClient.SetObjectId(std::to_string(objectId));
    } else if (GetArgList().size() == maxArgs) {
        if (!Utils::IsNumber(GetArgList()[0]) || !Utils::IsNumber(GetArgList()[1]) ||
            !Utils::IsNumber(GetArgList()[maxArgs - 1])) {
            return ErrCode::ERR_FAIL;
        }
        runtimeClient.SetIsInitializeTree(false);
        VariableManager &variableManager = session->GetVariableManager();
        int32_t objectId = std::stoi(GetArgList()[0]);
        int32_t startIndex = std::stoi(GetArgList()[1]);
        int32_t groupCount = std::stoi(GetArgList()[2]);
        runtimeClient.SetObjectId(std::to_string(objectId));
        runtimeClient.SetStartIndex(startIndex);
        runtimeClient.SetGroupCount(groupCount);
    }
    result = runtimeClient.DispatcherCmd(cmd);
    if (result) {
        runtimeClient.SetObjectId("0");
    } else {
        return ErrCode::ERR_FAIL;
    }
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::WatchCommand(const std::string &cmd)
{
    if (GetArgList().size() != 1) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return ErrCode::ERR_FAIL;
    }
    WatchManager &watchManager = session->GetWatchManager();
    watchManager.AddWatchInfo(GetArgList()[0]);
    if (watchManager.GetDebugState()) {
        watchManager.SendRequestWatch(watchManager.GetWatchInfoSize() - 1, watchManager.GetCallFrameId());
    } else {
        std::cout << "Start debugging your code before using the watch command" << std::endl;
        return ErrCode::ERR_FAIL;
    }
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::SessionAddCommand([[maybe_unused]] const std::string &cmd)
{
    VecStr argList = GetArgList();
    if (argList.size() == 1) {
        if (!SessionManager::getInstance().CreateNewSession(argList[0])) {
            OutputCommand(cmd, true);
            return ErrCode::ERR_OK;
        } else {
            std::cout << "session add failed" << std::endl;
        }
    } else {
        OutputCommand(cmd, false);
    }

    return ErrCode::ERR_FAIL;
}

ErrCode CliCommand::SessionDelCommand([[maybe_unused]] const std::string &cmd)
{
    VecStr argList = GetArgList();
    if (argList.size() == 1 && Utils::IsNumber(argList[0])) {
        uint32_t sessionId = 0;
        if (Utils::StrToUInt(argList[0].c_str(), &sessionId)) {
            if (sessionId == 0) {
                std::cout << "cannot remove default session 0" << std::endl;
                return ErrCode::ERR_OK;
            }
            if (SessionManager::getInstance().DelSessionById(sessionId) == 0) {
                std::cout << "session remove success" << std::endl;
                return ErrCode::ERR_OK;
            } else {
                std::cout << "sessionId is not exist" << std::endl;
            }
        }
    } else {
        OutputCommand(cmd, false);
    }
    
    return ErrCode::ERR_FAIL;
}

ErrCode CliCommand::SessionListCommand([[maybe_unused]] const std::string &cmd)
{
    if (GetArgList().size()) {
        OutputCommand(cmd, false);
        return ErrCode::ERR_FAIL;
    }
    SessionManager::getInstance().SessionList();
    OutputCommand(cmd, true);
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::SessionSwitchCommand([[maybe_unused]] const std::string &cmd)
{
    VecStr argList = GetArgList();
    if (argList.size() == 1 && Utils::IsNumber(argList[0])) {
        uint32_t sessionId = 0;
        if (Utils::StrToUInt(argList[0].c_str(), &sessionId)) {
            if (SessionManager::getInstance().SessionSwitch(sessionId) == 0) {
                OutputCommand(cmd, true);
                return ErrCode::ERR_OK;
            } else {
                std::cout << "sessionId is not exist" << std::endl;
            }
        }
    } else {
        OutputCommand(cmd, false);
    }
    return ErrCode::ERR_FAIL;
}

ErrCode CliCommand::TestCommand(const std::string &cmd)
{
    if (cmd == "success" || cmd == "fail") {
        Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
        if (session == nullptr) {
            LOGE("get session by id %{public}u failed", sessionId_);
            return ErrCode::ERR_FAIL;
        }
        TestClient &testClient = session->GetDomainManager().GetTestClient();
        testClient.DispatcherCmd(cmd);
    } else {
        return ErrCode::ERR_FAIL;
    }
    return ErrCode::ERR_OK;
}

ErrCode CliCommand::SaveAllPossibleBreakpointsCommand(const std::string &cmd)
{
    return BreakCommand(cmd);
}

ErrCode CliCommand::ExecHelpCommand()
{
    std::cout << HELP_MSG;
    return ErrCode::ERR_OK;
}

void CliCommand::OutputCommand(const std::string &cmd, bool flag)
{
    if (flag) {
        std::cout << "exe success, cmd is " << cmd << std::endl;
    } else {
        std::cout << cmd << " parameters is incorrect" << std::endl;
    }
}

ErrCode CliCommand::OnCommand()
{
    std::map<StrPair, std::function<ErrCode()>>::iterator it;
    StrPair cmdPair;
    bool haveCmdFlag = false;

    for (it = commandMap_.begin(); it != commandMap_.end(); it++) {
        cmdPair = it->first;
        if (!strcmp(cmdPair.first.c_str(), cmd_.c_str())
            ||!strcmp(cmdPair.second.c_str(), cmd_.c_str())) {
            auto respond = it->second;
            return respond();
        }
    }

    for (unsigned int i = 0; i < cmdList.size(); i++) {
        if (!strncmp(cmdList[i].c_str(), cmd_.c_str(), std::strlen(cmd_.c_str()))) {
            haveCmdFlag = true;
            std::cout << cmdList[i] << " ";
        }
    }

    if (haveCmdFlag) {
        std::cout << std::endl;
    } else {
        ExecHelpCommand();
    }

    return ErrCode::ERR_FAIL;
}
} // namespace OHOS::ArkCompiler::Toolchain
