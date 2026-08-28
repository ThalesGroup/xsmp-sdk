# XSMP-SDK: Lightweight and Efficient SMP Simulator Development Kit

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

**XSMP SDK** is a framework for the development of SMDL (Simulation Model Definition Language) as defined in the [ECSS SMP standard](https://ecss.nl/standard/ecss-e-st-40-07c-simulation-modelling-platform-2-march-2020/). It provides all SMP components to develop and test catalogues:

- a Component Development Kit (CDK)
- a lightweight simulator and standard services for test purposes
- a Python SMP unit-test framework

## Usage with XSMP Modeler

**XSMP SDK** is designed to be used in conjunction with the [XSMP Modeler](https://github.com/ThalesGroup/xsmp-modeler-core) framework. Together, these two projects offer a comprehensive and open solution for modeling, developing, and managing SMP projects.

- **Seamless Integration**: The XSMP Modeler provides an XSMP SDK profile, facilitating easy integration.
- **Enhanced Productivity**: Using these two projects together accelerates development and reduces errors.
- **Consistency and Standardization**: Ensures adherence to established standards and practices.

## Building and installing

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
# the install rule is enabled by default when xsmp-sdk is the top level project
cmake --install build --prefix /path/to/install
```

### CMake options

| Option | Default | Description |
| --- | --- | --- |
| `XSMP_BUILD_TESTS` | `ON` for the top level project | Build the unit tests. |
| `XSMP_BUILD_PYTHON_BINDINGS` | `ON` | Build the `ecss_smp` Python module and install the Python packages. |
| `XSMP_BUILD_PACKAGE` | `ON` for the top level project | Build the CPack package. |
| `XSMP_BUILD_EXAMPLES` | `OFF` | Build the examples. |
| `XSMP_ENABLE_INSTALL` | `ON` for the top level project | Enable the install rule, required to export the targets and use `find_package(xsmp-sdk)`. |
| `XSMP_ENABLE_CODECOVERAGE` | `OFF` | Enable the code coverage support. |
| `XSMP_BUILD_WITH_WARNINGS` | `OFF` | Enable all compiler warnings. |
| `XSMP_INSTALL_PYTHONDIR` | `<libdir>/python<X.Y>/site-packages` | Directory where the Python modules are installed. |

### Installing the Python modules

The `ecss_smp` module and the Python packages (`xsmp`, `xsmp_logger`, ...) are installed in
`XSMP_INSTALL_PYTHONDIR`, whose default value is deduced from the `site-packages` layout of the
Python interpreter found at configuration time (e.g. `lib/python3.12/site-packages`).

- A **relative** path (the default) is resolved against the install prefix, so
  `cmake --install build --prefix /tmp/test` installs everything under `/tmp/test`. Add
  `/tmp/test/lib/python<X.Y>/site-packages` to your `PYTHONPATH` to use them.
- An **absolute** path installs the modules outside of the install prefix, e.g. to install them
  directly in the `site-packages` directory of the current interpreter:
  ```bash
  cmake -S . -B build -DXSMP_INSTALL_PYTHONDIR="$(python3 -c 'import sysconfig; print(sysconfig.get_path("purelib"))')"
  ```

Building with `-DXSMP_BUILD_PYTHON_BINDINGS=OFF` installs no Python module at all; only the C++
libraries, the headers and the CMake configuration files are installed.

## Documentation

Comprehensive documentation is available [here](https://ThalesGroup.github.io/xsmp-sdk/). We encourage you to read it to get the most out of the XSMP SDK.

## Contributing

If you are interested in contributing, start by reading the [Contributing guide](/CONTRIBUTING.md). We welcome contributions of all kinds and appreciate your help in improving XSMP SDK.

## License

This project is licensed under the [Apache License Version 2.0](https://www.apache.org/licenses/LICENSE-2.0).
