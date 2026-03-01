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

#include "js_native_api_v8_inspector.h"

#include <unistd.h>

#include "inspector_socket_server.h"
#include "inspector_utils.h"
#include "js_native_api_v8.h"
#include "jsvm_mutex.h"
#include "libplatform/libplatform.h"
#include "v8-inspector.h"
#include "v8-platform.h"

#ifdef __POSIX__
#include <climits> // PTHREAD_STACK_MIN
#include <pthread.h>
#endif // __POSIX__

#include <algorithm>
#include <cstring>
#include <securec.h>
#include <sstream>
#include <unordered_map>
#include <vector>
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD003F00

namespace v8impl {

namespace {
using jsvm::ConditionVariable;
using jsvm::Mutex;
using jsvm::inspector::StringViewToUtf8;
using jsvm::inspector::Utf8ToStringView;
using v8_inspector::StringBuffer;
using v8_inspector::StringView;

class MainThreadInterface;

class Request {
public:
    virtual void Call(MainThreadInterface*) = 0;
    virtual ~Request() = default;
};

class Deletable {
public:
    virtual ~Deletable() = default;
};

using MessageQueue = std::deque<std::unique_ptr<Request>>;

class MainThreadHandle : public std::enable_shared_from_this<MainThreadHandle> {
public:
    explicit MainThreadHandle(MainThreadInterface* mainThread) : mainThread(mainThread) {}
    ~MainThreadHandle()
    {
        Mutex::ScopedLock scopedLock(blockLock);
        CHECK_NULL(mainThread); // mainThread should have called Reset
    }
    std::unique_ptr<InspectorSession> Connect(std::unique_ptr<InspectorSessionDelegate> delegate, bool preventShutdown);
    int NewObjectId()
    {
        return ++nextObjectId;
    }
    bool Post(std::unique_ptr<Request> request);

private:
    void Reset();

    MainThreadInterface* mainThread;
    Mutex blockLock;
    int nextSessionId = 0;
    std::atomic_int nextObjectId = { 1 };

    friend class MainThreadInterface;
};

class MainThreadInterface : public std::enable_shared_from_this<MainThreadInterface> {
public:
    explicit MainThreadInterface(Agent* agent);
    ~MainThreadInterface();

    void DispatchMessages();
    void Post(std::unique_ptr<Request> request);
    bool WaitForFrontendEvent();
    std::shared_ptr<MainThreadHandle> GetHandle();
    Agent* InspectorAgent()
    {
        return agent;
    }
    void AddObject(int handle, std::unique_ptr<Deletable> object);
    Deletable* GetObject(int id);
    Deletable* GetObjectIfExists(int id);
    void RemoveObject(int handle);

private:
    MessageQueue requests;
    Mutex requestsLock; // requests live across threads
    // This queue is to maintain the order of the messages for the cases
    // when we reenter the DispatchMessages function.
    MessageQueue dispatchingMessageQueue;
    bool dispatchingMessages = false;
    ConditionVariable incomingMessageCond;
    // Used from any thread
    Agent* const agent;
    std::shared_ptr<MainThreadHandle> handle;
    std::unordered_map<int, std::unique_ptr<Deletable>> managedObjects;
};

template<typename T>
class DeletableWrapper : public Deletable {
public:
    explicit DeletableWrapper(std::unique_ptr<T> object) : object(std::move(object)) {}
    ~DeletableWrapper() override = default;

    static T* Get(MainThreadInterface* thread, int id)
    {
        return static_cast<DeletableWrapper<T>*>(thread->GetObject(id))->object.get();
    }

private:
    std::unique_ptr<T> object;
};

template<typename T>
std::unique_ptr<Deletable> WrapInDeletable(std::unique_ptr<T> object)
{
    return std::make_unique<DeletableWrapper<T>>(std::move(object));
}

template<typename Factory>
class CreateObjectRequest : public Request {
public:
    CreateObjectRequest(int objectId, Factory factory) : objectId(objectId), factory(std::move(factory)) {}

    void Call(MainThreadInterface* thread) override
    {
        thread->AddObject(objectId, WrapInDeletable(factory(thread)));
    }

private:
    int objectId;
    Factory factory;
};

template<typename Factory>
std::unique_ptr<Request> NewCreateRequest(int objectId, Factory factory)
{
    return std::make_unique<CreateObjectRequest<Factory>>(objectId, std::move(factory));
}

class DeleteRequest : public Request {
public:
    explicit DeleteRequest(int objectId) : objectId(objectId) {}

    void Call(MainThreadInterface* thread) override
    {
        thread->RemoveObject(objectId);
    }

private:
    int objectId;
};

template<typename Target, typename Fn>
class CallRequest : public Request {
public:
    CallRequest(int id, Fn fn) : id(id), fn(std::move(fn)) {}

