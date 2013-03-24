Prog1:= THex3_Commander_SSC32
Prog2:= THex3_DIY_SSC32

Programs := $(Prog1) $(Prog2)


.PHONY: all $(Programs) 

all: $(Programs)


THex3_Commander_SSC32:
	$(MAKE) -f  makefile_THSC $(MAKECMDGOALS)

THex3_DIY_SSC32:
	$(MAKE) -f  makefile_THSX $(MAKECMDGOALS)

clean:
	$(MAKE) -f  makefile_THSC clean
	$(MAKE) -f  makefile_THSX clean

