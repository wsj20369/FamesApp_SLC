@echo off

REM  ����slc
CD   src
CALL MAKEFILE > ..\t.txt
CD   ..

REM  ����
make makefile

ECHO ALL OK. 

notepad t.txt

