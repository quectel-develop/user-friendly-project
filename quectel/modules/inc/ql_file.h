#include "QuectelConfig.h"

#ifdef __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-12     armink       first version
 */
#ifndef __QL_FILE_H__
#define __QL_FILE_H__
#include "at.h"
#include "ff.h"
#include "qosa_system.h"
typedef struct ql_file_info
{
    char filename[256];
    u32_t filesize;
    struct ql_file_info *next;   // list next
} ql_file_info_s;

typedef enum
{
    QL_FILE_MODE_CREATE_OR_OPEN = 0, // If file doesn't exist, create a new one; if exists, open it directly. support read and write
    QL_FILE_MODE_CREATE_OR_TRUNCATE, // If file doesn't exist, create a new one; if exists, truncate it. support read and write
    QL_FILE_MODE_OPEN_READ_ONLY, // If file exists, open it in read-only mode; if doesn't exist, return error.

    QL_FILE_TYPE_MAX
} QL_FILE_MODE_E;

typedef enum
{
    QL_SEEK_SET = 0,
    QL_SEEK_CUR, 
    QL_SEEK_END,

    QL_SEEK_MAX
} QL_SEEK_MODE_E;

typedef struct ql_flie
{
    at_client_t client;
    int fd;
    void *ptr;
    size_t len;
    osa_sem_t sem;
    int err;
    ql_file_info_s *file_list;
    #ifdef __QUECTEL_UFP_FEATURE_SUPPORT_TFCARD__
    FIL file;
    #endif
} ql_flie_s;

typedef ql_flie_s* QL_FILE;

/**
 * @brief  open file
 * @param  file_name: file name, max length 80, must be filenames only("/" not allowed)
 * @param  mode: file open mode
 * @return QL_FILE: file stream
 */
QL_FILE ql_fopen(const char *file_name, QL_FILE_MODE_E mode);

/**
 * @brief  close file
 * @param  stream: file stream， returned by ql_fopen()
 * @return 0: success, -1: fail
 */
int ql_fclose(QL_FILE stream);

/*
 * @brief  read file
 * @param  ptr: pointer to the buffer to save read data
 * @param  size: size of each element
 * @param  nmemb: number of elements
 * @param  stream: file stream， returned by ql_fopen()
 * @return number of elements read
*/
int ql_fread(void *ptr, size_t size, size_t nmemb, QL_FILE stream);

/*
 * @brief  write file
 * @param  ptr: pointer to the buffer to be written
 * @param  size: size of each element
 * @param  nmemb: number of elements
 * @param  stream: file stream， returned by ql_fopen()
 * @return number of elements written
*/
int ql_fwrite(const void *ptr, size_t size, size_t nmemb, QL_FILE stream);

/**
 * @brief  Reposition the file position indicator of a stream.
 * @param  stream  Pointer to the FILE object (returned by ql_fopen()).
 * @param  offset  Number of bytes to offset from the reference position.
 *                 Can be positive (forward), negative (backward), or zero.
 * @param  whence  Reference position for the offset. Valid values:
 *                 - SEEK_SET: Beginning of file.
 *                 - SEEK_CUR: Current position.
 *                 - SEEK_END: End of file.
 * @retval 0       Success.
 * @retval -1      Failure.
 */
int ql_fseek(QL_FILE stream, long offset, QL_SEEK_MODE_E whence);

/**
 * @brief  Get the current position of the file pointer.
 * @param  stream  Pointer to the FILE object (returned by ql_fopen()).
 * @retval Position of the file pointer in bytes from the beginning of the file.
 */
int ql_ftell(QL_FILE stream);

/**
 * @brief  Flush the output buffer of a stream.
 * @param  stream  Pointer to the FILE object (returned by ql_fopen()).
 * @retval 0       Success.
 * @retval -1      Failure.
 */
int ql_fflush(QL_FILE stream);

/**
 * @brief  Delete a file.
 * @param  client  Pointer to the AT client object.
 * @param  filename  Name of the file to delete.
 * @retval 0       Success.
 * @retval -1      Failure.
 */
int ql_remove(at_client_t client, const char *filename);

/**
 * @brief  Get the free space and total space of the storage.
 * @param  client  Pointer to the AT client object.
 * @param  filename  Storage path pattern
 * @param  free  Pointer to the variable to store the free space in bytes.
 * @param  total  Pointer to the variable to store the total space in bytes.
 * @retval 0       Success.
 * @retval -1      Failure.
 */
int ql_get_storage_space(at_client_t client,  const char *filename, size_t *free, size_t *total);

/**
 * @brief List files in Quectel storage media
 * @param  client  Pointer to the AT client object.
 * @param filename Storage path pattern in formats:
 *                "*"       : All files in UFS
 * 
 * @param list [OUT] Pointer to array of file info structures.
 *                   Caller must free with ql_free_file_list().
 * @retval 0       Success.
 * @retval -1      Failure.
 */
int ql_file_list(at_client_t client, const char *filename, ql_file_info_s **list);

/**
 * @brief Free file info list.
 * 
 * @param list [IN] Pointer to file info list.
 */
void ql_free_file_list(ql_file_info_s *list);

/**
 * @brief Upload file to Quectel storage media
 * @param  client  Pointer to the AT client object.
 * @param localname Local file path
 * @param remotename Remote file path
 * @retval 0       Success.
 * @retval -1      Failure.
 */
int ql_file_upload(at_client_t client, const char *localname, const char *remotename);

#endif /* __QL_FILE_H__ */
#endif /* __QUECTEL_UFP_FEATURE_SUPPORT_FILESYSTEM__ */

