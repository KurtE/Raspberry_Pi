#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = testJoystick

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
SOURCES = \
		Joystick.cpp
MAIN_OBJS:= $(subst .cpp,.o,$(SOURCES))

MAIN_DEPS:= $(subst .cpp,.d,$(SOURCES))

#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
INCLUDE_DIRS = -I.

#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf

#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -pedantic -g -O2 -fno-rtti
#COMPILE_OPTS = -Wall -pedantic -g -O2

#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~
LDFLAGS = $(LIBRARY_DIRS) -lpthread 

#~~~~~~~~~~~~~~~~~~~~ Toolchain Prefix ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
TCHAIN_PREFIX=arm-angstrom-linux-gnueabi-
else
ifeq ($(OSTYPE),Edison)
#Edison hard coded OSTYPE
TCHAIN_PREFIX=i586-poky-linux-
else
TCHAIN_PREFIX=arm-linux-gnueabihf-
endif
endif
#TCHAIN_PREFIX=x86_64-linux-gnu-

CXX = $(TCHAIN_PREFIX)g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)

#~~~~~~~~~~~~~~~~~~~~ all ~~~~~~~~~~~~~~~~~~~~
all: begin gccversion build end

#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build: $(MAIN_OUT)

$(MAIN_OUT): $(MAIN_OBJS)
	$(CXX) $(CXXFLAGS) $(MAIN_OBJS) -o $(MAIN_OUT) $(LDFLAGS)

MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------

#~~~~~~~~~~~~~~~~~~~~ Eye candy ~~~~~~~~~~~~~~~~~~~~
begin:
	@echo
	@echo $(MSG_BEGIN)

end:
	@echo $(MSG_END)
	@echo

gccversion:
	@$(CC) --version

#~~~~~~~~~~~~~~~~~~~~ clean ~~~~~~~~~~~~~~~~~~~~
clean: begin clean_list end

clean_list:
	-rm $(MAIN_OBJS)
	-rm $(MAIN_OUT)
	-rm $(MAIN_DEPS)

#~~~~~~~~~~~~~~~~~~~~ backup ~~~~~~~~~~~~~~~~~~~~
backup: clean
	tar cJvf ../$(MAIN_OUT)_`date +"%Y-%m-%d_%H%M"`.tar.xz *

#~~~~~~~~~~~~~~~~~~~~ Dependency Generation
include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
