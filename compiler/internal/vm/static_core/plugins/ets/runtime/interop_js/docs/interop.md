# 混合应用编译支持开发者提供interop声明文件或胶水代码
## 背景
ArkTS1.1和ArkTS1.2混合应用中，混合应用场景，一个编译器只能接受一种语法规范的文件，因此需要interop声明文件和胶水代码参与编译检查。当前这两部分代码都通过declgen工具自动生成。由于各种场景的复杂性，存在interop转换规格识别不全和工具自动转换能力有限等问题。为了不阻塞开发，支持优先使用开发者提供interop声明文件和胶水代码来替代自动生成的文件。

## 备份声明文件和胶水代码

手写声明文件和胶水代码放在build目录下。先由declgen自动生成，开发者手动修改build目录下有问题的interop声明文件和胶水代码。

- 声明文件的具体生成路径为 `build\default\intermediates\declgen\default\declgenV1\...\test.d.ets`;

- 胶水代码的具体生成路径为 `build\default\intermediates\declgen\default\declgenBridgeCode\...\test.ts`。

### 增量编译场景
1.源码不变，interop声明文件/胶水代码未改，不自动生成声明文件和胶水代码；

2.源码不变，interop声明文件/胶水代码被改，不自动生成声明文件和胶水代码，新增判断：当源文件与声明文件/胶水代码均被修改时，将声明文件和胶水代码进行备份；

3.源码修改，interop声明文件/胶水代码未改，自动生成声明文件和胶水代码；

4.源码修改，interop声明文件/胶水代码被改，自动生成声明文件和胶水代码。declgen新生成的声明文件和胶水代码可能导致之前已修改的interop声明文件/胶水代码被覆盖，因此进行一次备份，备份文件在同级目录，使用固定文件名，${fileName}.backup。

