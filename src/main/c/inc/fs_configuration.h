/*
 * C
 *
 * Copyright 2014-2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 * Build: 7E4D1F7C.
 */

#ifndef  FS_CONFIGURATION_H
#define  FS_CONFIGURATION_H

/**
 * @file
 * @brief LLFS configuration macros and user-overridable settings.
 * @author MicroEJ Developer Team
 * @version 2.3.0
 */

// include MicroEJ User configuration file
#include <sni.h>
#if !defined(__CC_ARM)
// cppcheck-suppress [misra-c2012-20.9]: The macro '__has_include' is defined by the toolchain.
   #if __has_include("veeport_configuration.h")
	  #include "veeport_configuration.h"
   #endif // __has_include("veeport_configuration.h")
#else
   #warning "This C module needs a 'veeport_configuration.h' in your ARMCC project. "
   #include "veeport_configuration.h"
#endif // !defined ( __CC_ARM)

#include "microej_async_worker.h"

/* <stdio.h> is pulled here, ahead of every declaration in this header, because the
 * LLFS_DEBUG_TRACE default below needs printf and an #include may only be preceded by
 * preprocessor directives and comments (MISRA C:2012 Rule 20.1). */
#ifdef LLFS_DEBUG
  #ifndef LLFS_DEBUG_TRACE
	#include <stdio.h>
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compatibility sanity check value.
 */
#if (defined(FS_MAJOR_CFG_VERSION) && ((FS_MAJOR_CFG_VERSION != 2) || (defined(FS_MINOR_CFG_VERSION) && \
	((FS_MINOR_CFG_VERSION != 2) || (defined(FS_PATCH_CFG_VERSION) &&                                   \
	(FS_PATCH_CFG_VERSION != 0))))))
#error "This FS configuration version is different from the one declared in the veeport_configuration.h file."
#endif

/**
 * @brief Use this macro to define the initialization function of the file system stack.
 * Called from LLFS_IMPL_initialize().
 * By default this macro does nothing.
 */
#ifndef llfs_init
  #define llfs_init   (fs_helper_init)

void fs_helper_init(void);
#endif

/**
 * @brief Set the FS_CUSTOM_WORKER define to use a custom worker to handle FS asynchronous jobs. Define FS custom worker
 * if required.
 */
#ifdef FS_CUSTOM_WORKER

/**
 * @brief Set the FS_CUSTOM_WORKER_NAME define  the name of the custom worker if it used.
 *
 */
#ifndef FS_CUSTOM_WORKER_NAME
// cppcheck-suppress [misra-c2012-2.5]: macro defined for optional function implementations.
  #define FS_CUSTOM_WORKER_NAME   my_custom_worker
#endif

/**
 * @ brief Control structure used by the custom worker and defined from its name.
 *
 */
extern MICROEJ_ASYNC_WORKER_handle_t FS_CUSTOM_WORKER_NAME;
// cppcheck-suppress [misra-c2012-2.5]: macro defined for optional function implementations.
#define fs_worker FS_CUSTOM_WORKER_NAME

#else
/**
 * @brief Control structure used by the default worker implemented in the C module AsyncWorker.
 *
 */
// cppcheck-suppress [misra-c2012-5.5]: Macro name same as the global variable name for simplicity of implementation.
extern MICROEJ_ASYNC_WORKER_handle_t fs_worker;
#endif

/**
 * @brief Number of workers dedicated to the FS in async_worker.
 */
#ifndef FS_WORKER_JOB_COUNT
#define FS_WORKER_JOB_COUNT (1)
#endif

/**
 * @brief Size of the waiting list for FS jobs in async_worker.
 */
#ifndef FS_WAITING_LIST_SIZE
#define FS_WAITING_LIST_SIZE (1)
#endif

/**
 * @brief Size of the FS stack in bytes.
 */
#ifndef FS_WORKER_STACK_SIZE
  #error The FS_WORKER_STACK_SIZE compatible must be defined.
#endif

/**
 * @brief FS worker stack size must be calibrated, unless using a custom worker defined in another module.
 */
#if !defined(FS_CUSTOM_WORKER) && !defined(FS_WORKER_STACK_SIZE)
#error \
	"FS_WORKER_STACK_SIZE not declared. Please uncomment the line above to enable macro declaration and put the right value according to the stack size calibration done for your environment"
#endif // FS_WORKER_STACK_SIZE

/**
 * @brief Priority of the FS workers.
 */
#ifndef FS_WORKER_PRIORITY
  #error The FS_WORKER_PRIORITY constant must be defined  in the veeport_configuration.h file.
#endif

/**
 * @brief Maximum path length.
 */
#ifndef FS_PATH_LENGTH
#define FS_PATH_LENGTH (256)
#endif

/**
 * @brief Size of the IO buffer in bytes.
 */
#ifndef FS_IO_BUFFER_SIZE
#define FS_IO_BUFFER_SIZE (1024)
#endif

/**
 * @brief macro used to mark unused function parameters for MISRA compliance
 */
#define FS_UNUSED_PARAM(x)                     ((void)(x))

/**
 * @brief macro used to mark function calls for which the return value is ignored (MISRA compliance)
 */
#define FS_UNUSED_RETURN(x)                     (void)(x)

/**
 * @brief Optional path prefix prepended to all file paths before passing them to the underlying
 * filesystem. Define this macro in veeport_configuration.h to a string literal
 * (e.g. @code #define FS_PATH_PREFIX "/SD:" @endcode).
 * When not defined, no prefix is added.
 */
// FS_PATH_PREFIX is optional, define it in veeport_configuration.h if needed.

/**
 * @brief Copies a file path from an input buffer to another buffer that will be sent to
 * the async_worker job, checking against path size constraints.
 *
 * @param[in] path caller buffer where the file path is stored.
 * @param[in] path_param buffer where the file path will be copied if the path size constraints are solved.
 *
 * @return <code>LLFS_OK</code> on success, else a negative error code.
 */
int32_t LLFS_set_path_param(const uint8_t *path, uint8_t *path_param);

/**
 * @brief Set this define to print debug traces.
 */
#ifdef LLFS_DEBUG
  #ifndef LLFS_DEBUG_TRACE
	#define LLFS_DEBUG_TRACE printf("[DEBUG][FS] "); printf
  #endif
#else
#define LLFS_DEBUG_TRACE(...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif // FS_CONFIGURATION_H
