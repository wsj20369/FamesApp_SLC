COMPILER    = tcc -c -1 -B -S -ml -w -Ic:\tc\include
LINKER      = tlink /3
STD_OBJ     = c:\tc\lib\c0l.obj
STD_LIB     = c:\tc\lib\cxc.lib+..\lib\os.lib
ALL_LIB     = slc.lib
ALL_LIST    = slc.lib


EXE_FILE    = slc.exe


$(EXE_FILE): $(ALL_LIST)
	$(LINKER) $(STD_OBJ),$(EXE_FILE),,$(STD_LIB)+$(ALL_LIB);
	copy $(EXE_FILE) slc

