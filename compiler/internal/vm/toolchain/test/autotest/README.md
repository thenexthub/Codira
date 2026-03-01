# 调试调优自动化测试框架使用指南

[TOC]

## 1 框架设计

调试调优的通用流程可抽象为如下步骤：

- IDE将应用推入设备，并拉起应用（或attach到已启动应用上）

- IDE与应用建立websocket连接
- IDE与应用之间通过websocket发送和接受调试调优相关的业务消息，消息格式遵循CDP协议

该自动化框架依据以上流程，实现了应用启动、websocket连接、消息交互等步骤的自动化，基于此可以进行调试调优用例的执行与开发

框架的目录结构与功能描述如下：

### aw

封装了自动化框架通用的Action Words，包括：

- `application.py`：应用启动、停止、安装、卸载等相关接口
- `fport.py`：执行hdc fport命令的相关接口，指定端口号、进程ID和线程ID等，用于后续的websocket连接
- `utils.py`：封装各种通用的工具类接口
- `websocket.py`：websocket连接的关键基础类，功能包括：
  - connect server和debugger server的websocket连接
  - 采用消息队列实现websocket的消息发送与接收
  - **支持多实例应用的实现**：connect server的消息接收队列持续监听addInstance消息，一旦接收到新的instance后，便会创建对应的debugger server连接
  - **异步实现**：多条websocket连接的消息接收与发送通道均实现为协程，可被提交到taskpool中作为异步任务执行
- `taskpool.py`：异步任务池，功能包括：
  - 采用队列实现任务池的生命周期控制
  - **可在任意时刻提交任务到taskpool中进行异步执行**
  - 可为每个任务添加任意多个回调函数，在任务结束后执行
- `cdp`：cdp协议的封装

### scenario_test

场景测试用例目录

- `conftest.py`：基于pytest fixture实现的测试套方法
- `test_name_number.py`：测试用例文件

### resource

用于放置编译后的应用hap包

目录下的archive.json记录了应用工程名、hap包名和bundle name的对应关系

### pytest.ini

python pytest库的配置文件

### run.py

批量执行测试用例的入口文件

### requirements.txt

自动化测试框架的python库依赖

## 2 测试用例执行

### 准备工作

- 操作系统为Windows

- 已安装hdc.exe并配置好环境变量，可通过cmd启动执行

- 测试设备已连接，且开启USB调试

- 下载自动化测试项目

  代码仓：https://gitcode.com/openharmony/arkcompiler_toolchain

  项目目录：`.\arkcompiler_toolchain\test\autotest`

- Python已安装且版本 >= 3.8.0

- 安装Python依赖

  ```powershell
  > cd .\arkcompiler_toolchain\test\autotest
  > python -m pip install -r requirements.txt [--trusted-host hostname -i Python_Package_Index]
  ```

- 下载hap包

  通过归档链接下载hap包，并将所有hap包移动到.\arkcompiler_toolchain\test\autotest\resource目录下

### 执行用例

cmd进入`.\arkcompiler_toolchain\test\autotest`目录，执行：

```python
> python .\run.py
```

用例执行过程中，cmd会实时输出用例日志信息，执行完成后，cmd会展示执行结果，同时会生成测试报告，并导出hilog日志：

- `.\arkcompiler_toolchain\test\autotest\report\report.html`：测试报告

- `.\arkcompiler_toolchain\test\autotest\report\xxx.hilog.txt`：每个测试用例对应一份debug级别的hilog日志

- `.\arkcompiler_toolchain\test\autotest\report\faultlogger`：用例执行过程中产生的错误日志

## 3 测试用例开发

### 测试应用

根据测试用例场景，确定并开发你的测试应用，编译后将应用hap包放在`.\arkcompiler_toolchain\test\autotest\resource`目录下

### 测试套

测试套以pytest fixture形式开发，对应于`.\arkcompiler_toolchain\test\autotest\scenario_test\conftest.py`中的一个方法；

对于一个确定的测试应用，和一套固定的配置信息，我们可以将他们抽象出来作为一个fixture，基于该fixture我们可以开发多个测试用例

fixture写法举例如下：