    void Call(MainThreadInterface* thread) override
    {
        fn(DeletableWrapper<Target>::Get(thread, id));
    }

private:
    int id;
    Fn fn;
};

template<typename T>
class AnotherThreadObjectReference {
public:
    AnotherThreadObjectReference(std::shared_ptr<MainThreadHandle> thread, int objectId)
        : thread(thread), objectId(objectId)
    {}

    template<typename Factory>
    AnotherThreadObjectReference(std::shared_ptr<MainThreadHandle> thread, Factory factory)
        : AnotherThreadObjectReference(thread, thread->NewObjectId())
    {
        thread->Post(NewCreateRequest(objectId, std::move(factory)));
    }

    AnotherThreadObjectReference(AnotherThreadObjectReference&) = delete;

    AnotherThreadObjectReference& operator=(const AnotherThreadObjectReference&) = delete;

    ~AnotherThreadObjectReference()
    {
        // Disappearing thread may cause a memory leak
        thread->Post(std::make_unique<DeleteRequest>(objectId));
    }

    template<typename Fn>
    void Call(Fn fn) const
    {
        using Request = CallRequest<T, Fn>;
        thread->Post(std::make_unique<Request>(objectId, std::move(fn)));
    }

    template<typename Arg>
    void Call(void (T::*fn)(Arg), Arg argument) const
    {
        Call(std::bind(Apply<Arg>, std::placeholders::_1, fn, std::move(argument)));
    }

private:
    // This has to use non-const reference to support std::bind with non-copyable
    // types
    template<typename Argument>
    static void Apply(T* target, void (T::*fn)(Argument), Argument& argument) /* NOLINT (runtime/references) */
    {
        (target->*fn)(std::move(argument));
    }

    std::shared_ptr<MainThreadHandle> thread;
    const int objectId;
};

class MainThreadSessionState {
public:
    MainThreadSessionState(MainThreadInterface* thread, bool preventShutdown)
        : thread(thread), preventShutdown(preventShutdown)
    {}

    static std::unique_ptr<MainThreadSessionState> Create(MainThreadInterface* thread, bool preventShutdown)
    {
        return std::make_unique<MainThreadSessionState>(thread, preventShutdown);
    }

    void Connect(std::unique_ptr<InspectorSessionDelegate> delegate)
    {
        Agent* agent = thread->InspectorAgent();
        if (agent != nullptr) {
            session = agent->Connect(std::move(delegate), preventShutdown);
        }
    }

    void Dispatch(std::unique_ptr<StringBuffer> message)
    {
        session->Dispatch(message->string());
    }

private:
    MainThreadInterface* thread;
    bool preventShutdown;
    std::unique_ptr<InspectorSession> session;
};

class CrossThreadInspectorSession : public InspectorSession {
public:
    CrossThreadInspectorSession(int id,
                                std::shared_ptr<MainThreadHandle> thread,
                                std::unique_ptr<InspectorSessionDelegate> delegate,
                                bool preventShutdown)
        : state(thread, std::bind(MainThreadSessionState::Create, std::placeholders::_1, preventShutdown))
    {
        state.Call(&MainThreadSessionState::Connect, std::move(delegate));
    }

    void Dispatch(const StringView& message) override
    {
        state.Call(&MainThreadSessionState::Dispatch, StringBuffer::create(message));
    }

private:
    AnotherThreadObjectReference<MainThreadSessionState> state;
};

class ThreadSafeDelegate : public InspectorSessionDelegate {
public:
    ThreadSafeDelegate(std::shared_ptr<MainThreadHandle> thread, int objectId)
        : thread(thread), delegate(thread, objectId)
    {}

