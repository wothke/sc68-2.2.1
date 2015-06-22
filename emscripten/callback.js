/*
* Declaration of the JavaScript functions that will be 
* invoked from the EMSCRIPTEN generated code.
* 
* In EMSCRIPTEN terminology it is called a *.js library (which must
* be in this somewhat peculiar format.. see below)
*
* 2014 by Juergen Wothke
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.le 
*/
mergeInto(LibraryManager.library, {
	callback_check_replay: function(name) {
		window['fileRequestCallback'](name);		// use hook provided by scriptprocessor_player.js
	}
});