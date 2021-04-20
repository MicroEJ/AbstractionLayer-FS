# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.1] - 2021-04-14

### Fixed
- M0074FS-186 Fix compiler errors

## [1.2.0] - 2021-02-15

### Changed
- M0074FS-177 fs-generic cco: add support for custom worker

### Fixed
- M0074FS-174 Add missing inclusion of microej_async_worker.h and LLFS_impl.h in fs_helper.h

### Added
- M0074FS-176 Add an optional llfs_init() function macro to define the initialization function of the file system stack.

## [1.1.0] - 2020-05-27

### Added
- M0074FS-147 Add native flush implementation
- M0074FS-149 Fix compilation warning/error: no value returned for non-void function.

### Fixed
- M0074FS-148: Fix CCO-FS-GENERIC after sni.h fix 

## [1.0.1] - 2020-01-30

### Fixed
- M0074FS-146: Read may throw an exception on EOF

## [1.0.0] - 2019-10-15

### Added
- Initial revision, extracted from CCO-FS-Posix.

---
_Copyright 2019-2021 MicroEJ Corp. All rights reserved._  
_ This library is provided in source code for use, modification and test, subject to license terms._  
_Any modification of the source code will break MicroEJ Corp. warranties on the whole library._  
