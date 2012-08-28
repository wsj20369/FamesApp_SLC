echo. > ltall.txt

cd os
call ltall.bat
cd ..

lint  options.lnt -Isrc src\*.c    >> ltall.txt

notepad ltall.txt

