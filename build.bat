
@set INCLUDES=/I ..\imgui /I ..\imgui\backends /I "%DXSDK_DIR%/Include"
@set LIBS=/LIBPATH:"%DXSDK_DIR%/Lib/x86" d3d9.lib
@set SOURCES=..\main.cpp ..\gui.cpp ..\globals.cpp ..\imgui\backends\imgui_impl_dx9.cpp ..\imgui\backends\imgui_impl_win32.cpp ..\imgui\imgui*.cpp ..\imgui\imnodes.cpp

@if exist out (
  @rmdir /S /Q out
)

@mkdir out

@cd out 
@cl /ZI /DEBUG /source-charset:windows-1251 /std:c++20 /EHsc %INCLUDES% /LD %SOURCES% /link /machine:X86 %LIBS% /out:kimo.dll
@cd ..
