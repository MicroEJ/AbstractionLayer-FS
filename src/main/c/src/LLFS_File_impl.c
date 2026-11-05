/*
 * C
 *
 * Copyright 2015-2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 * Build: 7E4D1F7C.
 */

/**
 * @file
 * @brief LLFS file-level I/O operations dispatched via async worker.
 * @author MicroEJ Developer Team
 * @version 2.3.0
 */

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stddef.h>
#include "sni.h"
#include "LLFS_impl.h"
#include "LLFS_File_impl.h"
#include "fs_configuration.h"
#include "fs_helper.h"
#include "veeport_errors.h"
#include "LLFS_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Value of the descriptor field of a Java stream once it is closed.
 *
 * A Java method that does not check the closed state can send this value. The file system helper would cast it to
 * a pointer, so every native that receives a file identifier rejects it first. Only this value is rejected: another
 * negative identifier is a valid pointer cast on some targets.
 */
#define LLFS_CLOSED_FILE_ID (-1)

static bool LLFS_is_closed_file_id(int32_t file_id);
static int32_t LLFS_async_exec_write_read_job(int32_t file_id, uint8_t *data, int32_t offset, int32_t length,
                                              bool exec_write, SNI_callback retry_function,
                                              MICROEJ_ASYNC_WORKER_action_t action, SNI_callback on_done);
static int32_t LLFS_async_exec_write_read_byte_job(int32_t file_id, int32_t data, bool exec_write,
                                                   SNI_callback retry_function, MICROEJ_ASYNC_WORKER_action_t action,
                                                   SNI_callback on_done);
static int32_t LLFS_File_IMPL_open_on_done(uint8_t *path, uint8_t mode);
static int32_t LLFS_File_IMPL_write_on_done(int32_t file_id, uint8_t *data, int32_t offset, int32_t length);
static void LLFS_File_IMPL_write_byte_on_done(int32_t file_id, int32_t data);
static int32_t LLFS_File_IMPL_read_on_done(int32_t file_id, uint8_t *data, int32_t offset, int32_t length);
static int32_t LLFS_File_IMPL_read_byte_on_done(int32_t file_id);
static void LLFS_File_IMPL_close_on_done(int32_t file_id);
static void LLFS_File_IMPL_seek_on_done(int32_t file_id, int64_t n);
static int64_t LLFS_File_IMPL_get_file_pointer_on_done(int32_t file_id);
static void LLFS_File_IMPL_set_length_on_done(int32_t file_id, int64_t newLength);
static int64_t LLFS_File_IMPL_get_length_with_fd_on_done(int32_t file_id);
static int32_t LLFS_File_IMPL_available_on_done(int32_t file_id);
static void LLFS_File_IMPL_flush_on_done(int32_t file_id);

int32_t LLFS_File_IMPL_open(uint8_t *path, uint8_t mode) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker, (SNI_callback)LLFS_File_IMPL_open);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return LLFS_NOK;
	}

	FS_open_t *params = (FS_open_t *)job->params;
	if (LLFS_set_path_param(path, (uint8_t *)&params->path) == LLFS_OK) {
		params->mode = mode;

		MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job,
		                                                                       LLFS_File_IMPL_open_action,
		                                                                       (SNI_callback)LLFS_File_IMPL_open_on_done);
		if (status == MICROEJ_ASYNC_WORKER_OK) {
			// Wait for the action to be done
			return SNI_IGNORED_RETURNED_VALUE;//returned value not used
		} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception
	}

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return LLFS_NOK;
}

int32_t LLFS_File_IMPL_write(int32_t file_id, uint8_t *data, int32_t offset, int32_t length) {
	return LLFS_async_exec_write_read_job(file_id, data, offset, length, true, (SNI_callback)LLFS_File_IMPL_write,
	                                      LLFS_File_IMPL_write_action, (SNI_callback)LLFS_File_IMPL_write_on_done);
}

void LLFS_File_IMPL_write_byte(int32_t file_id, int32_t data) {
	FS_UNUSED_RETURN(LLFS_async_exec_write_read_byte_job(file_id, data, true, (SNI_callback)LLFS_File_IMPL_write_byte,
	                                                     LLFS_File_IMPL_write_action,
	                                                     (SNI_callback)LLFS_File_IMPL_write_byte_on_done));
}

