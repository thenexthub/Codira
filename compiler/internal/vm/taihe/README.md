# Taihe: 多语言系统接口编程模型

## Taihe 是什么

Taihe 提供了简单易用的 API 发布和消费机制。

对于 API 发布方，Taihe 可以轻松地描述要发布的接口。
```ts
// idl/integer.arithmetic.taihe

struct DivModResult {
    quo: i32;
    rem: i32;
}

function divmod_i32(a: i32, b: i32): DivModResult;
```

对于 API 的发布和消费方，Taihe 生成各语言的绑定，提供原生的开发体验。
```c++
// 发布 API 为 libinteger.so，源码位于 author/integer.arithmetic.impl.cpp

#include "integer.arithmetic.impl.hpp"

integer::arithmetic::DivModResult ohos_int_divmod(int32_t a, int32_t b) {
  return {
      .quo = a / b,
      .rem = a % b,
  };
}

TH_EXPORT_CPP_API_divmod_i32(ohos_int_divmod)
```

Taihe 将 API 的发布方和消费方在二进制级别隔离，允许二者在闭源的情况下独立升级。
```c++
// 使用 libinteger.so 编写用户应用

#include "integer.arithmetic.abi.hpp"
#include <cstdio>

using namespace integer;

int main() {
  auto [quo, rem] = arithmetic::divmod_i32(a, b);
  printf("q=%d r=%d\n", quo, rem);
  return 0;
}
```

## 用户指南

- [Taihe 工具的环境配置及使用](docs/UserSetup.md)
- [书写 IDL 文件](docs/DSL.md)
- [Taihe 数据类型](docs/Types.md)
- [Taihe C++ 用户文档](docs/CppUserDoc.md)
- [Taihe ANI 用户文档](docs/AniUserDoc.md)

关于 ANI 开发的更多教程可以参考 [Taihe ANI CookBook](cookbook/README.md).

## 开发指南

- [搭建开发环境](docs/DevSetup.md)
- [整体设计](docs/Architecture.md)
- [Interface 的二进制标准](docs/InterfaceABI.md)
- [注解系统设计文档](docs/AttributeDesign.md)
