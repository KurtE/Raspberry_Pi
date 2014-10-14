#try to create a static library of code for me to use for different programs
#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = libBioloidEX.a

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
COMMON_SOURCES = \
		BioloidEX.cpp \
		dxl_hal.cpp \
		dynamixel.cpp

SOURCES = $(COMMON_SOURCES) $(EXTRA_SOURCES)

MAIN_OBJS:= $(subst .cpp,.o,$(SOURCES))

MAIN_DEPS:= $(subst .cpp,.d,$(SOURCES))

#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
INCLUDE_DIRS = -I. 

else
INCLUDE_DIRS = -I. 
endif

#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -Wall -pedantic -g -O2 -fno-rtti -Wno-write-strings

#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~

#~~~~~~~~~~~~~~~~~~~~ Toolchain Prefix ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
#BBBk
TCHAIN_PREFIX=arm-angstrom-linux-gnueabi-
else
ifeq ($(OSTYPE),Edison)
#Edison hard coded OSTYPE
TCHAIN_PREFIX=i586-poky-linux-
else
#RPI
TCHAIN_PREFIX=arm-linux-gnueabihf-
endif
endif
#TCHAIN_PREFIX=x86_64-linux-gnu-

CXX = $(TCHAIN_PREFIX)g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)
AR = ar
ARFLAGS = rcs

#~~~~~~~~~~~~~~~~~~~~ all ~~~~~~~~~~~~~~~~~~~~
all: begin gccversion build end

#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build: $(MAIN_OUT)

$(MAIN_OUT): $(MAIN_OBJS)
	$(AR) $(ARFLAGS) $(MAIN_OUT) $(MAIN_OBJS)

MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------

#~~~~~~~~~~~~~~~~~~~~ Eye candy ~~~~~~~~~~~~~~~~~~~~
begin:
	@echo
	@echo $(MSG_BEGIN)
	@echo ${OSTYPE}

end:
	@echo $(MSG_END)
	@echo

gccversion:
	@$(CC) --version

#~~~~~~~~~~~~~~~~~~~~ clean ~~~~~~~~~~~~~~~~~~~~
clean: begin clean_list end

clean_list:
	-rm -f $(MAIN_OBJS)
	-rm -f $(MAIN_OUT)
	-rm -f $(MAIN_DEPS)

#~~~~~~~~~~~~~~~~~~~~ backup ~~~~~~~~~~~~~~~~~~~~
backup: clean
	tar cJvf ../$(MAIN_OUT)_`date +"%Y-%m-%d_%H%M"`.tar.xz *

#~~~~~~~~~~~~~~~~~~~~ Dependency Generation
include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