int32_t LLFS_File_IMPL_read(int32_t file_id, uint8_t *data, int32_t offset, int32_t length) {
	return LLFS_async_exec_write_read_job(file_id, data, offset, length, false, (SNI_callback)LLFS_File_IMPL_read,
	                                      LLFS_File_IMPL_read_action, (SNI_callback)LLFS_File_IMPL_read_on_done);
}

int32_t LLFS_File_IMPL_read_byte(int32_t file_id) {
	return LLFS_async_exec_write_read_byte_job(file_id, 0, false, (SNI_callback)LLFS_File_IMPL_read_byte,
	                                           LLFS_File_IMPL_read_action,
	                                           (SNI_callback)LLFS_File_IMPL_read_byte_on_done);
}

void LLFS_File_IMPL_close(int32_t file_id) {
	if (LLFS_is_closed_file_id(file_id)) {
		return;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker, (SNI_callback)LLFS_File_IMPL_close);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return;
	}

	FS_close_t *params = (FS_close_t *)job->params;
	params->file_id = file_id;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job, LLFS_File_IMPL_close_action,
	                                                                       (SNI_callback)LLFS_File_IMPL_close_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

void LLFS_File_IMPL_seek(int32_t file_id, int64_t n) {
	if (LLFS_is_closed_file_id(file_id)) {
		return;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker, (SNI_callback)LLFS_File_IMPL_seek);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return;
	}

	FS_seek_t *params = (FS_seek_t *)job->params;
	params->file_id = file_id;
	params->n = n;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job, LLFS_File_IMPL_seek_action,
	                                                                       (SNI_callback)LLFS_File_IMPL_seek_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return;
}

int64_t LLFS_File_IMPL_get_file_pointer(int32_t file_id) {
	if (LLFS_is_closed_file_id(file_id)) {
		return LLFS_NOK;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker,
	                                                                    (SNI_callback)LLFS_File_IMPL_get_file_pointer);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return SNI_IGNORED_RETURNED_VALUE;
		;
	}

	FS_getfp_t *params = (FS_getfp_t *)job->params;
	params->file_id = file_id;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job,
	                                                                       LLFS_File_IMPL_get_file_pointer_action,
	                                                                       (SNI_callback)
	                                                                       LLFS_File_IMPL_get_file_pointer_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return SNI_IGNORED_RETURNED_VALUE;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return LLFS_NOK;
}

void LLFS_File_IMPL_set_length(int32_t file_id, int64_t newLength) {
	if (LLFS_is_closed_file_id(file_id)) {
		return;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker,
	                                                                    (SNI_callback)LLFS_File_IMPL_set_length);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return;
	}

	FS_set_length_t *params = (FS_set_length_t *)job->params;
	params->file_id = file_id;
	params->length = newLength;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job,
	                                                                       LLFS_File_IMPL_set_length_action,
	                                                                       (SNI_callback)
	                                                                       LLFS_File_IMPL_set_length_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return;
}

int64_t LLFS_File_IMPL_get_length_with_fd(int32_t file_id) {
	if (LLFS_is_closed_file_id(file_id)) {
		return LLFS_NOK;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker,
	                                                                    (SNI_callback)LLFS_File_IMPL_get_length_with_fd);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return SNI_IGNORED_RETURNED_VALUE;
	}

	FS_get_length_with_fd_t *params = (FS_get_length_with_fd_t *)job->params;
	params->file_id = file_id;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job,
	                                                                       LLFS_File_IMPL_get_length_with_fd_action,
	                                                                       (SNI_callback)
	                                                                       LLFS_File_IMPL_get_length_with_fd_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return SNI_IGNORED_RETURNED_VALUE;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return LLFS_NOK;
}

int32_t LLFS_File_IMPL_available(int32_t file_id) {
	if (LLFS_is_closed_file_id(file_id)) {
		return LLFS_NOK;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker,
	                                                                    (SNI_callback)LLFS_File_IMPL_available);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return LLFS_NOK;
	}

	FS_available_t *params = (FS_available_t *)job->params;
	params->file_id = file_id;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job,
	                                                                       LLFS_File_IMPL_available_action,
	                                                                       (SNI_callback)
	                                                                       LLFS_File_IMPL_available_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return SNI_IGNORED_RETURNED_VALUE;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return LLFS_NOK;
}

