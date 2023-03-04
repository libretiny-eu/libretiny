/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _FDB_CFG_H_
#define _FDB_CFG_H_

/* using KVDB feature */
#define FDB_USING_KVDB

#ifdef FDB_USING_KVDB
/* Auto update KV to latest default when current KVDB version number is changed. @see fdb_kvdb.ver_num */
// #define FDB_KV_AUTO_UPDATE
#endif

/* using TSDB (Time series database) feature */
// #define FDB_USING_TSDB

/* Using FAL storage mode */
#define FDB_USING_FAL_MODE

#ifdef FDB_USING_FAL_MODE
/* the flash write granularity, unit: bit
 * only support 1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1) */
#define FDB_WRITE_GRAN 8
#endif

/* Using file storage mode by LIBC file API, like fopen/fread/fwrte/fclose */
// #define FDB_USING_FILE_LIBC_MODE

/* Using file storage mode by POSIX file API, like open/read/write/close */
// #define FDB_USING_FILE_POSIX_MODE

/* MCU Endian Configuration, default is Little Endian Order. */
// #define FDB_BIG_ENDIAN

#if LT_DEBUG_FDB
#include <libretuya.h>
#include <printf/printf.h>
#define FDB_PRINT(...) __wrap_printf(__VA_ARGS__)
#define FDB_DEBUG_ENABLE
#else
#define FDB_PRINT(...)
#endif

#endif /* _FDB_CFG_H_ */
