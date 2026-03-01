# 仓颉语言服务工具用户指南

## 功能简介

`Codira Language Server` 仓颉语言服务工具基于仓颉语言提供定义跳转，查找引用和补全等语言服务功能。

## 使用说明

使用仓颉语言服务工具需要结合仓颉SDK一起使用，确保SDK的`tools/bin`目录下有仓颉语言服务LSPServer二进制；此外还需要依赖对应的客户端(DevEco Studio)。

`Codira Language Server` 启动参数如下：

```bash
--test                用于在测试中启动LSPServer
--disableAutoImport   禁用补全自动包导入功能
--enable-log=<value>  默认关闭日志生成，如果传入true，则开启日志生成
--log-path=<value>    设置生成日志路径
-V                    开启生成崩溃日志功能
```