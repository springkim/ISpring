﻿/**
* @file		FileManager.h
* @author		kimbomm (springnode@gmail.com)
* @date		2017. 05. 23...
* @version	1.1.0
*
*  @brief
*			파일, 폴더 조작 라이브러리
*	@remark
*			Created by kimbom on 2017. 05. 23...
*			Copyright 2017 kimbom.All rights reserved.
*/
#if !defined(ISPRING_7E1_05_17_FILE_H_INCLUDED)
#define ISPRING_7E1_05_17_FILE_H_INCLUDED
#include"../defines.h"
#if defined(ISPRING_WINDOWS) || defined(DOXYGEN)
#include<vector>
#include<string>
#include<algorithm>
#ifndef DOXYGEN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#include<Windows.h>
#include<direct.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include"../Verify/VerifyError.h"
namespace ispring {
	/**
	*	@brief 이 정적 클래스는 파일, 폴더를 조작하는 함수를 포함합니다.
	*	@author kimbomm
	*	@date 2017-05-23
	*/
	namespace File{
		/**
		*	@brief 폴더에서 파일들의 경로를 가져 옵니다.
		*	@param dir_path 폴더 경로
		*	@param ext  가져올 파일의 확장자
		*	@param recursive  재귀적으로 탐색해 파일을 가져올지 결정합니다.
		*	@return 파일 경로들의 집합
		*	@details 
		*			확장자가 jpg인 파일들만 가져오고 싶다면 "*.jpg" 또는 "jpg"를 입력하면 됩니다.
		*			여러개의 확장자는 ; 로 구분 합니다. ex) "*.jpg;*.png"
		*			모든 확장자를 가져오고 싶으면 "*.*" 을 입력 합니다.
		*/
		inline std::vector<std::string> FileList(std::string dir_path, std::string ext, bool recursive = false,bool folder=false) {
			std::vector<std::string> paths; //return value
			if (dir_path.back() != '/' && dir_path.back() != '\\') {
				dir_path.push_back('/');
			}
			std::string str_exp = dir_path + "*.*";
			std::vector<std::string> allow_ext;
			std::string::size_type offset = 0;
			while (offset < ext.length()) {
				std::string str = ext.substr(offset, ext.find(';', offset) - offset);
				std::transform(str.begin(), str.end(), str.begin(), toupper);
				offset += str.length() + 1;
				std::string::size_type pos = str.find_last_of('.');
				pos = pos == std::string::npos ? 0 : pos + 1;
				allow_ext.push_back(str.substr(pos, str.length()));
			}
			WIN32_FIND_DATAA fd;
			HANDLE hFind = ::FindFirstFileA(str_exp.c_str(), &fd);
			if (hFind == INVALID_HANDLE_VALUE) {
				return paths;
			}
			do {
				std::string path = fd.cFileName;
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { //if this is file
					std::string path_ext = path.substr(path.find_last_of('.') + 1, path.length());  //파일의 확장자 추출
					std::transform(path_ext.begin(), path_ext.end(), path_ext.begin(), toupper);
					int i = -1;
					while (++i < (int)allow_ext.size() && allow_ext[i] != path_ext);
					if (i < (int)allow_ext.size() || allow_ext.front() == "*") {    //allow_ext에 포함되어있으면
						paths.push_back(dir_path + path);
					}
				}
				else if (recursive == true && path != "." && path != "..") {
					std::vector<std::string> temps = FileList(dir_path + path, ext, recursive);
					for (auto&temp : temps) {
						paths.push_back(temp);
					}
				}else if(folder==true){
				    paths.push_back(dir_path+path);
				}
			} while (::FindNextFileA(hFind, &fd));
			::FindClose(hFind);
			return paths;   //RVO
		}
		/**
		*	@brief 파일이 존재 하는지 확인 합니다.
		*	@param file 파일의 경로
		*	@return 파일이 존재하면 true
		*/
		inline bool FileExist(std::string file) {
			return PathFileExistsA(file.c_str()) == TRUE;
		}
		/**
		*	@brief 파일을 삭제 합니다.
		*	@param file 파일의 경로
		*	@return 삭제에 성공하면 true
		*/
		inline bool FileErase(std::string file) {
			return DeleteFileA(file.c_str()) == TRUE;
		}
		/**
		*	@brief 파일을 복사합니다.
		*	@param src 복사할 파일의 경로
		*	@param dst 목적 파일의 경로
		*	@return 복사에 성공하면 true
		*/
		inline bool FileCopy(std::string src, std::string dst) {
			return CopyFileA(src.c_str(), dst.c_str(), FALSE) == TRUE;
		}
		/**
		*	@brief 폴더를 삭제 합니다.
		*	@param dir_path 폴더의 경로
		*	@param noRecycleBin 이 값이 true이면 삭제한 파일을 휴지통에 넣지 않습니다.
		*	@return 삭제에 성공하면 true
		*/
		inline bool DirectoryErase(std::string dir_path, bool noRecycleBin = true) {
			char *pszFrom = new char[dir_path.length() + 2];
#ifdef _MSC_VER
			strcpy_s(pszFrom, dir_path.length() + 2, dir_path.c_str());
#elif __GNUC__
			strcpy(pszFrom, dir_path.c_str());
#endif
			pszFrom[dir_path.length()] = '\0';       // double-null termination
			pszFrom[dir_path.length() + 1] = '\0';   // double-null termination
			SHFILEOPSTRUCTA	FileOp;
			FileOp.hwnd = NULL;
			FileOp.wFunc = FO_DELETE;
			FileOp.pFrom = pszFrom;
			FileOp.pTo = NULL;
			FileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
			if (noRecycleBin == false) {
				FileOp.fFlags |= FOF_ALLOWUNDO;
			}
			FileOp.fAnyOperationsAborted = FALSE;
			FileOp.lpszProgressTitle = NULL;
			FileOp.hNameMappings = NULL;
			int ret = SHFileOperationA(&FileOp);
			delete[] pszFrom;
			return (ret == 0);
		}
		/**
		*	@brief 폴더를 복사합니다.
		*	@param src 복사할 폴더의 경로
		*	@param dst 목적 폴더의 경로
		*	@return 복사에 성공하면 true
		*/
		inline bool DirectoryCopy(std::string src, std::string dst) {
			std::string new_sf = src + "\\*";
			char sf[MAX_PATH + 1];
			char tf[MAX_PATH + 1];
#ifdef _MSC_VER
			strcpy_s(sf, MAX_PATH, new_sf.c_str());
			strcpy_s(tf, MAX_PATH, dst.c_str());
#elif __GNUC__
			strcpy(sf,  new_sf.c_str());
			strcpy(tf, dst.c_str());
#endif
			sf[strlen(sf) + 1] = 0;
			tf[strlen(tf) + 1] = 0;
			SHFILEOPSTRUCTA s = { 0 };
			s.wFunc = FO_COPY;
			s.pTo = tf;
			s.pFrom = sf;
			s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI;
			int res = SHFileOperationA(&s);
			if (res == 0xB7) {	//MAX_PATH problem
				auto files = FileList(src,"*.*",true);
				decltype(files) rfiles(files);
				for(int i=0;i<files.size();i++){
					
					std::string rfile = files[i].substr(src.length(), files[i].length() - src.length() + 1);
					if (dst.back() == '/' || dst.back() == '\\') {
						if (rfile.front() == '/' || rfile.front() == '\\') {
							dst.pop_back();
						}
					}
					else {
						if (rfile.front() != '/' && rfile.front() != '\\') {
							dst.push_back('\\');
						}
					}
					std::string csrc(dst + rfile);
					std::wstring wdst(csrc.begin(),csrc.end());
					std::wstring wsrc(files[i].begin(), files[i].end());
					CopyFileW(wsrc.data(), wdst.data(), FALSE);
				}

			}
			return res == 0;
		}
		/**
		*	@brief 폴더를 생성합니다.
		*	@param dir 생성할 폴더
		*	@return 생성에 성공하면 true
		*/
		inline bool DirectoryMake(std::string dir) {
			int r=_mkdir(dir.c_str());;
			if (r == ENOENT) {
				ISPRING_VERIFY("Path was not found.");
			}
			return r == 0;
		}
		/**
		*	@brief 폴더가 존재 하는지 확인 합니다.
		*	@param dir 폴더의 경로
		*	@return 폴더가 존재하면 true
		*/
		inline bool DirectoryExist(std::string dir) {
			DWORD ftyp = GetFileAttributesA(dir.c_str());
			return ftyp == FILE_ATTRIBUTE_DIRECTORY;
		}
		
	};
};
#elif defined(ISPRING_LINUX)
#include<vector>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
namespace ispring{
    namespace File{
        inline std::vector<std::string> FileList(std::string dir_path, std::string ext, bool recursive = false) {
			std::vector<std::string> paths; //return value
			if (dir_path.back() != '/' && dir_path.back() != '\\') {
				dir_path.push_back('/');
			}
			std::string str_exp = dir_path + "*.*";
			std::vector<std::string> allow_ext;
			std::string::size_type offset = 0;
			while (offset < ext.length()) {
				std::string str = ext.substr(offset, ext.find(';', offset) - offset);
				std::transform(str.begin(), str.end(), str.begin(), toupper);
				offset += str.length() + 1;
				std::string::size_type pos = str.find_last_of('.');
				pos = pos == std::string::npos ? 0 : pos + 1;
				allow_ext.push_back(str.substr(pos, str.length()));
			}
			DIR* fd=opendir(dir_path.c_str());
			if (fd == NULL) {
				return paths;
			}
			struct dirent* hFind=NULL;
			while(hFind=readdir(fd)){
				std::string path = hFind->d_name;
				if(hFind->d_type==DT_REG) {    //is File?
					std::string path_ext = path.substr(path.find_last_of('.') + 1, path.length());  //파일의 확장자 추출
					std::transform(path_ext.begin(), path_ext.end(), path_ext.begin(), toupper);
					int i = -1;
					while (++i < (int) allow_ext.size() && allow_ext[i] != path_ext);
					if (i < (int) allow_ext.size() || allow_ext.front() == "*") {    //allow_ext에 포함되어있으면
						paths.push_back(dir_path + path);
					}
				}else if (recursive == true && path != "." && path != "..") {	//is Directory?
					std::vector<std::string> temps = FileList(dir_path + path, ext, recursive);
					for (auto&temp : temps) {
						paths.push_back(temp);
					}
				}
			}
			closedir(fd);
			return paths;   //RVO
		}
		inline bool FileExist(std::string file) {
			return access(file.c_str(),F_OK)==0;
		}
		inline bool FileErase(std::string file) {
			return remove(file.c_str()) == 0;
		}
		inline bool FileCopy(std::string src, std::string dst) {
			//https://stackoverflow.com/questions/3680730/c-fileio-copy-vs-systemcp-file1-x-file2-x
			std::ifstream f1 (src, std::fstream::binary);
			std::ofstream f2 (dst, std::fstream::trunc|std::fstream::binary);
			if(f1.is_open()==false || f2.is_open()==false){
				f1.close();
				f2.close();
				return false;
			}
			f2 << f1.rdbuf ();
			f2.close();
			f1.close();
			return true;
		}
		inline int64_t FileSize(std::string file){
			FILE* fp=fopen(file.c_str(),"r");
			fseek(fp,0,SEEK_END);
			long sz=ftell(fp);
			fclose(fp);
			return (int64_t)sz;
		}
		inline bool DirectoryErase(std::string dir_path, bool noRecycleBin = true) {
			std::string cmd="rm -r " + dir_path;
			return system(cmd.c_str())==0;
		}
		inline bool DirectoryCopy(std::string src, std::string dst) {
			std::string cmd="cp -r " + src + " " + dst;
			return system(cmd.c_str())==0;
		}

		inline bool DirectoryMake(std::string dir) {
			int r=mkdir(dir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			return r == 0;
		}
		inline bool DirectoryExist(std::string dir) {
			struct stat statbuf;
			if (stat(dir.c_str(), &statbuf) != 0)
				return 0;
			return S_ISDIR(statbuf.st_mode);
		}
    };
}
#endif
#endif