void LLFS_File_IMPL_flush(int32_t file_id) {
	if (LLFS_is_closed_file_id(file_id)) {
		return;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker, (SNI_callback)LLFS_File_IMPL_flush);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return;
	}

	FS_flush_t *params = (FS_flush_t *)job->params;
	params->file_id = file_id;

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job, LLFS_File_IMPL_flush_action,
	                                                                       (SNI_callback)LLFS_File_IMPL_flush_on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return;
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

/**
 * @brief Prepare and send an execution job to async_worker, called either from
 * <code>LLFS_File_IMPL_write</code> or <code>LLFS_File_IMPL_read</code>.
 *
 * @param[in] file_id file identifier.
 * @param[in] data buffer used for reading or writing operations.
 * @param[in] offset the offset inside the buffer where the data has to be manipulated.
 * @param[in] length buffer length.
 * @param[in] exec_write true if the data buffer content need to be sent to the async_worker job, else false.
 * @param[in] retry_function if the current Java thread has been suspended, this function is called when it is resumed.
 * @param[in] action the function to execute asynchronously.
 * @param[in] on_done the <code>SNI_callback</code> called when the job is done.
 *
 * @return <code>SNI_IGNORED_RETURNED_VALUE</code> on success, else a negative error code.
 */
static int32_t LLFS_async_exec_write_read_job(int32_t file_id, uint8_t *data, int32_t offset, int32_t length,
                                              bool exec_write, SNI_callback retry_function,
                                              MICROEJ_ASYNC_WORKER_action_t action, SNI_callback on_done) {
	if (LLFS_is_closed_file_id(file_id)) {
		return LLFS_NOK;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker, retry_function);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return LLFS_NOK;
	}

	FS_write_read_t *params = (FS_write_read_t *)job->params;

	bool do_copy = exec_write;
	int32_t result = SNI_retrieveArrayElements((int8_t *)data, offset, length, (int8_t *)&params->buffer,
	                                           sizeof(params->buffer), (int8_t **)&params->data,
	                                           (uint32_t *)&params->length, do_copy);

	if (result != SNI_OK) {
		VEEPORT_ERRORS_throw_runtime_exception(RUNTIME_EXCEPTION_SNI_INTERNAL_ERROR);
	} else {
		params->file_id = file_id;

		MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job, action, on_done);
		if (status == MICROEJ_ASYNC_WORKER_OK) {
			// Wait for the action to be done
			return SNI_IGNORED_RETURNED_VALUE;//returned value not used
		} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception
	}

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return LLFS_NOK;
}

/**
 * @brief Prepare and send an execution job to async_worker, called either from
 * <code>LLFS_File_IMPL_write_byte</code> or <code>LLFS_File_IMPL_read_byte</code>.
 *
 * @param[in] file_id file identifier.
 * @param[in/out] data byte to be read/written.
 * @param[in] exec_write true if the data buffer content need to be sent to the async_worker job, else false.
 * @param[in] retry_function if the current Java thread has been suspended, this function is called when it is resumed.
 * @param[in] action the function to execute asynchronously.
 * @param[in] on_done the <code>SNI_callback</code> called when the job is done.
 *
 * @return <code>SNI_IGNORED_RETURNED_VALUE</code> on success, else a negative error code.
 */
static int32_t LLFS_async_exec_write_read_byte_job(int32_t file_id, int32_t data, bool exec_write,
                                                   SNI_callback retry_function, MICROEJ_ASYNC_WORKER_action_t action,
                                                   SNI_callback on_done) {
	if (LLFS_is_closed_file_id(file_id)) {
		return LLFS_NOK;
	}
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_allocate_job(&fs_worker, retry_function);
	if (job == NULL) {
		// No job available, either:
		// - wait for a job to be available and this function to be executed again,
		// - or an exception is pending
		return LLFS_NOK;
	}

	FS_write_read_t *params = (FS_write_read_t *)job->params;

	params->file_id = file_id;
	params->data = (uint8_t *)&params->buffer;
	params->length = sizeof(uint8_t);
	if (exec_write == true) {
		params->buffer[0] = (uint8_t)data;
	}

	MICROEJ_ASYNC_WORKER_status_t status = MICROEJ_ASYNC_WORKER_async_exec(&fs_worker, job, action, on_done);
	if (status == MICROEJ_ASYNC_WORKER_OK) {
		// Wait for the action to be done
		return SNI_IGNORED_RETURNED_VALUE;//returned value not used
	} // else an error occurred and MICROEJ_ASYNC_WORKER_async_exec has thrown a SNI exception

	// Error
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return LLFS_NOK;
}

