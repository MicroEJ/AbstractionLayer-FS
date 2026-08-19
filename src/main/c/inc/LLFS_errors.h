/*
 * C
 *
 * Copyright 2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 * Build: 7E4D1F7C.
 */

#ifndef LLFS_ERRORS_H
#define LLFS_ERRORS_H

/**
 * @file
 * @brief LLFS caught exception codes and error definitions.
 * @author MicroEJ Developer Team
 * @version 1.0.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Caught exception codes for LLFS (use with VEEPORT_ERRORS_throw_caught_exception).
 * Each code represents a distinct error category that the Java side can handle. */

/** @brief An undefined error occurred */
#define LLFS_CAUGHT_EXCEPTION_UNDEFINED          (-1)
/** @brief File open failed */
#define LLFS_CAUGHT_EXCEPTION_OPEN               (-2)
/** @brief File close failed */
#define LLFS_CAUGHT_EXCEPTION_CLOSE              (-3)
/** @brief File read failed */
#define LLFS_CAUGHT_EXCEPTION_READ               (-4)
/** @brief File write failed */
#define LLFS_CAUGHT_EXCEPTION_WRITE              (-5)
/** @brief File seek or tell failed */
#define LLFS_CAUGHT_EXCEPTION_SEEK               (-6)
/** @brief File stat failed */
#define LLFS_CAUGHT_EXCEPTION_STAT               (-7)
/** @brief File truncate failed */
#define LLFS_CAUGHT_EXCEPTION_TRUNCATE           (-8)
/** @brief File sync/flush failed */
#define LLFS_CAUGHT_EXCEPTION_SYNC               (-9)
/** @brief No space left on device */
#define LLFS_CAUGHT_EXCEPTION_DISK_FULL          (-10)
/** @brief Too many open files or directories */
#define LLFS_CAUGHT_EXCEPTION_RESOURCE_EXHAUSTED (-11)
/** @brief Unsupported opening mode */
#define LLFS_CAUGHT_EXCEPTION_UNSUPPORTED_MODE   (-12)
/** @brief Invalid parameter */
#define LLFS_CAUGHT_EXCEPTION_INVALID_PARAMETER  (-13)
/** @brief Permission denied */
#define LLFS_CAUGHT_EXCEPTION_PERMISSION         (-14)
/** @brief File size query failed */
#define LLFS_CAUGHT_EXCEPTION_FILE_SIZE          (-15)

#ifdef __cplusplus
}
#endif

#endif /* LLFS_ERRORS_H */
