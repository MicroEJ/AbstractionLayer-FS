# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.2] - 2025-04-09

### Added

- Add a mechanism to allow the C module configuration without the need to modify `fs_configuration.h`. This centralizes the user configuration in the `veeport_configuration.h` file.

### Changed

- Apply Uncrustify code formatting.  

## [2.1.1] - 2023-04-26

### Fixed

  - Make the CCO MISRA C:2012 compliant.
  - CCO build type skeleton.

## [2.1.0] - 2022-06-16

### Changed

  - Change license terms to MicroEJ Corp. BSD-style license.

## [2.0.1] - 2022-04-13

### Fixed

  - Fix compiler errors with strict compiler flags

## [2.0.0] - 2021-04-27

### Added

  - Implement seek, get file pointer, set length and length with fd actions.
  - Remove skip action.
 
## [1.2.0] - 2021-02-15

### Changed
 
  - fs-generic cco: add support for custom worker

### Fixed

  - Add missing inclusion of microej_async_worker.h and LLFS_impl.h in fs_helper.h

### Added

  - Add an optional llfs_init() function macro to define the initialization function of the file system stack.

## [1.1.0] - 2020-05-27

### Added

  - Add native flush implementation
  - Fix compilation warning/error: no value returned for non-void function.

### Fixed

  - M0074FS-148: Fix CCO-FS-GENERIC after sni.h fix 

## [1.0.1] - 2020-01-30

### Fixed

  - M0074FS-146: Read may throw an exception on EOF

## [1.0.0] - 2019-10-15

### Added

  - Initial revision, extracted from CCO-FS-Posix.

---

_Copyright 2019-2023 MicroEJ Corp. All rights reserved._  
_Use of this source code is governed by a BSD-style license that can be found with this software._  
