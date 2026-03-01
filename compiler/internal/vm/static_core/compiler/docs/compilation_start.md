# How to start compilation

At the moment we have two compilation modes (in-place and in the background):
- in-place mode starts compilation in the same thread
- background mode starts compilation in worker thread of `taskmanager::TaskScheduler` (asynchronously) and the compilation process is divided into parts to make `taskmanager::Task` shorter


On successful completion of compilation, `ark::TaskRunner` calls callbacks on successful completion, otherwise, callbacks on failure. It also calls finalize functions in both cases (see details in `libarkbase/task_runner.h`)

Since we have a background (asynchronous) mode, we cannot get the compilation result directly (by the return value). Therefore, if we want to know the result of compilation or get a modified graph, we create a callback.

Examples:
```
bool success = true;
task_runner.AddCallbackOnFail([&success](CompilerContext<RUNNER_MODE> &compiler_ctx) { 
    success = false;
});

compiler::Graph *graph = nullptr;
task_runner.AddFinalize([&graph](CompilerContext<RUNNER_MODE> &compiler_ctx) {
    graph = compiler_ctx.GetGraph();
});
```

How to start compilation in-place:
```
compiler::InPlaceCompilerTaskRunner task_runner;

Method *method = ...;
bool is_osr = ...;
ark::ArenaAllocator allocator;
ark::ArenaAllocator graph_local_allocator;
std::string method_name = ...;

auto &task_ctx = task_runner.GetContext();
task_ctx.SetMethod(method);
task_ctx.SetOsr(is_osr);
task_ctx.SetAllocator(&allocator);
task_ctx.SetLocalAllocator(&graph_local_allocator);
task_ctx.SetMethodName(method_name);

bool compilation_result = true;
task_runner.AddCallbackOnFail([&compilation_result] { compilation_result = false; });

compiler::RuntimeInterface *runtime = ...;
bool is_dynamic = ...;
compiler::Arch arch = ...;

auto inplace_task = [runtime, is_dynamic, arch](compiler::InPlaceTaskRunner runner) {
    compiler::CompileInGraph<compiler::INPLACE_MODE>(runtime, is_dynamic, arch, std::move(runner));
};
compiler::InPlaceTaskRunner::StartTask(std::move(task_runner), inplace_task);

...

if (compilation_result == true) {
    ...
} else {
    ...
}
```

Note: the method `compiler::InPlaceTaskRunner::StartTask` has no effect and is used for compatibility with the background mode, so you can just call
`compiler::CompileInGraph<compiler::INPLACE_MODE>;` instead of `compiler::InPlaceTaskRunner::StartTask`.

The similar way for background mode:
```
taskmanager::TaskQueue task_manager_queue = ...;
compiler::BackgroundCompilerContext::CompilerThread compiler_thread = ...;
compiler::RuntimeInterface *runtime = ...;

compiler::BackgroundCompilerTaskRunner task_runner(task_manager_queue, compiler_thread.get(), runtime);

compiler::BackgroundCompilerContext::CompilerTask compiler_task = ...;
std::unique_ptr<ark::ArenaAllocator> allocator = ...;
std::unique_ptr<ark::ArenaAllocator> graph_local_allocator = ...;
std::string method_name = ...;

auto &task_ctx = task_runner.GetContext();
task_ctx.SetCompilerThread(std::move(compiler_thread));
task_ctx.SetCompilerTask(std::move(compiler_task));
task_ctx.SetAllocator(std::move(allocator));
task_ctx.SetLocalAllocator(std::move(graph_local_allocator));
task_ctx.SetMethodName(method_name);

std::promise<bool> *compilation_result_promise = ...;
std::future<bool> compilation_result_future = compilation_result_promise->get_future();

task_runner.AddCallbackOnSuccess([compilation_result_promise] {
    compilation_result_promise->set_value(true);
});

task_runner.AddCallbackOnFail([compilation_result_promise] {
    compilation_result_promise->set_value(false);
});

bool is_dynamic = ...;
compiler::Arch arch = ...;

auto background_task = [runtime, is_dynamic, arch](compiler::BackgroundCompilerTaskRunner runner) {
    compiler::CompileInGraph<compiler::BACKGROUND_MODE>(runtime, is_dynamic, arch, std::move(runner));
};
compiler::BackgroundCompilerTaskRunner::StartTask(std::move(task_runner), background_task);

...

compilation_result_future.wait();

if (compilation_result_future.get() == true) {
    ...
} else {
    ...
}
```
