cl netcli.c ^
lib/arg.c lib/context.c lib/bstr.c lib/error.c lib/context_commands.c lib/log.c ^
contexts/wifi/*.c contexts/network/*.c contexts/radio/*.c contexts/skeleton/*.c ^
-I . /Zc:preprocessor

rm *.obj