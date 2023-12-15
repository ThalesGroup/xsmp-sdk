# XSMP-SDK: lightweight and efficient SMP Simulator Development Kit

[![Build Status](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/build.yml/badge.svg)](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/build.yml)
[![doc](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/ThalesGroup/xsmp-sdk/actions/workflows/pages/pages-build-deployment)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ThalesGroup_xsmp-sdk&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=ThalesGroup_xsmp-sdk)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/29012/badge.svg)](https://scan.coverity.com/projects/ThalesGroup-xsmp-sdk)
[![Open questions](https://img.shields.io/static/v1?label=Discussions&message=Ask&color=blue&logo=github)](https://github.com/ThalesGroup/xsmp-sdk/discussions)
[![Open bugs](https://img.shields.io/badge/Open-bugs-red.svg)](https://github.com/ThalesGroup/xsmp-sdk/labels/bug)

<p align="center">
    <picture>
      <source media="(prefers-color-scheme: dark)" width="200" srcset="https://github.com/ThalesGroup/xsmp-modeler-core/raw/main/docs/images/xsmp_logo_dark.svg">
      <source media="(prefers-color-scheme: light)" width="200" srcset="https://github.com/ThalesGroup/xsmp-modeler-core/raw/main/docs/images/xsmp_logo_light.svg">
      <img alt="XSMP logo" width="200" src="https://github.com/ThalesGroup/xsmp-modeler-core/raw/main/docs/images/xsmp_logo_light.svg">
    </picture>
</p>

XSMP Simulator Development Kit is a framework for development of SMDL (Simulation Model Definition Language) as defined in the [ECSS SMP standard](https://ecss.nl/standard/ecss-e-st-40-07c-simulation-modelling-platform-2-march-2020/).

It provides all SMP components to develop and test catalogues:

- a Component Development Kit (CDK)
- a lightweight simulator and standard services for test purpose
- a Python binding to ease the writing of unit-tests

## System Requirements

- Linux, Windows or MacOS
- A C++ 17 compiler: Clang 5+, GCC 7+ and MSVC 2019+ are officially supported
- CMake 3.14+ (3.20+ for Python tests)
- Python 3.7+ and pytest

## How to Build

Both CMake and Python (pip) builds are supported.

### Build with CMake

```bash
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release
```

### Build with Python (pip)

```bash
python -m pip install .[test] -v
```

## Documentation

Documentation is available [here](https://ThalesGroup.github.io/xsmp-sdk/).

## Contributing

If you are interested in contributing, start by reading the [Contributing guide](/CONTRIBUTING.md).

## License

- [Apache License Version 2.0](https://www.apache.org/licenses/LICENSE-2.0)
