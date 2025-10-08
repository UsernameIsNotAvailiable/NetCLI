# Makefile for MSVC recursive build (clean output + tidy obj folder)

CC = cl
CFLAGS = /I . /Zc:preprocessor /nologo
TARGET = netcli.exe
OBJDIR = build/obj

all: $(TARGET)

$(TARGET):
	@echo Preparing build directories...
	@if not exist "$(OBJDIR)" mkdir "$(OBJDIR)" >nul
	@echo Discovering source files...
	@del sources.tmp 2>nul
	@for /R . %%f in (*.c) do @echo "%%~f" >> sources.tmp
	@echo Compiling...
	@for /F "usebackq delims=" %%f in (`type sources.tmp`) do @$(CC) $(CFLAGS) /Fo"$(OBJDIR)\%%~nf.obj" -c %%f
	@echo Linking...
	@$(CC) $(CFLAGS) $(OBJDIR)\*.obj /Fe$(TARGET)
	@del sources.tmp
	@echo Build complete: $(TARGET)

clean:
	@echo Cleaning build artifacts...
	@if exist "$(OBJDIR)" rmdir /S /Q "$(OBJDIR)"
	@del /Q $(TARGET) 2>nul
	@echo Done.

.PHONY: all clean