    void SendMessageToFrontend(const v8_inspector::StringView& message) override
    {
        delegate.Call([m = StringBuffer::create(message)](InspectorSessionDelegate* delegate) {
            delegate->SendMessageToFrontend(m->string());
        });
    }

private:
    std::shared_ptr<MainThreadHandle> thread;
    AnotherThreadObjectReference<InspectorSessionDelegate> delegate;
};

MainThreadInterface::MainThreadInterface(Agent* agent) : agent(agent) {}

MainThreadInterface::~MainThreadInterface()
{
    if (handle) {
        handle->Reset();
    }
}

void MainThreadInterface::Post(std::unique_ptr<Request> request)
{
    CHECK_NOT_NULL(agent);
    Mutex::ScopedLock scopedLock(requestsLock);
    bool needsNotify = requests.empty();
    requests.push_back(std::move(request));
    if (needsNotify) {
        std::weak_ptr<MainThreadInterface> weakSelf { shared_from_this() };
        agent->env()->RequestInterrupt([weakSelf](Environment*) {
            if (auto iface = weakSelf.lock()) {
                iface->DispatchMessages();
            }
        });
    }
    incomingMessageCond.Broadcast(scopedLock);
}

bool MainThreadInterface::WaitForFrontendEvent()
{
    // We allow DispatchMessages reentry as we enter the pause. This is important
    // to support debugging the code invoked by an inspector call, such
    // as Runtime.evaluate
    dispatchingMessages = false;
    if (dispatchingMessageQueue.empty()) {
        Mutex::ScopedLock scopedLock(requestsLock);
        while (requests.empty())
            incomingMessageCond.Wait(scopedLock);
    }
    return true;
}

void MainThreadInterface::DispatchMessages()
{
    if (dispatchingMessages) {
        return;
    }
    dispatchingMessages = true;
    bool hadMessages = false;
    do {
        if (dispatchingMessageQueue.empty()) {
            Mutex::ScopedLock scopedLock(requestsLock);
            requests.swap(dispatchingMessageQueue);
        }
        hadMessages = !dispatchingMessageQueue.empty();
        while (!dispatchingMessageQueue.empty()) {
            MessageQueue::value_type task;
            std::swap(dispatchingMessageQueue.front(), task);
            dispatchingMessageQueue.pop_front();

            v8::SealHandleScope sealHandleScope(agent->env()->isolate);
            task->Call(this);
        }
    } while (hadMessages);
    dispatchingMessages = false;
}

std::shared_ptr<MainThreadHandle> MainThreadInterface::GetHandle()
{
    if (handle == nullptr) {
        handle = std::make_shared<MainThreadHandle>(this);
    }
    return handle;
}

void MainThreadInterface::AddObject(int id, std::unique_ptr<Deletable> object)
{
    CHECK_NOT_NULL(object);
    managedObjects[id] = std::move(object);
}

void MainThreadInterface::RemoveObject(int handle)
{
    CHECK_EQ(1, managedObjects.erase(handle));
}

Deletable* MainThreadInterface::GetObject(int id)
{
    Deletable* pointer = GetObjectIfExists(id);
    // This would mean the object is requested after it was disposed, which is
    // a coding error.
    CHECK_NOT_NULL(pointer);
    return pointer;
}

Deletable* MainThreadInterface::GetObjectIfExists(int id)
{
    auto iterator = managedObjects.find(id);
    if (iterator == managedObjects.end()) {
        return nullptr;
    }
    return iterator->second.get();
}

std::unique_ptr<InspectorSession> MainThreadHandle::Connect(std::unique_ptr<InspectorSessionDelegate> delegate,
                                                            bool preventShutdown)
{
    return std::unique_ptr<InspectorSession>(
        new CrossThreadInspectorSession(++nextSessionId, shared_from_this(), std::move(delegate), preventShutdown));
}

bool MainThreadHandle::Post(std::unique_ptr<Request> request)
{
    Mutex::ScopedLock scopedLock(blockLock);
    if (!mainThread) {
        return false;
    }
    mainThread->Post(std::move(request));
    return true;
}

void MainThreadHandle::Reset()
{
    Mutex::ScopedLock scopedLock(blockLock);
    mainThread = nullptr;
}
} // namespace

namespace {
using jsvm::InspectPublishUid;
using jsvm::inspector::CheckedUvLoopClose;
using jsvm::inspector::CSPRNG;
using jsvm::inspector::FormatWsAddress;
using jsvm::inspector::GetHumanReadableProcessName;
using jsvm::inspector::InspectorSocketServer;

// K_KILL closes connections and stops the server, K_STOP only stops the server
enum class TransportAction { K_KILL, K_SEND_MESSAGE, K_STOP };

std::string ScriptPath(uv_loop_t* loop, const std::string& scriptName)
{
    std::string scriptPath;

    if (!scriptName.empty()) {
        uv_fs_t req;
        req.ptr = nullptr;
        if (0 == uv_fs_realpath(loop, &req, scriptName.c_str(), nullptr)) {
            CHECK_NOT_NULL(req.ptr);
            scriptPath = std::string(static_cast<char*>(req.ptr));
        }
        uv_fs_req_cleanup(&req);
    }

    return scriptPath;
}

// UUID RFC: https://www.ietf.org/rfc/rfc4122.txt
// Used ver 4 - with numbers
std::string GenerateID()
{
    uint16_t buffer[8];
    CHECK(CSPRNG(buffer, sizeof(buffer)));

    char uuid[256];
    int ret = snprintf_s(uuid, sizeof(uuid), sizeof(uuid) - 1, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
                         buffer[0],                     // time_low
                         buffer[1],                     // time_mid
                         buffer[2],                     // time_low
                         (buffer[3] & 0x0fff) | 0x4000, // time_hi_and_version
                         (buffer[4] & 0x3fff) | 0x8000, // clk_seq_hi clk_seq_low
                         buffer[5],                     // node
                         buffer[6], buffer[7]);
    CHECK(ret >= 0);
    return uuid;
}

class RequestToServer {
public:
    RequestToServer(TransportAction action, int sessionId, std::unique_ptr<v8_inspector::StringBuffer> message)
        : action(action), sessionId(sessionId), message(std::move(message))
    {}

