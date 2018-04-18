: move the dll's and lib files to the API folder
copy %1\* ..\..\..\%1\DevDriverAPI\lib
copy ..\..\..\..\..\source\DevDriverAPI\DevDriverAPI.h ..\..\..\%1\DevDriverAPI\include
move ..\..\..\%1\DevDriverAPI.* ..\..\..\%1\DevDriverAPI\bin
