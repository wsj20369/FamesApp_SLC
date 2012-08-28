@echo off

REM  ±àÒëslc
CD   src
CALL MAKEFILE > ..\t.txt
CD   ..

REM  Á¬½Ó
make makefile

ECHO ALL OK. 

notepad t.txt