    void Dispatch(InspectorSocketServer* server) const
    {
        switch (action) {
            case TransportAction::K_KILL:
                server->TerminateConnections();
                [[fallthrough]];
            case TransportAction::K_STOP:
                server->Stop();
                break;
            case TransportAction::K_SEND_MESSAGE:
                server->Send(sessionId, StringViewToUtf8(message->string()));
                break;
        }
    }

private:
    TransportAction action;
    int sessionId;
    std::unique_ptr<v8_inspector::StringBuffer> message;
};

class RequestQueue;

class RequestQueueData {
public:
    using MessageQueue = std::deque<RequestToServer>;

    explicit RequestQueueData(uv_loop_t* loop) : handle(std::make_shared<RequestQueue>(this))
    {
        int err = uv_async_init(loop, &async, [](uv_async_t* async) {
            RequestQueueData* wrapper = jsvm::inspector::ContainerOf(&RequestQueueData::async, async);
            wrapper->DoDispatch();
        });
        CHECK_EQ(0, err);
    }

    static void CloseAndFree(RequestQueueData* queue);

    void Post(int sessionId, TransportAction action, std::unique_ptr<StringBuffer> message)
    {
        Mutex::ScopedLock scopedLock(stateLock);
        bool notify = messages.empty();
        messages.emplace_back(action, sessionId, std::move(message));
        if (notify) {
            CHECK_EQ(0, uv_async_send(&async));
            incomingMessageCond.Broadcast(scopedLock);
        }
    }

    void Wait()
    {
        Mutex::ScopedLock scopedLock(stateLock);
        if (messages.empty()) {
            incomingMessageCond.Wait(scopedLock);
        }
    }

    void SetServer(InspectorSocketServer* serverParam)
    {
        server = serverParam;
    }

    std::shared_ptr<RequestQueue> GetHandle()
    {
        return handle;
    }

private:
    ~RequestQueueData() = default;

    MessageQueue GetMessages()
    {
        Mutex::ScopedLock scopedLock(stateLock);
        MessageQueue messagesQ;
        messages.swap(messagesQ);
        return messagesQ;
    }

    void DoDispatch()
    {
        if (server == nullptr) {
            return;
        }
        for (const auto& request : GetMessages()) {
            request.Dispatch(server);
        }
    }

    std::shared_ptr<RequestQueue> handle;
    uv_async_t async;
    InspectorSocketServer* server = nullptr;
    MessageQueue messages;
    Mutex stateLock; // Locked before mutating the queue.
    ConditionVariable incomingMessageCond;
};

class RequestQueue {
public:
    explicit RequestQueue(RequestQueueData* data) : data(data) {}

    void Reset()
    {
        Mutex::ScopedLock scopedLock(lock);
        data = nullptr;
    }

    void Post(int sessionId, TransportAction action, std::unique_ptr<StringBuffer> message)
    {
        Mutex::ScopedLock scopedLock(lock);
        if (data != nullptr) {
            data->Post(sessionId, action, std::move(message));
        }
    }

    bool Expired()
    {
        Mutex::ScopedLock scopedLock(lock);
        return data == nullptr;
    }

private:
    RequestQueueData* data;
    Mutex lock;
};

class IoSessionDelegate : public InspectorSessionDelegate {
public:
    explicit IoSessionDelegate(std::shared_ptr<RequestQueue> queue, int id) : requestQueue(queue), id(id) {}
    void SendMessageToFrontend(const v8_inspector::StringView& message) override
    {
        requestQueue->Post(id, TransportAction::K_SEND_MESSAGE, StringBuffer::create(message));
    }

private:
    std::shared_ptr<RequestQueue> requestQueue;
    int id;
};

// Passed to InspectorSocketServer to handle WS inspector protocol events,
// mostly session start, message received, and session end.
class InspectorIoDelegate : public jsvm::inspector::SocketServerDelegate {
public:
    InspectorIoDelegate(std::shared_ptr<RequestQueueData> queue,
                        std::shared_ptr<MainThreadHandle> mainThread,
                        const std::string& targetId,
                        const std::string& scriptPath,
                        const std::string& scriptName);
    ~InspectorIoDelegate() override = default;

    void StartSession(int sessionId, const std::string& inTargetId) override;
    void MessageReceived(int sessionId, const std::string& message) override;
    void EndSession(int sessionId) override;

