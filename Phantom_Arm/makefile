#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = Phantom_Arm

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
SOURCES = \
		Phantom_Arm.cpp 

MAIN_OBJS:= $(subst .cpp,.o,$(SOURCES))

MAIN_DEPS:= $(subst .cpp,.d,$(SOURCES))

#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
INCLUDE_DIRS = -I. -I../library -I../BioloidEX


#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf -L../BioloidEX -L../library
else
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf -L../BioloidEX -L../library
endif


#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -pedantic -g -O2 -fno-rtti -Wno-write-strings


#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~
LDFLAGS = $(LIBRARY_DIRS) -lpthread -lBioloidEX -lArduinoPort -lasound -lrt

#~~~~~~~~~~~~~~~~~~~~ Toolchain Prefix ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
TCHAIN_PREFIX=arm-angstrom-linux-gnueabi-
else
TCHAIN_PREFIX=arm-linux-gnueabihf-
endif
#TCHAIN_PREFIX=x86_64-linux-gnu-

CXX = $(TCHAIN_PREFIX)g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)

#~~~~~~~~~~~~~~~~~~~~ all ~~~~~~~~~~~~~~~~~~~~

all: LIBRARY begin gccversion build end

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
LIBRARY:
	cd ../library; $(MAKE)


#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~


build: $(MAIN_OUT)

$(MAIN_OUT): $(MAIN_OBJS) ../library/libArduinoPort.a
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
	-cd ../library ; make clean

#~~~~~~~~~~~~~~~~~~~~ backup ~~~~~~~~~~~~~~~~~~~~
backup: clean
	tar cJvf ../$(MAIN_OUT)_`date +"%Y-%m-%d_%H%M"`.tar.xz *

#~~~~~~~~~~~~~~~~~~~~ Dependency Generation
include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	$(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
