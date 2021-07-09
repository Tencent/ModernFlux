# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
TINYXML_USE_STL := YES

#****************************************************************************

TINYXML = .
ARCH=`getconf LONG_BIT`

CFLAGS +=  -DSUS_LINUX -pthread

CFLAGS += -O2 -fno-inline -g -ggdb -Wall -D_GNU_SOURCE -D_MP_MODE -Wno-write-strings #-Werror 
CFLAGS += -pipe -fno-ident -fPIC -shared -z defs
#ifeq (YES, ${TINYXML_USE_STL})
#	CFLAGS += -DTIXML_USE_STL
#endif

INC += -I./ -I$(TINYXML)
LIB += -lpthread -ldl
TARGET=libtinyxml.a
all : $(TARGET)
OBJ = $(TINYXML)/tinystr.o \
	  $(TINYXML)/tinyxml.o \
	  $(TINYXML)/tinyxmlerror.o \
	  $(TINYXML)/tinyxmlparser.o

RED = \\e[1m\\e[31m
DARKRED = \\e[31m
GREEN = \\e[1m\\e[32m
DARKGREEN = \\e[32m
BLUE = \\e[1m\\e[34m
DARKBLUE = \\e[34m
YELLOW = \\e[1m\\e[33m
DARKYELLOW = \\e[33m
MAGENTA = \\e[1m\\e[35m
DARKMAGENTA = \\e[35m
CYAN = \\e[1m\\e[36m
DARKCYAN = \\e[36m
RESET = \\e[m
CRESET =  ;echo -ne \\e[m; test -s $@

$(TARGET):$(OBJ)
	@echo -e  Linking $(CYAN)$@$(RESET) ...$(RED) 
	ar crs $@ $^ $(CRESET)
	chmod +x $@

%.o: %.cpp
	@echo -e Compiling $(GREEN)$<$(RESET) ...$(RED)
	$(CXX) $(CFLAGS) -c -o $@ $< $(INC) $(CRESET)   
%.o: %.c
	@echo -e Compiling $(GREEN)$<$(RESET) ...$(RED)
	$(CC) $(CFLAGS) -c -o $@ $< $(INC) $(CRESET)    
clean:
	rm -f $(OBJ) $(TARGET)

