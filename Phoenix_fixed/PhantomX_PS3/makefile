#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = Phantom_Phoenix

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
SOURCES = \
		Phantom_Phoenix.cpp \
		../src/Phoenix_Code.cpp \
		../src/Phoenix_Input_PS3.cpp \
		../src/Phoenix_Driver_AX12.cpp 

MAIN_OBJS:= $(notdir $(subst .cpp,.o,$(SOURCES)))

MAIN_DEPS:= $(notdir $(subst .cpp,.d,$(SOURCES)))

#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
#BBBk
INCLUDE_DIRS = -I. -I../src/ -I../../library -I../../BioloidEX_Usb2Ax 
else
INCLUDE_DIRS = -I. -I../src/ -I../../library -I../../BioloidEX_Usb2Ax -I/usr/include/espeak
endif

#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
LIBRARY_DIRS = -L/usr/lib/arm-angstrom-linux-gnueabi/4.7.3 -L../../library -L../BioloidEX_Usb2Ax -L../library
else
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf -L../../library -L../../BioloidEX_Usb2Ax -L../library
endif

#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -Wall -pedantic -g -O2 -fno-rtti -Wno-write-strings

#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~
ifeq ($(OSTYPE),linux-gnueabi)
LDFLAGS = $(LIBRARY_DIRS) -lpthread -lBioloidEX -lArduinoPort -lasound -lespeak -lrt
else
LDFLAGS = $(LIBRARY_DIRS) -lpthread -lBioloidEX -lArduinoPort -lespeak -lasound -lrt -lbluetooth
endif

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
all: LIBRARY BioloidEX_Usb2Ax begin gccversion build end

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
BioloidEX_Usb2Ax:
	cd ../../BioloidEX_Usb2Ax; $(MAKE)

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
LIBRARY:
	cd ../../library; $(MAKE)

#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build: $(MAIN_OUT)

%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c $<


# added specific dependence on my own library...
$(MAIN_OUT): $(MAIN_OBJS) ../../library/libArduinoPort.a
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
	-rm -f $(MAIN_OBJS)
	-rm -f $(MAIN_OUT)
	-rm -f $(MAIN_DEPS)
	-cd ../../library ; make clean

#~~~~~~~~~~~~~~~~~~~~ backup ~~~~~~~~~~~~~~~~~~~~
backup: clean
	tar cJvf ../$(MAIN_OUT)_`date +"%Y-%m-%d_%H%M"`.tar.xz *

#~~~~~~~~~~~~~~~~~~~~ Dependency Generation
include $(notdir $(subst .cpp,.d,$(SOURCES)))

%.d: %.cpp
	$(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$

%.d: ../src/%.cpp
	$(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
