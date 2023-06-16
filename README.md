# gilde_editor
This is a memory editor for the game Europa 1400 The Guild.

## How to build
You need to have a 32 bit (x86) C++ compiler.

If you have the Command Line Tools for Visual Studio, you can run **x86 Native Tools Command Prompt**.

build.bat will build the out/kimo.dll with the editor inside

## How to run
You need to inject the .dll into the game. There are atleast 3 options:
1. Use a tool like LordPE (don't forget to create a backup)
2. Use Cheat Engine to inject dll at runtime
3. If you understand how import tables work you can implement your own solution ;)
