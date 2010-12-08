/*
*/
#include <stdio.h>
#include <c_lib/c_cfg.hpp>

int main(int argc, char** argv){

	if(argc< 2) return -1;
	TCfg cfg(argv[1]);
	cfg.setToFirst();
	while(cfg.isValid()){
		printf("%s %s\n", cfg.getCurrentKey(), cfg.getCurrentValue());
		cfg.setToNext();
	}
	return 0;
}
