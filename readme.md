```
--------------------------------------------------------------------------------
Name....: DLLPackager v1.0.1 - source code
Author..: ReWolf
Date....: XI.2006
Rel.Date: VI.2007


e.mail..: rewolf@poczta.onet.pl
www.....: http://blog.rewolf.pl
--------------------------------------------------------------------------------
```
Simple tool to bundle DLL's with executable (similar to PEBundle or MoleBox). 'Simple' means there is no encryption or compression. It's coded fully in C++ (also loader).

#### **Principle of operation:** ####
![http://rewolf.pl/gfx/custom/exe_before.png](http://rewolf.pl/gfx/custom/exe_before.png)
![http://rewolf.pl/gfx/custom/exe_after.png](http://rewolf.pl/gfx/custom/exe_after.png)

```
--------------------------------------------------------------------------------
Files:

gpl.txt                    - GNU GPL license text
bin\DLLPackager.exe        - compiled executable
doc\exe.after.png          - diagram - represents executable after packagging
doc\exe.before.png         - diagram - represents executable before packagging
doc\compile.txt            - instructions for compiler setup
doc\loader.txt             - comments for src\loader.cpp
doc\main.txt               - comments for src\main.cpp
src\loader.cpp             - loader source code
src\main.cpp               - main app source code
src\defines.h              - macros definitios
src\loader.h               - header file for loader.cpp
src\resource.h             - resources header
src\res.rc                 - resources file

--------------------------------------------------------------------------------
```
#### **Related links:** ####
  * http://blog.rewolf.pl/blog/?p=11
  * http://blog.rewolf.pl/blog/?p=18
