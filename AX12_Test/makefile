#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = AX12_Test

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
SOURCES = \
		AX12_Test.cpp 

MAIN_OBJS:= $(subst .cpp,.o,$(SOURCES))

MAIN_DEPS:= $(subst .cpp,.d,$(SOURCES))

#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
INCLUDE_DIRS = -I. -I../library -I../BioloidEX_Usb2Ax


#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf -L../BioloidEX_Usb2Ax -L../library
else
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf -L../BioloidEX_Usb2Ax -L../library
endif


#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -pedantic -g -O2 -fno-rtti -Wno-write-strings


#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~
LDFLAGS = $(LIBRARY_DIRS) -lpthread -lBioloidEX -lArduinoPort -lrt

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

all: LIBRARY BioloidEX_Usb2Ax begin gccversion build end

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
LIBRARY:
	cd ../library; $(MAKE)

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
BioloidEX_Usb2Ax:
	cd ../BioloidEX_Usb2Ax; $(MAKE)


#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~


build: $(MAIN_OUT)

$(MAIN_OUT): $(MAIN_OBJS) ../library/libArduinoPort.a ../BioloidEX_Usb2Ax/libBioloidEX.a
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
	-cd ../BioloidEX_Usb2Ax ; make clean

#~~~~~~~~~~~~~~~~~~~~ backup ~~~~~~~~~~~~~~~~~~~~
backup: clean
	tar cJvf ../$(MAIN_OUT)_`date +"%Y-%m-%d_%H%M"`.tar.xz *

#~~~~~~~~~~~~~~~~~~~~ Dependency Generation
include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	$(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