    std::vector<std::string> GetTargetIds() override;
    std::string GetTargetTitle(const std::string& id) override;
    std::string GetTargetUrl(const std::string& id) override;
    void AssignServer(InspectorSocketServer* server) override
    {
        requestQueue->SetServer(server);
    }

private:
    std::shared_ptr<RequestQueueData> requestQueue;
    std::shared_ptr<MainThreadHandle> mainThread;
    std::unordered_map<int, std::unique_ptr<InspectorSession>> sessions;
    const std::string scriptName;
    const std::string scriptPath;
    const std::string targetId;
};

InspectorIoDelegate::InspectorIoDelegate(std::shared_ptr<RequestQueueData> queue,
                                         std::shared_ptr<MainThreadHandle> mainThread,
                                         const std::string& targetId,
                                         const std::string& scriptPath,
                                         const std::string& scriptName)
    : requestQueue(queue), mainThread(mainThread), scriptName(scriptName), scriptPath(scriptPath), targetId(targetId)
{}

void InspectorIoDelegate::StartSession(int sessionId, const std::string& inTargetId)
{
    auto session = mainThread->Connect(std::make_unique<IoSessionDelegate>(requestQueue->GetHandle(), sessionId), true);
    if (session) {
        sessions[sessionId] = std::move(session);
        if (fprintf(stderr, "Debugger attached.\n") < 0) {
            return;
        }
    }
}

void InspectorIoDelegate::MessageReceived(int sessionId, const std::string& message)
{
    auto session = sessions.find(sessionId);
    if (session != sessions.end()) {
        session->second->Dispatch(Utf8ToStringView(message)->string());
    }
}

void InspectorIoDelegate::EndSession(int sessionId)
{
    sessions.erase(sessionId);
}

std::vector<std::string> InspectorIoDelegate::GetTargetIds()
{
    return { targetId };
}

std::string InspectorIoDelegate::GetTargetTitle(const std::string& id)
{
    return scriptName.empty() ? GetHumanReadableProcessName() : scriptName;
}

std::string InspectorIoDelegate::GetTargetUrl(const std::string& id)
{
    return "file://" + scriptPath;
}

// static
void RequestQueueData::CloseAndFree(RequestQueueData* queue)
{
    queue->handle->Reset();
    queue->handle.reset();
    uv_close(reinterpret_cast<uv_handle_t*>(&queue->async), [](uv_handle_t* handle) {
        uv_async_t* async = reinterpret_cast<uv_async_t*>(handle);
        RequestQueueData* wrapper = jsvm::inspector::ContainerOf(&RequestQueueData::async, async);
        delete wrapper;
    });
}
} // namespace

class InspectorIo {
public:
    // Start the inspector agent thread, waiting for it to initialize.
    // Returns empty pointer if thread was not started.
    static std::unique_ptr<InspectorIo> Start(std::shared_ptr<MainThreadHandle> mainThread,
                                              const std::string& path,
                                              std::shared_ptr<ExclusiveAccess<HostPort>> hostPortParam,
                                              const jsvm::InspectPublishUid& inspectPublishUid);

    // Will block till the transport thread shuts down
    ~InspectorIo();

    void StopAcceptingNewConnections();
    std::string GetWsUrl() const;

private:
    InspectorIo(std::shared_ptr<MainThreadHandle> handle,
                const std::string& path,
                std::shared_ptr<ExclusiveAccess<HostPort>> hostPortParam,
                const jsvm::InspectPublishUid& inspectPublishUid);

    // Wrapper for agent->ThreadMain()
    static void ThreadMain(void* io);

    // Runs a uv_loop_t
    void ThreadMain();

    // This is a thread-safe object that will post async tasks. It lives as long
    // as an Inspector object lives (almost as long as an Isolate).
    std::shared_ptr<MainThreadHandle> mainThread;
    // Used to post on a frontend interface thread, lives while the server is
    // running
    std::shared_ptr<RequestQueue> requestQueue;
    std::shared_ptr<ExclusiveAccess<HostPort>> hostPort;
    jsvm::InspectPublishUid inspectPublishUid;

    // The IO thread runs its own uv_loop to implement the TCP server off
    // the main thread.
    uv_thread_t thread;

