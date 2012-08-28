COMPILER    = tcc -c -1 -B -S -ml -w -Ic:\tc\include -I..\core -I\FamesOS\src\os\include
LINKER      = tlink /3
HEADFILE    = ..\core\core.h ..\core\type.h \FamesOS\src\os\include\checkinc

EXE_FILE    = test.exe

ALL_OBJ     = test.obj+c:\tc\lib\c0l.obj
STD_LIB     = c:\tc\lib\cxc.lib
ALL_LIB     = ..\core\slc_core.lib+\FamesOS\src\os\os.lib
ALL_LIST    = ..\core\slc_core.lib  test.obj

test.obj: test.c $(HEADFILE)
	$(COMPILER) test.c

$(EXE_FILE): $(ALL_LIST)
	$(LINKER) $(ALL_OBJ),$(EXE_FILE),,$(STD_LIB)+$(ALL_LIB);



