
CC = SDK:gcc/bin/gcc
LD = SDK:gcc/bin/gcc

SRC_DIR = source/
OBJ = $(SRC_DIR)main.o $(SRC_DIR)resources.o $(SRC_DIR)gui.o $(SRC_DIR)gamepad.o

BIN = vbaGUI


OS := $(shell uname)

ifeq ($(strip $(OS)),AmigaOS)
	AMIGADATE = $(shell c:date LFORMAT %d.%m.%Y)
	#YEAR = $(shell c:date LFORMAT %Y)
else
	AMIGADATE = $(shell date +"%-d.%m.%Y")
	#YEAR = $(shell date +"%Y")
endif

DEBUG = -DDEBUG


INCPATH = 

CFLAGS = $(DEBUG) $(INCPATH) -Wall -D__AMIGADATE__=\"$(AMIGADATE)\" -gstabs

LDFLAGS = 

LIBS = 
#	add any extra linker libraries you want here

.PHONY: all all-before all-after clean clean-custom realclean

all: all-before $(BIN) all-after

all-before: $(SRC_DIR)vbagui_strings.h
#	You can add rules here to execute before the project is built


all-after:
#	You can add rules here to execute after the project is built

clean: clean-custom
	rm -v $(OBJ)

realclean:
	rm -v $(OBJ) $(BIN) $(BIN).debug

$(BIN): $(OBJ) $(LIBS)
#	You may need to move the LDFLAGS variable in this rule depending on its contents
	@echo "Linking $(BIN)"
	@$(LD) -o $(BIN).debug $(OBJ) $(LDFLAGS) $(LIBS)
#	strip $(BIN).debug -o $(BIN)
	copy $(BIN).debug $(BIN) FORCE

###################################################################
##  Standard rules
###################################################################

# A default rule to make all the objects listed below
# because we are hiding compiler commands from the output

$(SRC_DIR)vbagui_strings.h: catalogs/vbagui.cd
	APPDIR:CatComp catalogs/vbagui.cd CFILE $(SRC_DIR)vbagui_strings.h


.c.o:
	@echo "Compiling $<"
	@$(CC) -c $< -o $*.o $(CFLAGS)


$(SRC_DIR)main.o: $(SRC_DIR)main.c $(SRC_DIR)includes.h $(SRC_DIR)vbagui_strings.h

$(SRC_DIR)resources.o: $(SRC_DIR)resources.c $(SRC_DIR)includes.h $(SRC_DIR)vbagui_strings.h

$(SRC_DIR)gui.o: $(SRC_DIR)gui.c $(SRC_DIR)includes.h $(SRC_DIR)vbagui_strings.h

$(SRC_DIR)gamepad.o: $(SRC_DIR)gamepad.c $(SRC_DIR)includes.h $(SRC_DIR)vbagui_strings.h


###################################################################
