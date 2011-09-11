/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


// A basic C++ wrapper for a zip file
// Adapted from miniunz.c from minizip open source code by Gilles Vollant.
// Copyright (C) 1998-2005 Gilles Vollant

#include "import/cross/zip_archive.h"
#include "contrib/minizip/unzip.h"

#include "base/cross/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef OS_POSIX
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#else
#include <direct.h>
#include <io.h>
#include <rpc.h>
#endif

using std::vector;
using std::string;

#define CASESENSITIVITY (1)  // activate case sensitivity
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (1024)

struct ZipFileInfo::LowLevelInfo {
	unz_file_info info;
};
ZipFileInfo::ZipFileInfo(): low_level_info(new ZipFileInfo::LowLevelInfo) { }
ZipFileInfo::ZipFileInfo(const ZipFileInfo& o): low_level_info(new ZipFileInfo::LowLevelInfo) {
	*low_level_info = *o.low_level_info;
	name = o.name;
}
ZipFileInfo& ZipFileInfo::operator=(const ZipFileInfo& o) {
	*low_level_info = *o.low_level_info;
	name = o.name;
	return *this;
}
ZipFileInfo::~ZipFileInfo() {
	delete low_level_info;
}

struct ZipArchive::PrivateStruct {
	std::string     zip_filename_;
	unzFile         zip_file_ref_;
	PrivateStruct(const std::string& zip_filename): zip_filename_(zip_filename), zip_file_ref_(0) { }
	~PrivateStruct() {
		if(zip_file_ref_) {
			unzClose(zip_file_ref_);
			zip_file_ref_ = 0;
		}
	}
};

// Creates a basic C++ wrapper for a zip file
ZipArchive::ZipArchive(const std::string& zip_filename, int* result)
	: private_(new ZipArchive::PrivateStruct(zip_filename)) {
	char filename_try[MAXFILENAME + 16] = "";
	strncpy(filename_try, &zip_filename[0], MAXFILENAME - 1);
	// strncpy doesn't append the trailing NULL if the string is too long.
	filename_try[MAXFILENAME] = '\0';
	private_->zip_file_ref_ = unzOpen(&zip_filename[0]);

	// try appending .zip if |zip_filename| as given wasn't found
	if(private_->zip_file_ref_ == NULL) {
		strcat(filename_try, ".zip");
		private_->zip_file_ref_ = unzOpen(filename_try);
	}

	if(private_->zip_file_ref_ == NULL) {
		O3D_LOG(WARNING) << "Cannot open " << zip_filename << " nor " << zip_filename << ".zip as a ZIP archive";

		if(result) {
			*result = 1;
			return;
		}
	}

	O3D_LOG(INFO) << "Opened ZIP archive " << filename_try;
	*result = UNZ_OK;
}

ZipArchive::~ZipArchive() {
	delete private_;
}

// The returned filenames should adhere to the zip archive spec
// (UTF8 with '/' as the path separator)
// If the zip file is badly constructed then this assumption may be invalid.
// The filenames will contain a leading '/', with '/' indicating the "root"
// of the zip archive (as if the zip archive were a filesystem)
//
int ZipArchive::GetInformationList(vector<ZipFileInfo> *infolist) {
	if(!infolist) return -1;

	unz_global_info gi;
	int result = unzGetGlobalInfo(private_->zip_file_ref_, &gi);

	if(result == UNZ_OK) {
		unzGoToFirstFile(private_->zip_file_ref_);

		for(uLong i = 0; i < gi.number_entry; ++i) {
			// get the info for this entry
			char filename_inzip[MAXFILENAME];  // MAX_PATH
			ZipFileInfo file_info;
			unz_file_info* info(&file_info.low_level_info->info);
			result = unzGetCurrentFileInfo(private_->zip_file_ref_,
			                               info,
			                               filename_inzip,
			                               sizeof(filename_inzip),
			                               NULL,
			                               0,
			                               NULL,
			                               0);
			file_info.name = "/" + string(filename_inzip);
			infolist->push_back(file_info);

			if((i + 1) < gi.number_entry) {
				result = unzGoToNextFile(private_->zip_file_ref_);

				if(result != UNZ_OK) {
					O3D_LOG(ERROR) << "error " << result << " with zipfile in unzGoToNextFile";
					break;
				}
			}
		}
	}

	return result;
}