```python
@pytest.fixture(scope='class')		# 该装饰器指定下面的函数是一个fixture
def test_suite_debug_01():			# fixture名称
    logging.info('running test_suite_debug_01')
	
    bundle_name = 'com.example.worker'
    hap_name = 'MyApplicationWorker.hap'

    config = {'connect_server_port': 15678,
              'debugger_server_port': 15679,
              'bundle_name': bundle_name,
              'hap_path': rf'{os.path.dirname(__file__)}\..\resource\{hap_name}'}
	
     # 将应用推入设备，并以-D模式启动
    pid = Application.launch_application(config['bundle_name'], config['hap_path'], start_mode='-D')
    assert pid != 0, logging.error(f'Pid of {hap_name} is 0!')
    config['pid'] = pid
	
    # hdc fport
    Fport.clear_fport()
    Fport.fport_connect_server(config['connect_server_port'], config['pid'], config['bundle_name'])
	
    # 实例化WebSocket对象
    config['websocket'] = WebSocket(config['connect_server_port'], config['debugger_server_port'])
	
    # 实例化TaskPool对象
    config['taskpool'] = TaskPool()

    return config

```

样例参考：`.\arkcompiler_toolchain\test\autotest\scenario_test\conftest.py => test_suite_debug_01`

### 测试用例

测试用例放置在`.\arkcompiler_toolchain\test\autotest\scenario_test\`目录下，每个.py文件对应一个测试用例，命名规则为`test\_测试项\_编号.py`

测试用例代码文件结构如下：

```python
@pytest.mark.debug
@pytest.mark.timeout(40)
class TestDebug01:
    def setup_method(self):
        pass
    
    def teardown_method(self):
        pass
    
    def test(self, test_suite_debug_01):
        pass
    
    async def procedure(self, websocket):
        pass

```

- 装饰器`@pytest.mark.debug`标明该用例是一个测试debug功能的用例，debug可以改为其它任意名字，如cpu_profiler

- 装饰器`@pytest.mark.timeout(40)`标明该用例必须在40s内执行完毕，否则不通过

- `class TestDebug01` 是该测试用例对应的测试类，测试类必须以**Test**开头

- `setup_class` 方法在用例开始执行前被执行，其中放置一些需要初始化的内容

- `teardown_clas` 方法在用例结束执行后被执行，其中放置一些需要结束或清理的内容

- `test` 方法是测试用例执行的入口，注意以下几点：

  - 第二个参数为fixture，不仅可以标识该用例属于哪个测试套，还可以获取该fixture执行后的返回结果；在test方法被执行前，fixture会首先被执行

  - 通过下面的代码，将websocket中封装的main task提交到taskpool，等待所有任务结束并捕获异常

    ```Python
    taskpool.submit(websocket.main_task(taskpool, websocket, self.procedure, pid))
    taskpool.await_taskpool()
    taskpool.task_join()
    if taskpool.task_exception:
        raise taskpool.task_exception
    ```

- **`procedure`方法**：测试步骤和预期结果执行的位置，在test方法中，procedure作为参数被提交到taskpool中，因此其同样作为一个**异步任务**在执行

当我们新增一个测试用例时，主要工作就是在procedure方法中放置我们的测试步骤和预期结果。首先，我们可以手动执行一次测试用例，并抓出connect server、debugger server交互的日志信息。此后，我们可以对照日志信息在procedure方法中添加内容，每一次消息发送可以当作一个测试步骤，每一次消息接收可以当作一次预期结果

例如：

```Python
################################################################################################################
# main thread: Debugger.getPossibleAndSetBreakpointByUrl
################################################################################################################
id = next(self.id_generator)	# 获取此次发送消息的id

# 构造断点信息
locations = [debugger.BreakLocationUrl(url='entry|entry|1.0.0|src/main/ets/pages/Index.ts', line_number=22),
             debugger.BreakLocationUrl(url='entry|entry|1.0.0|src/main/ets/pages/Index.ts', line_number=26)]

# 发送消息并获取返回值，调用cdp库中的debugger.get_possible_and_set_breakpoint_by_url(locations)可以自动拼接cdp消息
response = await communicate_with_debugger_server(main_thread_instance_id,
                                                  main_thread_to_send_queue,
                                                  main_thread_received_queue,
                                                  debugger.get_possible_and_set_breakpoint_by_url(locations),
                                                  id)

# 判断返回结果是否符合预期
response = json.loads(response)
assert response['id'] == id
assert response['result']['locations'][0]['id'] == 'id:22:0:entry|entry|1.0.0|src/main/ets/pages/Index.ts'
assert response['result']['locations'][1]['id'] == 'id:26:0:entry|entry|1.0.0|src/main/ets/pages/Index.ts'
```

注意：

- 建议在测试类声明后，用类注释写明该测试用例的内容；
- 每个测试步骤和预期结果之间，用上述注释方式隔开，并标明此次测试步骤的操作；
- 对于aw中未实现的CDP协议，请封装在aw/cdp目录下对应的domain文件中

样例参考：`.\arkcompiler_toolchain\test\autotest\scenario_test\test_debug_01.py`