    // For setting up interthread communications
    Mutex threadStartLock;
    jsvm::ConditionVariable threadStartCondition;
    std::string scriptName;
    // May be accessed from any thread
    const std::string id;
};

// static
std::unique_ptr<InspectorIo> InspectorIo::Start(std::shared_ptr<MainThreadHandle> mainThread,
                                                const std::string& path,
                                                std::shared_ptr<ExclusiveAccess<HostPort>> hostPortParam,
                                                const InspectPublishUid& inspectPublishUid)
{
    auto io = std::unique_ptr<InspectorIo>(new InspectorIo(mainThread, path, hostPortParam, inspectPublishUid));
    if (io->requestQueue->Expired()) { // Thread is not running
        return nullptr;
    }
    return io;
}

InspectorIo::InspectorIo(std::shared_ptr<MainThreadHandle> mainThread,
                         const std::string& path,
                         std::shared_ptr<ExclusiveAccess<HostPort>> hostPortParam,
                         const InspectPublishUid& inspectPublishUid)
    : mainThread(mainThread), hostPort(hostPortParam), inspectPublishUid(inspectPublishUid), thread(), scriptName(path),
      id(GenerateID())
{
    Mutex::ScopedLock scopedLock(threadStartLock);
    CHECK_EQ(uv_thread_create(&thread, InspectorIo::ThreadMain, this), 0);
    threadStartCondition.Wait(scopedLock);
}

InspectorIo::~InspectorIo()
{
    requestQueue->Post(0, TransportAction::K_KILL, nullptr);
    int err = uv_thread_join(&thread);
    CHECK_EQ(err, 0);
}

void InspectorIo::StopAcceptingNewConnections()
{
    requestQueue->Post(0, TransportAction::K_STOP, nullptr);
}

// static
void InspectorIo::ThreadMain(void* io)
{
    static_cast<InspectorIo*>(io)->ThreadMain();
}

void InspectorIo::ThreadMain()
{
    uv_loop_t loop;
    loop.data = nullptr;
    int err = uv_loop_init(&loop);
    CHECK_EQ(err, 0);
    std::shared_ptr<RequestQueueData> queue(new RequestQueueData(&loop), RequestQueueData::CloseAndFree);
    std::string scriptPath = ScriptPath(&loop, scriptName);
    std::unique_ptr<InspectorIoDelegate> delegate(
        new InspectorIoDelegate(queue, mainThread, id, scriptPath, scriptName));
    std::string host;
    int port;
    int pid;
    {
        ExclusiveAccess<HostPort>::Scoped scopedHostPort(hostPort);
        host = scopedHostPort->GetHost();
        port = scopedHostPort->GetPort();
        pid = scopedHostPort->GetPid();
    }
    InspectorSocketServer server(std::move(delegate), &loop, std::move(host), port, inspectPublishUid, stderr, pid);
    requestQueue = queue->GetHandle();
    // Its lifetime is now that of the server delegate
    queue.reset();
    {
        Mutex::ScopedLock scopedLock(threadStartLock);
        if (server.Start()) {
            ExclusiveAccess<HostPort>::Scoped scopedHostPort(hostPort);
            scopedHostPort->SetPort(server.GetPort());
        }
        threadStartCondition.Broadcast(scopedLock);
    }
    uv_run(&loop, UV_RUN_DEFAULT);
    CheckedUvLoopClose(&loop);
}

std::string InspectorIo::GetWsUrl() const
{
    ExclusiveAccess<HostPort>::Scoped scopedHostPort(hostPort);
    return FormatWsAddress(scopedHostPort->GetHost(), scopedHostPort->GetPort(), id, true);
}

namespace {

using jsvm::inspector::TwoByteValue;

using v8::Local;
using v8::Context;
using v8::Function;
using v8::HandleScope;
using v8::Isolate;
using v8::Message;
using v8::Object;
using v8::Value;

using v8_inspector::StringBuffer;
using v8_inspector::StringView;
using v8_inspector::V8InspectorClient;
using v8_inspector::V8Inspector;

std::unique_ptr<StringBuffer> ToProtocolString(Isolate* isolate, Local<Value> value)
{
    TwoByteValue buffer(isolate, value);
    return StringBuffer::create(StringView(*buffer, buffer.GetLength()));
}

const int CONTEXT_GROUP_ID = 1;

std::string GetWorkerLabel(Environment* env)
{
    std::ostringstream result;
    result << "Worker["
           << "env->thread_id()"
           << "]";
    return result.str();
}

class ChannelImpl final : public v8_inspector::V8Inspector::Channel {
public:
    explicit ChannelImpl(const std::unique_ptr<V8Inspector>& inspector,
                         std::unique_ptr<InspectorSessionDelegate> delegate,
                         std::shared_ptr<MainThreadHandle> mainThread,
                         bool preventShutdown)
        : delegate(std::move(delegate)), preventShutdown(preventShutdown)
    {
        session =
            inspector->connect(CONTEXT_GROUP_ID, this, StringView(), V8Inspector::ClientTrustLevel::kFullyTrusted);
    }

    ~ChannelImpl() = default;

    void DispatchProtocolMessage(const StringView& message)
    {
        session->dispatchProtocolMessage(message);
    }

    void SchedulePauseOnNextStatement(const std::string& reason)
    {
        std::unique_ptr<StringBuffer> buffer = Utf8ToStringView(reason);
        session->schedulePauseOnNextStatement(buffer->string(), buffer->string());
    }

    bool PreventShutdown()
    {
        return preventShutdown;
    }

private:
    void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override
    {
        SendMessageToFrontend(message->string());
    }

    void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override
    {
        SendMessageToFrontend(message->string());
    }

    void flushProtocolNotifications() override {}

    void SendMessageToFrontend(const StringView& message)
    {
        delegate->SendMessageToFrontend(message);
    }

    void sendMessageToFrontend(const std::string& message)
    {
        SendMessageToFrontend(Utf8ToStringView(message)->string());
    }

    std::unique_ptr<InspectorSessionDelegate> delegate;
    std::unique_ptr<v8_inspector::V8InspectorSession> session;
    bool preventShutdown;
};

class SameThreadInspectorSession : public InspectorSession {
public:
    SameThreadInspectorSession(int sessionId, std::shared_ptr<InspectorClient> client)
        : sessionId(sessionId), client(client)
    {}
    ~SameThreadInspectorSession() override;
    void Dispatch(const v8_inspector::StringView& message) override;

private:
    int sessionId;
    std::weak_ptr<InspectorClient> client;
};

} // namespace

class InspectorClient : public V8InspectorClient {
public:
    explicit InspectorClient(Environment* env, bool isMain) : env(env), isMain(isMain)
    {
        client = V8Inspector::create(env->isolate, this);
        std::string name = isMain ? GetHumanReadableProcessName() : GetWorkerLabel(env);
        ContextInfo info(name);
        info.isDefault = true;
        ContextCreated(env->context(), info);
    }

