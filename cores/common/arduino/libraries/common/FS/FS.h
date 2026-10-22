/*
 FS.h - file system wrapper
 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#include <Arduino.h>
#include <memory>

namespace fs {

#define FILE_READ	"r"
#define FILE_WRITE	"w"
#define FILE_APPEND "a"

class File;

class FileImpl;
typedef std::shared_ptr<FileImpl> FileImplPtr;
class FSImpl;
typedef std::shared_ptr<FSImpl> FSImplPtr;

enum SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2 };

class File : public Stream {
  public:
	File(FileImplPtr p = FileImplPtr()) : _p(p) {
		_timeout = 0;
	}

	size_t write(uint8_t) override;
	size_t write(const uint8_t *buf, size_t size) override;
	int available() override;
	int read() override;
	int peek() override;
	void flush() override;
	size_t read(uint8_t *buf, size_t size);

	size_t readBytes(char *buffer, size_t length) {
		return read((uint8_t *)buffer, length);
	}

	bool seek(uint32_t pos, SeekMode mode);

	bool seek(uint32_t pos) {
		return seek(pos, SeekSet);
	}

	size_t position() const;
	size_t size() const;
	bool setBufferSize(size_t size);
	void close();
	operator bool() const;
	time_t getLastWrite();
	const char *path() const;
	const char *name() const;

	boolean isDirectory(void);
	File openNextFile(const char *mode = FILE_READ);
	void rewindDirectory(void);

  protected:
	FileImplPtr _p;
};

class FileImpl {
  public:
	virtual ~FileImpl() {}

	virtual size_t write(const uint8_t *buf, size_t size) = 0;
	virtual size_t read(uint8_t *buf, size_t size)		  = 0;
	virtual void flush()								  = 0;
	virtual bool seek(uint32_t pos, SeekMode mode)		  = 0;
	virtual size_t position() const						  = 0;
	virtual size_t size() const							  = 0;
	virtual bool setBufferSize(size_t size)				  = 0;
	virtual void close()								  = 0;
	virtual time_t getLastWrite()						  = 0;
	virtual const char *path() const					  = 0;
	virtual const char *name() const					  = 0;
	virtual boolean isDirectory(void)					  = 0;
	virtual FileImplPtr openNextFile(const char *mode)	  = 0;
	virtual void rewindDirectory(void)					  = 0;
	virtual operator bool()								  = 0;
};

class FS {
  public:
	FS(FSImplPtr impl) : _impl(impl) {}

	File open(const char *path, const char *mode = FILE_READ, const bool create = false);
	File open(const String &path, const char *mode = FILE_READ, const bool create = false);

	bool exists(const char *path);
	bool exists(const String &path);

	bool remove(const char *path);
	bool remove(const String &path);

	bool rename(const char *pathFrom, const char *pathTo);
	bool rename(const String &pathFrom, const String &pathTo);

	bool mkdir(const char *path);
	bool mkdir(const String &path);

	bool rmdir(const char *path);
	bool rmdir(const String &path);

  protected:
	FSImplPtr _impl;
};

class FSImpl {
  public:
	FSImpl() {}

	virtual ~FSImpl() {}

	virtual FileImplPtr open(const char *path, const char *mode, const bool create) = 0;
	virtual bool exists(const char *path)											= 0;
	virtual bool rename(const char *pathFrom, const char *pathTo)					= 0;
	virtual bool remove(const char *path)											= 0;
	virtual bool mkdir(const char *path)											= 0;
	virtual bool rmdir(const char *path)											= 0;
};

} // namespace fs

#ifndef FS_NO_GLOBALS
using fs::File;
using fs::FS;
using fs::SeekCur;
using fs::SeekEnd;
using fs::SeekMode;
using fs::SeekSet;
#endif // FS_NO_GLOBALS
