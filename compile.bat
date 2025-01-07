REM Automatically generated from Makefile
mkdir obj
..\..\..\bin\lcc   -c -o obj\main.o src\main.c
..\..\..\bin\lcc   -c -o obj\scheduler.o src\scheduler.c
..\..\..\bin\lcc   -o obj\Example.gb obj\main.o obj\scheduler.o