/**
 * @brief Rejects the identifier of a closed Java stream before a job is allocated.
 *
 * When the identifier is <code>LLFS_CLOSED_FILE_ID</code>, a NativeIOException with the code
 * <code>LLFS_CAUGHT_EXCEPTION_CLOSED_FILE</code> is pending when this function returns, and the caller must return
 * without touching the file system helper.
 *
 * @param[in] file_id file identifier.
 *
 * @return true when the identifier is the one of a closed stream, false otherwise.
 */
static bool LLFS_is_closed_file_id(int32_t file_id) {
	bool is_closed = false;
	if (LLFS_CLOSED_FILE_ID == file_id) {
		VEEPORT_ERRORS_throw_caught_exception(LLFS_CAUGHT_EXCEPTION_CLOSED_FILE);
		is_closed = true;
	}
	return is_closed;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by <code>LLFS_File_IMPL_open</code>
 * is done.
 *
 * @param[in] path absolute path of file to open.
 * @param[in] mode opening mode.
 *
 * @return <code>LLFS_File_IMPL_open_action</code> function return code.
 */
static int32_t LLFS_File_IMPL_open_on_done(uint8_t *path, uint8_t mode) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_open_t *params = (const FS_open_t *)job->params;

	FS_UNUSED_PARAM(path);
	FS_UNUSED_PARAM(mode);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));

	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by <code>LLFS_File_IMPL_write</code>
 * is done.
 *
 * @param[in] file_id file identifier.
 * @param[in] data buffer used for writing operations.
 * @param[in] offset the offset inside the buffer where the data has to be manipulated.
 * @param[in] length buffer length.
 *
 * @return <code>LLFS_File_IMPL_write_action</code> function return code.
 */
static int32_t LLFS_File_IMPL_write_on_done(int32_t file_id, uint8_t *data, int32_t offset, int32_t length) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_write_read_t *params = (const FS_write_read_t *)job->params;

	FS_UNUSED_PARAM(file_id);
	FS_UNUSED_PARAM(data);
	FS_UNUSED_PARAM(offset);
	FS_UNUSED_PARAM(length);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));

	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by <code>LLFS_File_IMPL_read</code>
 * is done.
 *
 * @param[in] file_id file identifier.
 * @param[out] data buffer used for reading operations.
 * @param[in] offset the offset inside the buffer where the data has to be manipulated.
 * @param[in] length buffer length.
 *
 * @return <code>LLFS_File_IMPL_read_action</code> function return code.
 */
static int32_t LLFS_File_IMPL_read_on_done(int32_t file_id, uint8_t *data, int32_t offset, int32_t length) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_write_read_t *params = (const FS_write_read_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	} else if (result != LLFS_EOF) {
		// cppcheck-suppress misra-c2012-11.8 // SNI API does not use const-qualified parameters.
		int32_t release_result = SNI_flushArrayElements((int8_t *)data, offset, length, (int8_t *)params->data, result);
		if (release_result != SNI_OK) {
			VEEPORT_ERRORS_throw_runtime_exception(RUNTIME_EXCEPTION_SNI_INTERNAL_ERROR);
		}
	} else {
		// Successful: result hold the number of read bytes.
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));

	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by
 *<code>LLFS_File_IMPL_write_byte</code> is done.
 *
 * @param[in] file_id file identifier.
 * @param[in] data byte to be written.
 *
 * @return <code>LLFS_File_IMPL_write_action</code> function return code.
 */
static void LLFS_File_IMPL_write_byte_on_done(int32_t file_id, int32_t data) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_write_read_t *params = (const FS_write_read_t *)job->params;

	FS_UNUSED_PARAM(file_id);
	FS_UNUSED_PARAM(data);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by
 *<code>LLFS_File_IMPL_read_byte</code> is done.
 *
 * @param[in] file_id file identifier.
 *
 * @return <code>LLFS_File_IMPL_read_action</code> function return code.
 */
