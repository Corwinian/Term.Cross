/*
*/

#include <c_lib.hpp>
#include <stdio.h>

char* test = "/opt/term.cross/bin/aufitchip";
int main(){
	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], ext[MAX_EXT], path[MAX_PATH];
	splitpath(test,drive,dir,fname,ext);
	printf("test  : %s\n",test);
	printf("drive : %s\n",drive);
	printf("dir   : %s\n",dir);
	printf("fname : %s\n",fname);
	printf("ext   : %s\n",ext);
}
