@echo off

REM  ����slc
CD   src
CALL MAKEFILE
CD   ..

REM  ����
make makefile


ECHO ALL OK. 
