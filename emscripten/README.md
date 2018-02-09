# webAudio68

Copyright (C) 2014 Juergen Wothke

This project is based on sc68 version 2.2.1: Everything that's not needed for the command line tool 
"sc68" has been thrown out. The structure of the original sc68 project has been preserved. (This might 
ease the task to eventually migrate the project to some future sc68 version. All changes have been done 
using #ifdefs, i.e you can also search for "EMSCRIPTEN" to find the changes.) 

All resource/file-access has been disabled/replaced. The sources 
have then been amended to allow JavaScript translation using emscripten (a new file adapter.c
contains the APIs used to interface with the JavaScript player).

The kind support of Benjamin Gerard enabled me to also include two features from the most recent 
sc68 development branch:

1) added "replays" originally not available in version 2.2.1
2) new file loading/detection logic for improved SNDH file handling

## How to build
You'll need Emscripten (http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). The make script 
is designed for use of emscripten version 1.37.29 (unless you want to create WebAssembly output, older versions might 
also still work).

The below instructions assume that the sc68-2.2.1 project folder has been moved into the main emscripten 
installation folder (maybe not necessary) and that a command prompt has been opened within the 
project's "emscripten" sub-folder, and that the Emscripten environment vars have been previously 
set (run emsdk_env.bat).

The project can be built in 2 flavours:

1) Running the makeEmscriptenInline.bat in this folder will generate a standalone JavaScript version of the 
Emscripten generated part of 'webaudio 68': backend_sc68.js (i.e. the "replay" binaries are all inlined - which makes 
the file somewhat large). Ìn addition to the larger size this version has the disadvantage that it contains 
only the fixed set of "replays" available in the original sc68-2.2.1 distribution. I'd therefore recommend 
to always use the below flavour instead:

2) Running the makeEmscripten.bat in the project folder will generate a somewhat leaner library - which will 
load "replay" binary files on demand. At runtime this version depends on a "replay" folder containing the respective 
binaries (see "htdocs" sub-folder for example). This allows to add new "replays" without recompiling the library 
(you must make sure though that the "replays" have been compiled for an absolute load-address of $8000 - different
versions of sc68 are using different defaults here..)

Both build scripts will create a backend_sc68.js library directly within the htdos/ example folder. (The content 
of the respective folder can be deployed in some arbitrary folder under the document root of your web server 
for testing.)

By changing the "WASM" switch in the make-scripts, WebAssembly output can be generated: In this case an additional *.wasm
output will be generated. The resulting output will be smaller and run faster but not all browsers do currently support
this new technology.

## Dependencies
The current version requires version 1.02 (older versions will not
support WebAssembly) of my https://github.com/wothke/webaudio-player.

## License
This program (i.e web extensions of sc68) is free software: you can 
redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