    void runMessageLoopOnPause(int contextGroupId) override
    {
        waitingForResume = true;
        RunMessageLoop();
    }

    void WaitForSessionsDisconnect()
    {
        waitingForSessionsDisconnect = true;
        RunMessageLoop();
    }

    void WaitForFrontend()
    {
        waitingForFrontend = true;
        RunMessageLoop();
    }

    void maxAsyncCallStackDepthChanged(int depth) override
    {
        if (waitingForSessionsDisconnect) {
            // V8 isolate is mostly done and is only letting Inspector protocol
            // clients gather data.
            return;
        }
    }

    void ContextCreated(Local<Context> context, const ContextInfo& info)
    {
        auto nameBuffer = Utf8ToStringView(info.name);
        auto originBuffer = Utf8ToStringView(info.origin);
        std::unique_ptr<StringBuffer> auxDataBuffer;

        v8_inspector::V8ContextInfo v8info(context, CONTEXT_GROUP_ID, nameBuffer->string());
        v8info.origin = originBuffer->string();

        if (info.isDefault) {
            auxDataBuffer = Utf8ToStringView("{\"isDefault\":true}");
        } else {
            auxDataBuffer = Utf8ToStringView("{\"isDefault\":false}");
        }
        v8info.auxData = auxDataBuffer->string();

        client->contextCreated(v8info);
    }

    void ContextDestroyed(Local<Context> context)
    {
        client->contextDestroyed(context);
    }

    void quitMessageLoopOnPause() override
    {
        waitingForResume = false;
    }

    void runIfWaitingForDebugger(int contextGroupId) override
    {
        waitingForFrontend = false;
    }

    int ConnectFrontend(std::unique_ptr<InspectorSessionDelegate> delegate, bool preventShutdown)
    {
        int sessionId = nextSessionId++;
        channels[sessionId] =
            std::make_unique<ChannelImpl>(client, std::move(delegate), GetThreadHandle(), preventShutdown);
        return sessionId;
    }

    void DisconnectFrontend(int sessionId)
    {
        auto it = channels.find(sessionId);
        if (it == channels.end()) {
            return;
        }
        channels.erase(it);
        if (waitingForSessionsDisconnect && !isMain) {
            waitingForSessionsDisconnect = false;
        }
    }

    void DispatchMessageFromFrontend(int sessionId, const StringView& message)
    {
        channels[sessionId]->DispatchProtocolMessage(message);
    }

    Local<Context> ensureDefaultContextInGroup(int contextGroupId) override
    {
        return env->context();
    }

    void ReportUncaughtException(Local<Value> error, Local<Message> message)
    {
        Isolate* isolate = env->isolate;
        Local<Context> context = env->context();

        int scriptId = message->GetScriptOrigin().ScriptId();

        Local<v8::StackTrace> stackTrace = message->GetStackTrace();

        if (!stackTrace.IsEmpty() && stackTrace->GetFrameCount() > 0 &&
            scriptId == stackTrace->GetFrame(isolate, 0)->GetScriptId()) {
            scriptId = 0;
        }

        const uint8_t details[] = "Uncaught";

        client->exceptionThrown(context, StringView(details, sizeof(details) - 1), error,
                                ToProtocolString(isolate, message->Get())->string(),
                                ToProtocolString(isolate, message->GetScriptResourceName())->string(),
                                message->GetLineNumber(context).FromMaybe(0),
                                message->GetStartColumn(context).FromMaybe(0), client->createStackTrace(stackTrace),
                                scriptId);
    }

    void startRepeatingTimer(double interval, TimerCallback callback, void* data) override {}

    void cancelTimer(void* data) override {}

    void SchedulePauseOnNextStatement(const std::string& reason)
    {
        for (const auto& idChannel : channels) {
            idChannel.second->SchedulePauseOnNextStatement(reason);
        }
    }

