# ArkTS CPU Profiler
## Enable CPU profiler
    Enable the CPU profiler when running pandafile by using the parameter "sampling-profiler-create." 

```bash
${BUILD_DIR}/bin/ark \
  --load-runtimes=ets \
  --boot-panda-files=${BUILD_DIR}/plugins/ets/etsstdlib.abc \
  --sampling-profiler-create \
  ${BUILD_DIR}/sampling_app.abc \
  ETSGLOBAL::main
```
## Usage
    The CPU profiler backend interacts with the frontend using the CDP (Chrome DevTools Protocol). The interaction commands are as follows:  

|               CDP              |  unction Specifications  |                notes               |
|--------------------------------|--------------------------|------------------------------------|
|       "Profiler.enable"        |          no-op           |Replaced by sampling-profiler-create|  
|       "Profiler.disable"       |          no-op           |             not required           |
| "Profiler.setSamplingInterval" | set sampling interval    |          default value 500 us      |
|       "Profiler.start"         |      start cpu profiler  |                                    |
|       "Profiler.stop"          |      stop cpu profiler   |         return json format data    |


    Profiler.disable is unnecessary as the Sampler object's lifecycle aligns with the runtime environment.

## Implementation
    CPU Profiler base on Sampling Profiler. For the CPU Profiler, the difference lies in that the data read from the pipe is ultimately written into the tidToSampleInfosMap_ member of the SamplesRecord object by the InspectorStreamWriter.   
    When the InspectorServer receives 'Profiler.stop', it will:
        1. Terminate data collection
        2. Process all captured data by SamplesRecord::GetAllThreadsProfileInfos()
        3. Serialize the return value of GetAllThreadsProfileInfos() into JSON format and return it to the CPU Profiler client.