# KV
A simple KV Store for N64Recomp projects

## Usage
> Note, this interface may change in the future, or I may provide additional shorthands for getting/setting primitive datatypes

Add the library to your `mod.toml`
```diff
dependencies = []
native_libraries = [
+    { name = "ProxyRecomp_KV", funcs = [
+        "KV_Get",
+        "KV_Set",
+        "KV_Remove"] }
]
```

Add the header to your project
```h
#ifndef PROXYRECOMP_KV_H
#define PROXYRECOMP_KV_H

#include "modding.h"
#include "global.h"

RECOMP_IMPORT(".", int KV_Get(const char* key, void* dest, u32 size));
RECOMP_IMPORT(".", int KV_Set(const char* key, void* data, u32 size));
RECOMP_IMPORT(".", int KV_Remove(const char* key));

#endif // PROXYRECOMP_KV_H
```

Start Getting/Setting values

```cpp
u32 myValue = 4941;
KV_Set("MyMod.MyValue", &myValue, sizeof(u32));

u32 otherValue;
KV_Get("MyMod.MyValue", &otherValue, sizeof(u32));

assert(myValue == otherValue);
```

If you care to check if the get/set succeeded, they return 0/1 for failure/success respectively.
```cpp
u32 myValue = 3041;
if (KV_Get("MyMod.MyValue", &myValue, sizeof(u32))) {
    printf("Loaded myValue from KV\n");
}
```
