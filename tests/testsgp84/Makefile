#
#	log driver shared library
#

name	:= testsgp84

ifeq ($(strip $(ext)),)
ext		:= .exe
endif
#also exist libpref - library name prefix

ifeq ($(strip $(root)),)
root	:= ../..
endif

include $(root)/include/Makefile.incl

prg_root	:= $(root)/$(name)

ifeq ($(strip $(cxx)),)
cxx     := g++
endif

ifeq ($(debug),on)
bld_dir			:=$(prg_root)/debug$(sys)
compile_flags	:=-O0 -g -DDEBUG
lt              :=debug$(sys)
else
bld_dir			:=$(prg_root)/release$(sys)
ifeq ($(sys),_os2)
compile_flags	:=-O $(compile_flags)
else
compile_flags	:=-O3 $(compile_flags)
endif
lt              :=release$(sys)
endif
override compile_flags += $(cxxflags)

incl    := $(root)/include \
           $(root)/c_lib/include \
           $(root)/y_util/include \
           $(root)/orbmodel/include \
           $(prg_root)/include 
libs    := orbmodel y_util c_lib stdc++
libdirs := $(root)/c_lib/$(lt) \
           $(root)/y_util/$(lt) \
           $(root)/orbmodel/$(lt)

src_dir		:= $(prg_root)/src
src_files	:= $(wildcard $(addsuffix /*.cpp,$(src_dir)))
obj_files	:= $(patsubst %.cpp,%.o, $(notdir $(src_files)))

all: $(name)

info:
	@echo $@
	@echo "source dir   : $(src_dir)"
	@echo "source files : $(notdir $(src_files))"
	@echo "target dir   : $(bld_dir)"
	@echo "object files : $(obj_files)"
	@echo "target       : $(name)$(ext)"
	@echo "libdirs      : $(libdirs)"
	@echo "root         : $(root)"
	


$(name):$(obj_files)
	@echo $(libpref)$@$(ext)
#	$(cxx) $^ -v $(loadccflags) $(addprefix -l,$(libs)) $(addprefix -L,$(libdirs)) -o $@$(ext)
ifeq ($(strip $(noshared)),)
#	@echo shared
	$(cxx) $^ $(loadccflags) $(addprefix -l,$(libs)) $(addprefix -L,$(libdirs)) -o $@$(ext)
endif
ifeq ($(strip $(nostatic)),)
#	@echo static
	$(cxx) -static $^ $(loadccflags) $(addprefix -l,$(libs)) $(addprefix -L,$(libdirs)) -o $@.static$(ext)
endif


VPATH := $(src_dir)

%.o: %.cpp
	@echo $@
	$(cxx) -c -MD $(compile_flags) $(addprefix -I,$(incl)) -o $@ $<

include $(wildcard $(bld_dir)/*.d)

clean:
	@echo $@
	rm -f *.o
	rm -f *.d

cleanall:
	@echo $@
	@rm -f *.o
	@rm -f *.d
	@rm -f *.exe
	@rm -f *.dll
	@rm -f *.lib
	@rm -f *.so
	@rm -f *.a

install:
	@echo "Install ..."
ifeq ($(strip $(instdir)),)
	@echo "Install dir isnt seted."
else
	@cp -u *.exe $(strip $(instdir))/bin/
endif
