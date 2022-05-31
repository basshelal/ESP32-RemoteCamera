# Code Style

Functions in `.h` files should always have the keyword `extern` before their declaration.

Function definitions in `.c` files should always contain their access modifier,
`private` as a replacement to `C`'s `static` keyword (meaning file-private) and
`public` similar to Java's `public` signalling a definition of an `extern` declared function.

Each file contains one "module", "service" or sometimes "object". TODO explain further

Public functions (and optionally private ones too) use the naming convention as such:

```c
public const char *fileName_functionName(int param1, const int *param2) {...}
```

The filename must be the first thing followed by an `_` then the function name in camel case.
This is in an attempt to emulate Java style syntax:
```java
objectName.functionName(param1, param2); // member functions/methods
ClassName.functionName(param1, param2); // static functions/methods
```

```c
fileName_functionName(param1, param2); // global function
```