// Returns information for |filename| in |*info|
// returns 0 if successful
// |filename| is expected to have a leading '/' (as is returned by
// GetInformationList() )
//
int ZipArchive::GetFileInfo(const string& filename, ZipFileInfo* info) {
	if(!info) return -1;

	unzGoToFirstFile(private_->zip_file_ref_);
	unzFile uf = private_->zip_file_ref_;
	string actual_filename;
	GetActualFilename(filename, &actual_filename);

	if(unzLocateFile(uf, actual_filename.c_str(), CASESENSITIVITY) != UNZ_OK) {
		O3D_LOG(ERROR) << "file " << actual_filename << " not found in the zipfile";
		return 2;
	}

	// get the info for this entry
	unz_file_info* llinfo(&info->low_level_info->info);
	char filename_inzip[MAXFILENAME];
	int result = unzGetCurrentFileInfo(uf,
	                                   llinfo,
	                                   filename_inzip,
	                                   sizeof(filename_inzip),
	                                   NULL,
	                                   0,
	                                   NULL,
	                                   0);
	info->name = "/" + string(filename_inzip);
	return result;
}

// Extracts the entire archive to disk
int ZipArchive::Extract() {
	const char* filename_to_extract = NULL;
	const char* password = NULL;
	int opt_do_extract = 1;
	int opt_do_extract_withoutpath = 0;
	int opt_overwrite = 0;
	int opt_extractdir = 0;
	const char* dirname = NULL;

	if(opt_do_extract == 1) {
		if(opt_extractdir) {
			if(::chdir(dirname)) {
				O3D_LOG(ERROR) << "Error changing into " << dirname;
				return 2;
			}
		}

		if(filename_to_extract == NULL) {
			return DoExtract(opt_do_extract_withoutpath,
			                 opt_overwrite,
			                 password);
		}
		else {
			return ExtractOneFile(filename_to_extract,
			                      opt_do_extract_withoutpath,
			                      opt_overwrite,
			                      password);
		}
	}

	unzCloseCurrentFile(private_->zip_file_ref_);
	return 1;
}

// Extracts a single file to disk
int ZipArchive::ExtractOneFile(const string& filename,
                               int opt_extract_without_path,
                               int opt_overwrite,
                               const char* password) {
	string actual_filename;
	GetActualFilename(filename, &actual_filename);
	unzGoToFirstFile(private_->zip_file_ref_);

	if(unzLocateFile(private_->zip_file_ref_,
	                 actual_filename.c_str(),
	                 CASESENSITIVITY) != UNZ_OK) {
		O3D_LOG(ERROR) << "file " << actual_filename << " not found in the zipfile";
		return 2;
	}

	if(ExtractCurrentFile(&opt_extract_without_path,
	                      &opt_overwrite,
	                      password) == UNZ_OK) {
		return UNZ_OK;
	}
	else {
		return 1;
	}
}


// Extracts a single file and returns a pointer to the file's content.
// Returns NULL if |filename| doesn't match any in the archive
// or an error occurs.  The caller must call free() on the returned pointer
char*  ZipArchive::GetFileData(const string& filename, size_t* size) {
	string actual_filename;
	GetActualFilename(filename, &actual_filename);
	unzFile uf = private_->zip_file_ref_;
	unzGoToFirstFile(uf);

	if(unzLocateFile(uf, actual_filename.c_str(), CASESENSITIVITY) != UNZ_OK) {
		O3D_LOG(ERROR) << "file " << actual_filename << " not found in the zipfile";
		return NULL;
	}

	// determine the size of the uncompressed file
	unz_file_info file_info;
	char filename_inzip[MAXFILENAME];
	int result = unzGetCurrentFileInfo(uf,
	                                   &file_info,
	                                   filename_inzip,
	                                   sizeof(filename_inzip),
	                                   NULL,
	                                   0,
	                                   NULL,
	                                   0);

	if(result != UNZ_OK) return NULL;

	if(size) *size = file_info.uncompressed_size;

	result = unzOpenCurrentFilePassword(uf, NULL);
	char* buffer = NULL;

	if(result == UNZ_OK) {
		const int kBufferChunkSize = 32768;
		void* temp_buffer = malloc(kBufferChunkSize);
		// allocate one extra byte so we can NULL terminate
		// but don't report this extra byte in the |size| we return
		// NULL terminating is useful if the data retrieved is to be interpreted
		// as string data and doesn't harm anything else
		buffer = reinterpret_cast<char*>(malloc(file_info.uncompressed_size + 1));
		buffer[file_info.uncompressed_size] = 0;
		uint32_t buffer_index = 0;
		uint32_t nbytes;

		do {
			nbytes = unzReadCurrentFile(uf, temp_buffer, kBufferChunkSize);

			if(nbytes < 0) {
				O3D_LOG(ERROR) << "error " << result << " with zipfile in unzReadCurrentFile";
				result = -1;
				break;
			}

			if(nbytes > 0) {
				// check that we're not exceeding the expected uncompressed size!
				if(buffer_index + nbytes > file_info.uncompressed_size) {
					result = -2;
					break;
				}

				memcpy(buffer + buffer_index, temp_buffer, nbytes);
				buffer_index += nbytes;
			}
		}
		while(nbytes > 0);

		free(temp_buffer);

		if(result == UNZ_OK) {
			result = unzCloseCurrentFile(uf);
		}
	}

	if(result != UNZ_OK) {
		free(buffer);
		buffer = NULL;
	}

	return buffer;
}

