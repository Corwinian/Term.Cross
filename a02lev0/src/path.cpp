/*-------------------------------------------------------------------------
    c_misc.cpp
-------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include <tc_config.h>

#include <c_lib/c_misc.hpp>
#include <c_lib/c_types.hpp>

int splitpath(char* path, char* drive, char* dir, char* name, char* ext) {
	if(!path)
		return -1;
	char* drvEnd;
	char* pEnd;
	char* nEnd;
	char* tmp;
	drvEnd = pEnd = nEnd = path;
	for(tmp = path; *tmp != '\0'; tmp++) {
		switch(*tmp) {
		case '\\':
		case '/':
			pEnd = tmp+1;
			break;
		case '.':
			nEnd = tmp;
			break;
		case ':':
			drvEnd = tmp+1;
			break;
		}
	}
	if(pEnd < drvEnd)
		pEnd = drvEnd;
	if(nEnd < pEnd)
		nEnd = pEnd;

	if(drive) {
		memcpy(drive, path, (size_t) (drvEnd - path));
		drive[drvEnd - path] = '\0';
	}

	if(dir) {
		memcpy(dir, drvEnd, (size_t) (pEnd - drvEnd));
		dir[pEnd - drvEnd] = '\0';
	}

	if(name) {
		memcpy(name, pEnd, (size_t) (nEnd - pEnd));
		name[nEnd - pEnd] = '\0';
	}

	if(ext) {
		if(*nEnd == '.')
			nEnd++;
		memcpy(ext, nEnd, (size_t) (tmp - nEnd));
		ext[tmp - nEnd] = '\0';
	}

	return 0;
}

int makepath( char* path, const char* drive, const char* dir, const char* name, const char* ext ) {
	char* tmp = path;
	if(drive) {
		strcpy(tmp,drive);
		tmp+=strlen(drive);
	}
	if(dir) {
		strcpy(tmp,dir);
		tmp+=strlen(dir);
	}
	if(name) {
		if(tmp > path){
			if(*(tmp-1)!= '/'||*(tmp-1)!= '\\'){
				*tmp = DIRD; *(++tmp)='\0';
			}
		}
		strcpy(tmp,name);
		tmp+=strlen(name);
	}
	if(ext) {
		if(ext[0]!='.') {
			*tmp = '.';
			tmp++;
		}
		strcpy(tmp,ext);
	}

	return 0;
}
