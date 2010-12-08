#
#	y_util shared library
#

name    := test
ext     := .exe

ifeq ($(strip $(root)),)
root    := ../..
endif

include $(root)/include/Makefile.incl

prg_root        := $(root)/$(name)

ifeq ($(strip $(cxx)),)
cxx     := g++
endif

ifeq ($(debug),on)
bld_dir         :=$(prg_root)/debug$(sys)
compile_flags   :=-O0 -g -DDEBUG
lt              :=debug$(sys)
else
bld_dir         :=$(prg_root)/release$(sys)
ifeq ($(sys),_os2)
compile_flags	:=-O
else
compile_flags	:=-O3
endif
lt              :=release$(sys)
endif
override compile_flags += $(cxxflags)

incl    := $(root)/include \
           $(root)/c_lib/include \
           $(root)/hrpt/include \
           $(root)/log/include \
           $(root)/y_util/include \
           $(root)/orbmodel/include \
           $(prg_root)/include 
libs    := c_lib stdc++
libdirs := $(root)/c_lib/$(lt) \
           $(root)/hrpt/$(lt) \
           $(root)/log/$(lt) \
           $(root)/y_util/$(lt) \
           $(root)/orbmodel/$(lt)

src_dir    := $(prg_root)/src
src_files  := $(wildcard $(addsuffix /*.cpp,$(src_dir)))
obj_files  := $(patsubst %.cpp,%.o, $(notdir $(src_files)))

VPATH      := $(src_dir)

all: $(obj_files)
	@echo programms compiled

%.o: %.cpp
	@echo $@
	g++ -MD -c $(compile_flags) -o $@ $< $(addprefix -I,$(incl))
ifeq ($(strip $(noshared)),)
#	@echo shared
	g++ -o $(patsubst %.o,%$(ext),$@) $@  $(loadccflags) $(addprefix -l,$(libs)) $(addprefix -L,$(libdirs))
endif
ifeq ($(strip $(nostatic)),)
#	@echo static
	g++ -static -o $(patsubst %.o, %.static$(ext),$@) $@ $(loadccflags) $(addprefix -l,$(libs)) $(addprefix -L,$(libdirs))
endif

include $(wildcard $(bld_dir)/*.d)

clean:
	@echo $@
	@rm -f *.o
	@rm -f *.d

cleanall:
	@echo $@
	@rm -f *.o
	@rm -f *.d
	@rm -f *.exe
	@rm -f *.dll
	@rm -f *.lib
	@rm -f *.so
	@rm -f *.a

info:
	@echo $@
	@echo "source dir   : $(src_dir)"
	@echo "source files : $(notdir $(src_files))"
	@echo "target dir   : $(bld_dir)"
	@echo "object files : $(obj_files)"
	@echo "target       : $(name)$(ext)"

