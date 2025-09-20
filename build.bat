cl netcli.c ^
lib/* ^
contexts/wifi/*.c ^
contexts/network/*.c ^
contexts/radio/*.c ^
contexts/skeleton/*.c ^
contexts/general/*.c ^
-I . /Zc:preprocessor

rm *.obj