# Steps to debug an ArkTS application with Inspector

1. Build the application (along with ArkTS standard library if required)
```
es2panda --extension=ets \
         [--arktsconfig path/to/your/arktsconfig.json] \
         application.ets

ninja etsstdlib
```
2. Build the ArkInspector plugin:
```
ninja arkinspector
```
3. Start the application:
```
ark --load-runtimes=ets \
    --interpreter-type=cpp \
    --boot-panda-files=/path/to/build/plugins/ets/etsstdlib.abc \
    --debugger-library-path=/path/to/build/lib/libarkinspector.so \
    --debugger-break-on-start \
    /path/to/application.abc ETSGLOBAL::main
```
4. Connect via Browser with the following link:
```
devtools://devtools/bundled/js_app.html?ws=//IP_ADDR:19015
```
where IP_ADDR is the address of the debuggee machine.
