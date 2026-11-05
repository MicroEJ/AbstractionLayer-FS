![FS](https://shields.microej.com/endpoint?url=https://repository.microej.com/packages/badges/fs_6.json)

# Abstraction Layer File System

# Overview

MicroEJ Low Level API implementation for File System operations.

This module implements the `LLFS_impl` Low Level API to provide generic file system operations dispatched asynchronously using the `bsp-microej-async-worker` library. All filesystem operations (path-level and file-level I/O) are executed on a dedicated worker thread, ensuring non-blocking behavior for the MicroEJ application.

The functions called to perform the actual filesystem operations are specific to the target platform and must be implemented by the user. See [Usage section](#usage) for more information.

Here is a list of useful MicroEJ documentation pages:

- [FS Pack](https://docs.microej.com/en/latest/VEEPortingGuide/packFS.html)
- [LLFS: File System](https://docs.microej.com/en/latest/VEEPortingGuide/appendix/llapi.html#llfs-file-system)

This implementation has a default configuration file, [fs_configuration.h](src/main/c/inc/fs_configuration.h).
If you want to update a configuration, please edit or create the file `veeport_configuration.h`. See [Usage section](#usage) for more information.

# Usage

1. These sources can be included in the VEE Port with the method you prefer, by using this repository as a submodule or by doing a copy of the sources in the VEE Port repository.
2. Implement the filesystem action functions listed in [fs_helper.h](src/main/c/inc/fs_helper.h) by following the functions documentation. These functions are executed on the async worker thread.
3. Configure the implementation by editing or creating the file `veeport_configuration.h`.

# Requirements

- MicroEJ Architecture 8.0 or higher
- MicroEJ Pack FS 6.0 or higher
- AbstractionLayer-Essentials 1.0.0 or higher

# Validation

Here is a non-exhaustive list of tested environments:

- VEE Ports:
  - VEEPort-STM32F7508-DK 2.3.3 (FatFS back-end)
  - VEEPort-Zephyr 3.0.0 (Zephyr FS over FatFS back-end)
  - VEEPort-Linux 3.3.1 (LLFS over FS POSIX back-end)
  - A VEE Port for STM32U5G9 with ThreadX (FileX back-end)
- C Modules:
  - AbstractionLayer-Essentials 1.0.0
  - FS FileX 1.1.0

# MISRA Compliance

This Abstraction Layer implementation is MISRA-compliant (MISRA C:2012) with some noted exceptions.
It has been verified with Cppcheck v2.19. Here is the list of deviations from MISRA standard:

| Deviation  | Category | Justification                                                                                         |
|:----------:|:--------:|:----------------------------------------------------------------------------------------------------- |
|  Rule 2.3  | Advisory | A type can be defined at API level and not used by the application                                   |
|  Rule 2.5  | Advisory | A macro can be defined at API level and not used by the application                                  |
|  Rule 5.5  | Required | A macro has the same name as the global variable it gives access to, for simplicity of implementation. |
|  Rule 8.4  | Required | Objects defined by macros expanding from external libraries (async_worker, osal)                     |
|  Rule 8.7  | Advisory | Public API functions require external linkage as they are entry points called from Java via SNI      |
|  Rule 11.3 | Required | Cast used by many C framework to factorize code                                                      |
|  Rule 11.4 | Advisory | A string literal path is converted to the `uint8_t *` path buffer type.                              |
|  Rule 11.5 | Advisory | Used for code genericity/abstraction                                                                 |
|  Rule 11.8 | Required | The SNI and async worker APIs take non-const pointers.                                               |
|  Rule 15.5 | Advisory | Multiple return points are used for early error exits to simplify control flow                       |
|  Rule 18.4 | Advisory | Can be used in configurable C library                                                                |
|  Rule 19.2 | Advisory | Can be useful when designing C library                                                               |
|  Rule 20.1 | Advisory | Extern "C" block wraps includes and code together per coding conventions                             |
|  Rule 20.7 | Required | Macro parameters used in type declarations cannot be parenthesized                                   |
|  Rule 20.9 | Required | The `__has_include` macro is defined by the toolchain.                                               |
|  Rule 20.10| Advisory | Used by MicroEJ architectures                                                                        |
|  Rule 20.12| Advisory | Used by legacy LL APIs C header files automatically generated                                        |
|  Rule 21.6 | Required | Used for printf debugging                                                                            |

# Dependencies

See [Requirements section](#requirements).

# Source

N/A.

# Restrictions

None.


---

_Copyright 2020-2026 MicroEJ Corp. All rights reserved._  
_Use of this source code is governed by a BSD-style license that can be found with this software._  
_Build: 7E4D1F7C._