# 调试调优工具介绍<a name="section465421746"></a>

## 目录

-   [调试<a name="section41656121654457"></a>](#调试)
-   [调优<a name="section135464351324156"></a>](#调优)

## 调试<a name="section41656121654457"></a>
调试(Debugger)提供了断点调试，单步调试，watch表达式或变量等基本调试功能并支持多实例调试，热重载，混合调试等特性。开发者可以通过以调试模式启动或attach两种方式调试ArkTS应用。
### 断点调试
- 普通断点
- 条件断点
- 异常断点，异常断点可选择仅断在未捕获异常或断在所有异常
### 单步调试
支持step into, step out, step over, continue等调试时的执行方式。

### Watch功能
结合[DevEco Studio](https://developer.harmonyos.com/cn/develop/deveco-studio)可监听某个变量或多个变量的表达式在每次停顿时的值，也可在每次停顿时给某个变量赋值。

### 多实例调试
结合DevEco Studio可同时调试多个ArkCompiler虚拟机的实例，目前主要应用场景在调试主线程及多个worker子线程。

### 热重载
热重载(HotReload)提供了一种无需重启应用，保留页面状态，文件级实时给应用打补丁的调试手段。在修改量较小时，开发者可以通过热重载用较短的时间验证自己的修改是否有效，提升开发效率。

### 混合调试
混合调试特性支持开发者在调试时从ArkTS函数跳转到调用的C++函数，并提供ArkTS和C++函数调用的堆栈。混合调试支持下，开发者调试同时存在ArkTS和C++的应用无需再重启调试切换调试类型(Debug Type)，整体调试体验将更加流畅。

## 调优<a name="section135464351324156"></a>
### 性能调优
性能调优(CPUProfiler)通过对栈的采样或在函数中插桩两种方式可以提供ArkTS/C++函数以及napi built-in库函数的时间消耗，调用频率等数据。开发者可以通过函数的时间占比饼图快速定位到某个场景下负载较高的函数并做出针对性的优化。

### 内存调优
内存调优(HeapProfiler)提供了 **内存快照** 和 **分配跟踪** 两种用于虚拟机堆内存分析/调优的工具。

- 内存快照(Heap SnapShot Dump)可以生成堆内存的快照并通过DevEco Studio提供对象数据的可视化分析，方便开发者分类统计内存占用。开发者可以查看对象属性的值，对象之间的引用关系和距离根节点被引用的层数。

- 分配跟踪(Allocation Tracker)通过在分配对象的函数插桩可以录制一段时间内虚拟机分配对象的堆栈和消耗时间等数据。

### 附录
[DevEco Studio调试功能使用文档](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/ide_debug_device-0000001053822404-V3?catalogVersion=V3)

[DevEco Studio Profiler功能使用文档](https://developer.harmonyos.com/cn/docs/documentation/doc-guides-V3/ide_profiler_introduce-0000001193577970-V3?catalogVersion=V3)