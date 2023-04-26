# Overview

MicroEJ C Component: `@MMM_MODULE_NAME@`.

# Usage

Add the following line to your `module.ivy`:

    @MMM_DEPENDENCY_DECLARATION@

# Requirements

This library requires the following Foundation Libraries:

    @FOUNDATION_LIBRARIES_LIST@

## MISRA Compliance

This Abstraction Layer implementation is MISRA-compliant (MISRA C:2012) with some noted exception. 
It has been verified with Cppcheck v2.10. Here is the list of deviations from MISRA standard:

| Deviation  | Category | Justification                                                       |
|:----------:|:--------:|:------------------------------------------------------------------- |
|  Rule 2.3  | Advisory | A type can be defined at API level and not used by the application  |
|  Rule 2.5  | Advisory | A macro can be defined at API level and not used by the application |
|  Rule 8.4  | Required | The Cppcheck analysis is made only on this LL code                  |
| Rule 15.5  | Advisory | Dismissed for code readability                                      |
| Rule 19.2  | Advisory | Can be usefull when designing C library                             |
| Rule 21.6  | Required | Used for printf usage                                               |

# Dependencies

_All dependencies are retrieved transitively by MicroEJ Module Manager_.

# Source

N/A.

# Restrictions

None.


---

_Copyright 2020-2023 MicroEJ Corp. All rights reserved._  
_Use of this source code is governed by a BSD-style license that can be found with this software._  
