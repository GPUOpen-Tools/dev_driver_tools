: move the dll's and lib files to the API folder
copy %1\* ..\..\..\%1\RGP_API\lib
move ..\..\..\%1\RGP_API.* ..\..\..\%1\RGP_API\bin