    bool HasConnectedSessions()
    {
        for (const auto& idChannel : channels) {
            // Other sessions are "invisible" more most purposes
            if (idChannel.second->PreventShutdown()) {
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<MainThreadHandle> GetThreadHandle()
    {
        if (!interface) {
            interface = std::make_shared<MainThreadInterface>(static_cast<Agent*>(env->GetInspectorAgent()));
        }
        return interface->GetHandle();
    }

    bool IsActive()
    {
        return !channels.empty();
    }

private:
    bool ShouldRunMessageLoop()
    {
        if (waitingForFrontend) {
            return true;
        }
        if (waitingForSessionsDisconnect || waitingForResume) {
            return HasConnectedSessions();
        }
        return false;
    }

    void RunMessageLoop()
    {
        if (runningNestedLoop) {
            return;
        }

        runningNestedLoop = true;

        while (ShouldRunMessageLoop()) {
            if (interface) {
                interface->WaitForFrontendEvent();
            }
            env->RunAndClearInterrupts();
        }
        runningNestedLoop = false;
    }

    double currentTimeMS() override
    {
        return env->platform()->CurrentClockTimeMillis();
    }

    Environment* env;
    bool isMain;
    bool runningNestedLoop = false;
    std::unique_ptr<V8Inspector> client;
    std::unordered_map<int, std::unique_ptr<ChannelImpl>> channels;
    int nextSessionId = 1;
    bool waitingForResume = false;
    bool waitingForFrontend = false;
    bool waitingForSessionsDisconnect = false;
    // Allows accessing Inspector from non-main threads
    std::shared_ptr<MainThreadInterface> interface;
};

Agent::Agent(Environment* env) : parentEnv(env) {}

Agent::~Agent() = default;

bool Agent::Start(const std::string& pathParam,
                  std::shared_ptr<ExclusiveAccess<HostPort>> hostPortParam,
                  bool isMain,
                  bool waitForConnect)
{
    path = pathParam;
    CHECK_NOT_NULL(hostPortParam);
    hostPort = hostPortParam;

    client = std::make_shared<InspectorClient>(parentEnv, isMain);

    if (!StartIoThread()) {
        return false;
    }

    if (waitForConnect) {
        client->WaitForFrontend();
    }
    return true;
}

int FindAvailablePort()
{
    constexpr int startPort = 9229;
    constexpr int endPort = 9999;
    constexpr int invalidPort = -1;
    int sockfd = -1;

    for (auto port = startPort; port <= endPort; ++port) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            continue;
        }
        OHOS_CALL(fdsan_exchange_owner_tag(sockfd, 0, LOG_DOMAIN));
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            OHOS_SELECT(fdsan_close_with_tag(sockfd, LOG_DOMAIN), close(sockfd));
            if (errno == EADDRINUSE) {
                continue;
            } else {
                break;
            }
        }
        OHOS_SELECT(fdsan_close_with_tag(sockfd, LOG_DOMAIN), close(sockfd));
        return port;
    }
    return invalidPort;
}

bool Agent::Start(const std::string& pathParam, int pid)
{
    int port = FindAvailablePort();
    if (port < 0) {
        return false;
    }
    auto hostPort = std::make_shared<jsvm::ExclusiveAccess<jsvm::HostPort>>("localhost", port, pid);
    return Start(pathParam, hostPort, true, false);
}

bool Agent::StartIoThread()
{
    if (io != nullptr) {
        return true;
    }

    if (!client) {
        return false;
    }

    CHECK_NOT_NULL(client);

    io = InspectorIo::Start(client->GetThreadHandle(), path, hostPort, { false, true });
    if (io == nullptr) {
        return false;
    }
    return true;
}

void Agent::Stop()
{
    io.reset();
}

std::unique_ptr<InspectorSession> Agent::Connect(std::unique_ptr<InspectorSessionDelegate> delegate,
                                                 bool preventShutdown)
{
    if (!client) {
        return std::unique_ptr<InspectorSession> {};
    }

    CHECK_NOT_NULL(client);

    int sessionId = client->ConnectFrontend(std::move(delegate), preventShutdown);
    return std::make_unique<SameThreadInspectorSession>(sessionId, client);
}

void Agent::WaitForDisconnect()
{
    CHECK_NOT_NULL(client);
    if (client->HasConnectedSessions()) {
        if (fprintf(stderr, "Waiting for the debugger to disconnect...\n") < 0) {
            return;
        }
        if (fflush(stderr) != 0) {
            return;
        }
    }

    client->ContextDestroyed(parentEnv->context());

    if (io != nullptr) {
        io->StopAcceptingNewConnections();
        client->WaitForSessionsDisconnect();
    }
}

void Agent::PauseOnNextJavascriptStatement(const std::string& reason)
{
    client->SchedulePauseOnNextStatement(reason);
}

bool Agent::IsActive()
{
    if (client == nullptr) {
        return false;
    }
    return io != nullptr || client->IsActive();
}

void Agent::WaitForConnect()
{
    CHECK_NOT_NULL(client);
    client->WaitForFrontend();
}

SameThreadInspectorSession::~SameThreadInspectorSession()
{
    auto clientLock = client.lock();
    if (clientLock) {
        clientLock->DisconnectFrontend(sessionId);
    }
}

void SameThreadInspectorSession::Dispatch(const v8_inspector::StringView& message)
{
    auto clientLock = client.lock();
    if (clientLock) {
        clientLock->DispatchMessageFromFrontend(sessionId, message);
    }
}

} // namespace v8impl

jsvm::InspectorAgent* jsvm::InspectorAgent::New(JSVM_Env env)
{
    return new v8impl::Agent(env);
}
