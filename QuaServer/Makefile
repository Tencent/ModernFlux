include ../version_para

INC += -I./ \
	-I../Protocol/ \
	-I../Lib/tinyxml \
	-I../Lib/spp/module/include/spp_incl/ \
	-I../Lib/spp/module/include/ \
	-I../Lib/basiclib/inc/ \
	-I../Lib/l5/ \
	-I$(PROTOINC) \
	
LIB += -lrt \
	../Lib/tinyxml/libtinyxml.a \
	../Lib/l5/libqos_client.a \
	../Protocol/lib/libproto.a \
	$(PROTOLIB) \
	
OBJ_DIR=./obj
LIB_DIR=./lib

LIB_NAME=aquota.so

TARGET = $(LIB_DIR)/$(LIB_NAME)

OBJ = $(OBJ_DIR)/Frame.o \
	$(OBJ_DIR)/FlowMsg.o \
	$(OBJ_DIR)/ServiceQua.o \
	$(OBJ_DIR)/lib_md5.o \
	$(OBJ_DIR)/CpuInfo.o \
	$(OBJ_DIR)/StringProcess.o \
	$(OBJ_DIR)/SppApi.o \
	$(OBJ_DIR)/UDPAPI.o \
	$(OBJ_DIR)/TimeAPI.o \
	
CFLAGS  += -pg
CXXFLAGS+=$(CFLAGS)

# *.cpp -> obj/*.o
$(OBJ_DIR)/%$(OBJ_EXT): %$(CXXSRC_EXT)
		@echo
		@echo "Compiling $< ==> $@..."
		@-mkdir -p $(OBJ_DIR)
		$(CXX) $(CXXFLAGS) -c $< $(INC) -o $@

PRE_DIR= \
		../Lib/jsoncpp \
		../Lib/tinyxml \

all:PREMAKE $(TARGET)

PREMAKE:
		@echo "makeing in" $(PRE_DIR)
		$(foreach dir,$(PRE_DIR),$(MAKE) -C $(dir) || exit $$?;)
		
$(TARGET) : $(OBJ)
		@echo
		@echo "Building $^ ==> $@..."
		@-mkdir -p $(LIB_DIR)
		$(CXX) $(CXXFLAGS) -shared $^ $(LIB) -o $@
clean:
		@echo
		@echo "Cleaning..."
		rm -rf $(OBJ_DIR)
		rm -f $(TARGET)
		
cleanmore:clean
		@echo "Cleaning in "$(PRE_DIR)
		$(foreach dir,$(PRE_DIR),make clean -C $(dir);)
		
clear:
		@echo
		@echo "Cleaning..."
		rm -rf $(OBJ_DIR)
		rm -f $(TARGET)
		rm -rf *~
		rm -rf *.pb.*	
		rm -rf *.o
all32:
		make -e ARCH=32
		
		
