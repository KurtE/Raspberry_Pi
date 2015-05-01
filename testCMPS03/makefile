#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = testCMPS03

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
SOURCES = \
		testCMPS03.cpp
        
MAIN_OBJS:= $(subst .cpp,.o,$(SOURCES))

MAIN_DEPS:= $(subst .cpp,.d,$(SOURCES))


#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
INCLUDE_DIRS = -I. 

#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf 

#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -pedantic -g -O2 -fno-rtti

#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~
LDFLAGS = $(LIBRARY_DIRS) -lpthread  -lmraa

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

CXX = $(TCHAIN_PREFIX)g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS)

#~~~~~~~~~~~~~~~~~~~~ all ~~~~~~~~~~~~~~~~~~~~
all: begin gccversion build end


#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build: $(MAIN_OUT)

$(MAIN_OUT): $(MAIN_OBJS) 
	$(CXX) $(CXXFLAGS) $(MAIN_OBJS) -o $(MAIN_OUT) $(LDFLAGS)


#~~~~~~~~~~~~~~~~~~~~ Eye candy ~~~~~~~~~~~~~~~~~~~~
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
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
	$(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