// Convert paths relative to |root_path| to archive paths
// The |root_path| should start with '/' and should be an actual
// directory in the zip archive.
//
char*  ZipArchive::GetRelativeFileData(const string& relative_path,
                                       const string& root_path,
                                       size_t* size) {
	string converted_filename(relative_path);
	ConvertRelativeToAbsolutePath(&converted_filename, root_path);
	return GetFileData(converted_filename, size);
}

// private/protected methods
//
#ifdef OS_MACOSX
#pragma mark -
#endif

int ZipArchive::ExtractCurrentFile(const int* popt_extract_without_path,
                                   int* popt_overwrite,
                                   const char* password) {
	int result = UNZ_OK;
	unz_file_info file_info;
	char filename_inzip[MAXFILENAME];
	result = unzGetCurrentFileInfo(private_->zip_file_ref_,
	                               &file_info,
	                               filename_inzip,
	                               sizeof(filename_inzip),
	                               NULL,
	                               0,
	                               NULL,
	                               0);

	if(result != UNZ_OK) {
		O3D_LOG(ERROR) << "error " << result << " with zipfile in unzGetCurrentFileInfo";
		return result;
	}

	O3D_LOG(INFO) << "ExtractCurrentFile: " << filename_inzip;
	uInt size_buf = WRITEBUFFERSIZE;
	void* buf = malloc(size_buf);
	char* filename_withoutpath;
	char* p;
	p = filename_withoutpath = filename_inzip;

	while((*p) != '\0') {
		if(((*p) == '/') || ((*p) == '\\'))
			filename_withoutpath = p + 1;

		p++;
	}

	if((*filename_withoutpath) == '\0') {
		if((*popt_extract_without_path) == 0) {
			O3D_LOG(INFO) << "creating directory: " << filename_inzip;
			MyMkDir(filename_inzip);
		}
	}
	else {
		char* write_filename;
		int skip = 0;

		if((*popt_extract_without_path) == 0) {
			write_filename = filename_inzip;
		}
		else {
			write_filename = filename_withoutpath;
		}

		result = unzOpenCurrentFilePassword(private_->zip_file_ref_, password);

		if(result != UNZ_OK) {
			O3D_LOG(ERROR) << "error " << result << " with zipfile in unzOpenCurrentFilePassword";
		}

		FILE* fout = NULL;

		if((skip == 0) && (result == UNZ_OK)) {
			O3D_LOG(INFO) << "fopen: " << write_filename;
			fout = fopen(write_filename, "wb");

			// some zipfiles don't contain directory alone before file
			if((fout == NULL) && ((*popt_extract_without_path) == 0) &&
			        (filename_withoutpath != reinterpret_cast<char*>(filename_inzip))) {
				char c = *(filename_withoutpath - 1);
				*(filename_withoutpath - 1) = '\0';
				MakeDir(write_filename);
				*(filename_withoutpath - 1) = c;
				fout = fopen(write_filename, "wb");
			}

			if(fout == NULL) {
				O3D_LOG(ERROR) << "error opening " << write_filename;
			}
		}

		if(fout != NULL) {
			O3D_LOG(INFO) << "extracting: " << write_filename;

			do {
				result = unzReadCurrentFile(private_->zip_file_ref_, buf, size_buf);

				if(result < 0) {
					O3D_LOG(ERROR) << "error " << result << " with zipfile in unzReadCurrentFile";
					break;
				}

				if(result > 0)
					if(fwrite(buf, result, 1, fout) != 1) {
						O3D_LOG(ERROR) << "error in writing extracted file";
						result = UNZ_ERRNO;
						break;
					}
			}
			while(result > 0);

			if(fout) {
				fclose(fout);
			}
		}

		if(result == UNZ_OK) {
			result = unzCloseCurrentFile(private_->zip_file_ref_);

			if(result != UNZ_OK) {
				O3D_LOG(ERROR) << "error " << result << " with zipfile in unzCloseCurrentFile";
			}
		}
		else {
			unzCloseCurrentFile(private_->zip_file_ref_);  // don't lose the error
		}
	}

	free(buf);
	return result;
}