static int32_t LLFS_File_IMPL_read_byte_on_done(int32_t file_id) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_write_read_t *params = (const FS_write_read_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	} else if (result == 1) {
		// 1 byte read: return the read byte
		result = (uint8_t)params->data[0];
	} else if (result != LLFS_EOF) {
		// Invalid value returned by the read_byte action
		VEEPORT_ERRORS_throw_runtime_exception(RUNTIME_EXCEPTION_UNDEFINED);
	} else {
		// Successful: result hold the number of read bytes.
	}
	// else: result==LLFS_EOF: just return LLFS_EOF

	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));

	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by <code>LLFS_File_IMPL_close</code>
 * is done.
 *
 * @param[in] file_id file identifier.
 *
 * @return <code>LLFS_File_IMPL_close_action</code> function return code.
 */
static void LLFS_File_IMPL_close_on_done(int32_t file_id) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_close_t *params = (const FS_close_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}

	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by <code>LLFS_File_IMPL_seek</code>
 * is done.
 *
 * @param[in] file_id file identifier.
 * @param[in] n the new position of the file pointer.
 *
 * @return <code>LLFS_File_IMPL_seek_action</code> function return code.
 */
static void LLFS_File_IMPL_seek_on_done(int32_t file_id, int64_t n) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_seek_t *params = (const FS_seek_t *)job->params;

	FS_UNUSED_PARAM(file_id);
	FS_UNUSED_PARAM(n);

	if (params->result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by
 *<code>LLFS_File_IMPL_get_file_pointer</code> is done.
 *
 * @param[in] file_id file identifier.
 *
 * @return <code>LLFS_File_IMPL_get_file_pointer_on_done</code> function return code.
 */
static int64_t LLFS_File_IMPL_get_file_pointer_on_done(int32_t file_id) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_getfp_t *params = (const FS_getfp_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int64_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by
 *<code>LLFS_File_IMPL_set_length</code> is done.
 *
 * @param[in] file_id file identifier.
 * @param[in] newLength the new length of the file.
 *
 * @return <code>LLFS_File_IMPL_set_length_action</code> function return code.
 */
static void LLFS_File_IMPL_set_length_on_done(int32_t file_id, int64_t newLength) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_set_length_t *params = (const FS_set_length_t *)job->params;

	FS_UNUSED_PARAM(file_id);
	FS_UNUSED_PARAM(newLength);

	if (params->result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by
 *<code>LLFS_File_IMPL_get_length_with_fd</code> is done.
 *
 * @param[in] file_id file identifier.
 *
 * @return <code>LLFS_IMPL_get_length_with_fd_action</code> function return code.
 */
static int64_t LLFS_File_IMPL_get_length_with_fd_on_done(int32_t file_id) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_get_length_with_fd_t *params = (const FS_get_length_with_fd_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int64_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}
	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by
 *<code>LLFS_File_IMPL_available</code> is done.
 *
 * @param[in] file_id file identifier.
 *
 * @return <code>LLFS_File_IMPL_available_action</code> function return code.
 */
static int32_t LLFS_File_IMPL_available_on_done(int32_t file_id) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_available_t *params = (const FS_available_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}

	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));

	return result;
}

/**
 * @brief The <code>SNI_callback</code> called when the async_worker job requested by <code>LLFS_File_IMPL_flush</code>
 * is done.
 *
 * @param[in] file_id file identifier.
 *
 * @return <code>LLFS_File_IMPL_flush_action</code> function return code.
 */
static void LLFS_File_IMPL_flush_on_done(int32_t file_id) {
	MICROEJ_ASYNC_WORKER_job_t *job = MICROEJ_ASYNC_WORKER_get_job_done();
	const FS_flush_t *params = (const FS_flush_t *)job->params;

	FS_UNUSED_PARAM(file_id);

	int32_t result = params->result;
	if (result == LLFS_NOK) {
		// Exception
		VEEPORT_ERRORS_throw_caught_exception(params->error_code);
	}

	FS_UNUSED_RETURN(MICROEJ_ASYNC_WORKER_free_job(&fs_worker, job));
}

#ifdef __cplusplus
}
#endif
