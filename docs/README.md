# XSMP-SDK: lightweight and efficient SMP Simulator Development Kit

  [![Build Status](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/build.yml/badge.svg)](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/build.yml)
  [![doc](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/pages/pages-build-deployment)
  [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ThalesGroup_xsmp-sdk&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=ThalesGroup_xsmp-sdk)
  [![Coverity Scan Build Status](https://scan.coverity.com/projects/29012/badge.svg)](https://scan.coverity.com/projects/ThalesGroup-xsmp-sdk)
  [![Open questions](https://img.shields.io/static/v1?label=Discussions&message=Ask&color=blue&logo=github)](https://github.com/ThalesGroup/xsmp-sdk/discussions)
  [![Open bugs](https://img.shields.io/badge/Open-bugs-red.svg)](https://github.com/ThalesGroup/xsmp-sdk/labels/bug)

  ![XSMP logo](https://github.com/ThalesGroup/xsmp-modeler-core/blob/main/logo/xsmp.svg)


## Dependencies

- Linux or MacOS
- A C++ 17 compiler: Clang 5+ and GCC 7+ are officially supported
- CMake 3.14+
- Python 3.7+

### How to Build

Both CMake and Python (pip) builds are supported.

#### Build with CMake

```cmake
include(FetchContent)
FetchContent_Declare(
  xsmp-sdk
  GIT_REPOSITORY https://github.com/ThalesGroup/xsmp-sdk.git
  GIT_TAG        main # replace main with a specific tag
)

FetchContent_MakeAvailable(xsmp-sdk)
```