int ZipArchive::DoExtract(int opt_extract_without_path,
                          int opt_overwrite,
                          const char* password) {
	unz_global_info gi;
	int result = unzGetGlobalInfo(private_->zip_file_ref_, &gi);

	if(result != UNZ_OK)
		O3D_LOG(ERROR) << "error " << result << " with zipfile in unzGetGlobalInfo";

	for(uLong i = 0; i < gi.number_entry; ++i) {
		if(ExtractCurrentFile(&opt_extract_without_path,
		                      &opt_overwrite,
		                      password) != UNZ_OK)
			break;

		if((i + 1) < gi.number_entry) {
			result = unzGoToNextFile(private_->zip_file_ref_);

			if(result != UNZ_OK) {
				O3D_LOG(ERROR) << "error " << result << " with zipfile in unzGoToNextFile";
				break;
			}
		}
	}

	return UNZ_OK;
}

int ZipArchive::MyMkDir(const char* dirname) {
	int ret = 0;
#if defined(OS_LINUX) || defined(OS_MACOSX)
	ret = ::mkdir(dirname, 0775);
#endif
	return ret;
}

int ZipArchive::MakeDir(const char* newdir) {
	int  len = static_cast<int>(strlen(newdir));

	if(len <= 0)
		return 0;

	char* buffer = reinterpret_cast<char*>(malloc(len + 1));
	strcpy(buffer, newdir);

	if(buffer[len - 1] == '/') {
		buffer[len - 1] = '\0';
	}

	if(MyMkDir(buffer) == 0) {
		free(buffer);
		return 1;
	}

	char* p = buffer + 1;

	while(1) {
		while(*p && *p != '\\' && *p != '/')
			p++;

		char hold = *p;
		*p = 0;

		if((MyMkDir(buffer) == -1) && (errno == ENOENT)) {
			O3D_LOG(ERROR) << "couldn't create directory " << buffer;
			free(buffer);
			return 0;
		}

		if(hold == 0)
			break;

		*p++ = hold;
	}

	free(buffer);
	return 1;
}


bool ZipArchive::IsZipFile(const std::string& filename) {
	int result;
	// If we can open it, it's a zip file.
	ZipArchive archive(filename, &result);
	return result == UNZ_OK;
}

#ifdef OS_MACOSX
#pragma mark -
#endif

// assumes |path| is UTF8 with '/' as the path separator
void ZipArchive::RemoveLastPathComponent(string* path) {
	// This gets rid of trailing slashes, if any.
	int length = path->size();

	while((*path)[length - 1] == '/' && length > 0) {
		path->resize(length - 1);
		length = path->size();
	}

	string::size_type index = path->find_last_of('/');

	if(index == string::npos) {
		*path = "";
	}
	else {
		path->resize(index + 1);  // keep a trailing '/'
	}
}

// This assumes |path| is UTF8 with '/' as the path separator normally
// it should be a relative IETF URI path
void ZipArchive::ConvertRelativeToAbsolutePath(string* rel_path,
        const string& root_path) {
	string base_path(root_path);
	string path(*rel_path);

	if(!path.empty() && path[0] == '/') {
		// Path is already absolute.
		return;
	}
	else {
		while(path.size() >= 2 && path[0] == '.' && path[1] == '/') {
			path = path.substr(2);  // strip off leading ./'s
		}

		while(path.find("../") == 0) {
			path = path.substr(3);  // strip off a leading ../
			RemoveLastPathComponent(&base_path); // strip off a base dir element.
		}

		*rel_path = base_path + path;
	}
}

// This removes leading '/' which is the form that the minizip library
// likes.  The public ZipArchive API expects pathnames to have the
// leading '/' treating the zip archive as a file-system rooted at '/'
void ZipArchive::GetActualFilename(const string& filename,
                                   string* actual_filename) {
	if(filename.find('/') == 0) {
		*actual_filename = filename.substr(1);
	}
	else {
		*actual_filename = filename;
	}
}

bool ZipArchive::GetTempFileFromFile(const string& filename,
                                     string* temp_filename) {
	if(!temp_filename) return false;

	size_t data_size;
	char* data = GetFileData(filename, &data_size);

	if(data) {
		// get just the final path component
		string::size_type pos = filename.rfind('/');

		if(pos != string::npos) {
			// TODO : need to get "proper" temp dir for user
			// TODO : need to append GUID to filename
			std::string tmp = "/tmp/" + filename.substr(pos + 1);
			FILE* fp = fopen(tmp.c_str(), "w");

			if(fp) {
				fwrite(data, 1, data_size, fp);
				fclose(fp);
				*temp_filename = tmp;
			}
			else {
				return false;
			}
		}
	}

	return true;
}

void ZipArchive::DeleteFile(const string& filename) {
	::unlink(filename.c_str());
}
