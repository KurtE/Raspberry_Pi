#~~~~~~~~~~~~~~~~~~~~ Output File Name ~~~~~~~~~~~~~~~~~~~~
MAIN_OUT = graphictest

CLS_OUT  = cls

TOUCH_OUT = touchpaint

READPIXEL_OUT = readpixel

#~~~~~~~~~~~~~~~~~~~~ Source Files ~~~~~~~~~~~~~~~~~~~~
SOURCES = \
		graphictest.cpp
        
SOURCES_CLS = \
		cls.cpp        
    
SOURCES_TOUCH = touchpaint.cpp

SOURCES_READPIXEL = readpixel.cpp

MAIN_OBJS:= $(subst .cpp,.o,$(SOURCES))

MAIN_DEPS:= $(subst .cpp,.d,$(SOURCES))

CLS_OBJS:= $(subst .cpp,.o,$(SOURCES_CLS))

CLS_DEPS:= $(subst .cpp,.d,$(SOURCES_CLS))

TOUCH_OBJS:= $(subst .cpp,.o,$(SOURCES_TOUCH))

TOUCH_DEPS:= $(subst .cpp,.d,$(SOURCES_TOUCH))

READPIXEL_OBJS:= $(subst .cpp,.o,$(SOURCES_READPIXEL))

READPIXEL_DEPS:= $(subst .cpp,.d,$(SOURCES_READPIXEL))

#~~~~~~~~~~~~~~~~~~~~ Include Directories ~~~~~~~~~~~~~~~~~~~~
INCLUDE_DIRS = -I. -I../library -I../Adafruit_ILI9341C

#~~~~~~~~~~~~~~~~~~~~ Library Directories ~~~~~~~~~~~~~~~~~~~~
LIBRARY_DIRS = -L/usr/lib/arm-linux-gnueabihf -L../library -L../Adafruit_ILI9341C

#~~~~~~~~~~~~~~~~~~~~ Compiler Options ~~~~~~~~~~~~~~~~~~~~
COMPILE_OPTS = -pedantic -g -O2 -fno-rtti

#~~~~~~~~~~~~~~~~~~~~ Linker Options ~~~~~~~~~~~~~~~~~~~~
LDFLAGS = $(LIBRARY_DIRS) -lpthread -lAdafruit_ILI9341C -lArduinoPort  -lmraa

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
all: LIBRARY Adafruit_IlI9341C begin gccversion build build_cls build_touch build_readpixel end

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
Adafruit_IlI9341C:
	cd ../Adafruit_ILI9341C; $(MAKE)

#~~~~~~~~~~~~~~~~~~~~ library ~~~~~~~~~~~~~~~~~~~~
LIBRARY:
	cd ../library; $(MAKE)


#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build: $(MAIN_OUT)

$(MAIN_OUT): $(MAIN_OBJS) ../library/libArduinoPort.a ../Adafruit_ILI9341C/libAdafruit_ILI9341C.a
	$(CXX) $(CXXFLAGS) $(MAIN_OBJS) -o $(MAIN_OUT) $(LDFLAGS)


#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build_cls: $(CLS_OUT)

$(CLS_OUT): $(CLS_OBJS) ../library/libArduinoPort.a ../Adafruit_ILI9341C/libAdafruit_ILI9341C.a
	$(CXX) $(CXXFLAGS) $(CLS_OBJS) -o $(CLS_OUT) $(LDFLAGS)

#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build_touch: $(TOUCH_OUT)

$(TOUCH_OUT): $(TOUCH_OBJS) ../library/libArduinoPort.a ../Adafruit_ILI9341C/libAdafruit_ILI9341C.a
	$(CXX) $(CXXFLAGS) $(TOUCH_OBJS) -o $(TOUCH_OUT) $(LDFLAGS)

#~~~~~~~~~~~~~~~~~~~~ build ~~~~~~~~~~~~~~~~~~~~
build_readpixel: $(READPIXEL_OUT)

$(READPIXEL_OUT): $(READPIXEL_OBJS) ../library/libArduinoPort.a ../Adafruit_ILI9341C/libAdafruit_ILI9341C.a
	$(CXX) $(CXXFLAGS) $(READPIXEL_OBJS) -o $(READPIXEL_OUT) $(LDFLAGS)

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
	-rm $(MAIN_OBJS) $(CLS_OBJS) $(TOUCH_OBJS) $(READPIXEL_OBJS)
	-rm $(MAIN_OUT) $(CLS_OUT) $(TOUCH_OUT) $(READPIXEL_OUT)
	-rm $(MAIN_DEPS) $(CLS_DEPS) $(TOUCH_DEPS) $(READPIXEL_DEPS)
	-cd ../library ; make clean
	-cd ../Adafruit_ILI9341C ; make clean

#~~~~~~~~~~~~~~~~~~~~ backup ~~~~~~~~~~~~~~~~~~~~
backup: clean
	tar cJvf ../$(MAIN_OUT)_`date +"%Y-%m-%d_%H%M"`.tar.xz *

#~~~~~~~~~~~~~~~~~~~~ Dependency Generation
include $(subst .cpp,.d,$(SOURCES))

%.d: %.cpp
	$(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$
