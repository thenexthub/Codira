
## Static analysis: Scan API

### Requirement

Suppose that we have a list of privacy interfaces, and we are compiling an APP. We want to detect all the functions/methods that invoke these interfaces.

APP source code example
`modules/src3.js:`
```ts
import {
    geolocation
} from '@ohos.geolocation';

export function src3UseLocation() {
    geolocation.getCurrentLocation();
}

function src3UseOther() {
    geolocation.getLastLocation();
}

export class Cat {
    catUseLocation() {
        geolocation.getCurrentLocation();
    }
    catUseUseOther() {
        geolocation.getLastLocation();
    }
}
```

`api_scanner.js:`
```ts
class Person {
    personUseLocation() {
        geolocation.getCurrentLocation();
    }
    personUseOther() {
        geolocation.getLastLocation();
    }
}
```

### Input

List of API info: apiList example
```cpp
{
// {api_file_name, namespace_name, api_name}
{"@ohos.geolocation", "geolocation", "getCurrentLocation"}
//...
}
```

### Expected output

List of API usage info
```cpp
{
  APIInfo: {
    source: "@ohos.geolocation",
    objName: "geolocation",
    propName: "getCurrentLocation",
  },
  Usages: {
    {
      source: "modules/src3",
      funcName: "src3UseLocation",
      indexOfAPI: 2,
    },
    {
      source: "modules/src3",
      funcName: "Cat.catUseLocation",
      indexOfAPI: 2,
    },
    {
      source: "api_scanner",
      funcName: "Person.personUseLocation",
      indexOfAPI: 2,
    },
  }
}
```
