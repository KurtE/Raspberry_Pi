lib:= LIBRARY BioloidEX_Usb2Ax

Prog1:= Phantom_Phoenix

Programs := $(lib) $(Prog1)


.PHONY: all $(Programs) 

all: $(Programs)


LIBRARY:
	cd ../library; $(MAKE)

BioloidEX_Usb2Ax:
	cd ../BioloidEX_Usb2Ax; $(MAKE)

Phantom_Phoenix:
	$(MAKE) -f  makefile_PhantomX $(MAKECMDGOALS)


clean:
	$(MAKE) -f  makefile_PhantomX clean
	cd ../library ; $(MAKE) clean
	cd ../BioloidEX_Usb2Ax ;  $(MAKE) clean
    