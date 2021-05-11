# valsuite2020
low level window and control management system for scripting language. c++
Relies on "Standalone Headers" github repo @https://github.com/deltaInfiniti/StandaloneHeaders.
Main files actually reside on that repo because the bulk of this project are headers I use in other projects.

This repo basically only contains the main.cpp file for the tutorial/example projects and an implimentation of the live window editor.

The main files (in dependancy stack):
- ValWindowManager.h 
-> ValDataRegister.h
-> ValWindow.h
--> ValTimer.h
--> ValGFX.h
--> ValEventHandler.h
--> ValControl.h (ValGFX.h)
---> strlist.h & varlist2020.h
---> valtextcontrol.h
( some helper functions from valutils.h are also utilized) 

example files(what it says on the tin):
- ValBreakout.h 

- mandelbrot.h

also included in the standalone headers repo, but specific to valsuite:
- ValWindowEditor.h





