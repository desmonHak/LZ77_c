CC 			  = gcc
ARR			  = ar

VESRION_C     = 11

PATH_SRC 	      = src
PATH_INCLUDE      = include
PATH_EXAMPLES	  = example

INCLUDE_FLAGS = -I. -I$(PATH_INCLUDE)
GLOBAL_CFLAGS = -std=c$(VESRION_C) $(INCLUDE_FLAGS) -masm=intel \
				-D_ExceptionHandler -fdiagnostics-color=always $(DEBUG_LINUX)

CFLAGS 		  =  $(GLOBAL_CFLAGS) -O3 -Wno-unused-parameter \
				-Wno-implicit-fallthrough -Wno-type-limits  \
				-Wno-unused-variable -Wno-pointer-sign
ARR_FLAGS     = -rc

OBJECTS = 	lz77.o