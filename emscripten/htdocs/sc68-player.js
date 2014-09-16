/*
* sc68-player.js
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

var fetchSamples= function(e) { 		
	// it seems that it is necessary to keep this explicit reference to the event-handler
	// in order to pervent the dumbshit Chrome GC from detroying it eventually
	
	var f= window.player['genSamples'].bind(window.player); // need to re-bind the instance.. after all this 
															// joke language has no real concept of OO	
	f(e);
};

Sc68Player = function(sampleRate, songTimeout, onLoad, onEnd, onTrackChange, onLoop, onError) {
	this.sampleRate= sampleRate;
	this.songTimeout= songTimeout;
	
	this.title;
	this.author;
	this.composer;
	this.replay;
	this.hwname;
	this.songInMillis;
	this.numberOfTracks;

	this.isPaused= false;
	this.isWaitingForFile= false;

	this.onLoad= onLoad;
	this.onEnd= onEnd;
	this.onTrackChange= onTrackChange;
	this.onLoop= onLoop;
	this.onError= onError;
	
	this.sourceBuffer;
	this.sourceBufferLen;

	this.initialized= false;

	this.numberOfSamplesRendered= 0;
	this.numberOfSamplesToRender= 0;
	this.sourceBufferIdx=0;
	
	this.binaryFileMap = {};	// cache for loaded binaries
	this.pendingMap = {};	

	this.SAMPLES_PER_BUFFER = 8192;	// allowed: buffer sizes: 256, 512, 1024, 2048, 4096, 8192, 16384
	
	window.replayLoaderCallback= this.replayLoaderCallback.bind(this);	// setup callback for "replay" retrieval

	window.player= this;
};

Sc68Player.prototype = {
	createScriptProcessor: function(audioCtx) {
		var scriptNode = audioCtx.createScriptProcessor(this.SAMPLES_PER_BUFFER, 0, 2);
		scriptNode.onaudioprocess = fetchSamples;
	//	scriptNode.onaudioprocess = player.generateSamples.bind(player);	// doesn't work with dumbshit Chrome GC
		return scriptNode;
	},
	setPauseMode: function (pauseOn) {
		this.isPaused= pauseOn;
	},
	playTmpFile: function (file) {
		var reader = new FileReader();
		reader.onload = function() {
			this.loadData(reader.result, 0);
		}.bind(this);
		reader.readAsArrayBuffer(file);
	},
	// callback invoked by emscripten code to fetch "replay" binary code
	replayLoaderCallback: function (name) {
		var replayName= Module.Pointer_stringify(name);
		if (this.binaryFileMap[replayName])	{
			this.setReplayData(replayName, this.binaryFileMap[replayName]);
			return 0;
		}
		// requested data not available.. we better load it for next time
		if (!(replayName in this.pendingMap)) {		// avoid duplicate loading
			this.pendingMap[replayName] = 1;

			var oReq = new XMLHttpRequest();
			oReq.open("GET", "replay/"+replayName+".bin", true);
			oReq.responseType = "arraybuffer";

			oReq.onload = function (oEvent) {
				var arrayBuffer = oReq.response;
				if (arrayBuffer) {
					this.binaryFileMap[replayName]= new Uint8Array(arrayBuffer);
					this.setReplayData(replayName, this.binaryFileMap[replayName]);
				}
				delete this.pendingMap[replayName]; 
			}.bind(this);
			oReq.send(null);	
		}
		return -1;	
	},
	setReplayData: function(name, byteArray) {
		var bytes = new Uint8Array(name.length+1);	// we dont have any unicode here
		var i;
		for (i = 0; i < name.length; i++){ 
			var c= name.charCodeAt(i);
			bytes[i]= c & 0xff;
		}
		bytes[i]= 0;
		var keybuf = Module._malloc(bytes.length);
		Module.HEAPU8.set(bytes, keybuf);
		
		var buf = Module._malloc(byteArray.length);
		Module.HEAPU8.set(byteArray, buf);
		var ret = Module.ccall('emu_set_binary_data', 'number', ['number', 'number', 'number', 'number'], [keybuf, bytes.length, buf, byteArray.length]);
		Module._free(keybuf);
		Module._free(buf);
				
		// now that the correct "replay" has been set we can resume playback
		this.isWaitingForFile= false;
	},
	loadData: function(arrayBuffer, track) {
		this.isPaused= true;

		if (arrayBuffer) {
			var byteArray = new Uint8Array(arrayBuffer);
			// load the song's binary data
			var buf = Module._malloc(byteArray.length);
			Module.HEAPU8.set(byteArray, buf);
				
			var ret = Module.ccall('emu_init', 'number', ['number', 'number', 'number', 'number', 'number'], 
									[this.initialized, this.sampleRate, this.songTimeout, buf, byteArray.length]);
			this.initialized= true;

			Module._free(buf);

			this.updateTrackInfos();

			// get location of sample buffer
			this.sourceBuffer= Module.ccall('emu_get_audio_buffer', 'number');
			this.sourceBufferLen= Module.ccall('emu_get_audio_buffer_length', 'number');

			this.playSong(track);
			this.isPaused= false;

			if (this.onLoad) this.onLoad();
		}
	},
	updateTrackInfos: function() {
		// get song infos
		var track= -1;
		ret = Module.ccall('emu_get_track_info', 'number', ['number'], [track]);
		
		var array = Module.HEAP32.subarray(ret>>2, (ret>>2)+7);
		this.title= Module.Pointer_stringify(array[0]);
		this.author= Module.Pointer_stringify(array[1]);
		this.composer= Module.Pointer_stringify(array[2]);
		this.replay= Module.Pointer_stringify(array[3]);
		this.hwname= Module.Pointer_stringify(array[4]);
		this.songInMillis= parseInt(Module.Pointer_stringify(array[5]));
		this.numberOfTracks= parseInt(Module.Pointer_stringify(array[6]));
	},
	playSong: function(id) {
	  Module.ccall('emu_change_subsong', 'number', ['number'], [id]);
	},
	genSamples: function(event) {
		var output1 = event.outputBuffer.getChannelData(0);
		var output2 = event.outputBuffer.getChannelData(1);

		if (this.isWaitingForFile || (this.isPaused)) {
			var i;
			for (i= 0; i<output1.length; i++) {
				output1[i]= 0;
				output2[i]= 0;
			}		
		} else {
			var outSize= output1.length;
			
			this.numberOfSamplesRendered = 0;		

			while (this.numberOfSamplesRendered < outSize)
			{
				if (this.numberOfSamplesToRender == 0) {
					var status = Module.ccall('emu_compute_audio_samples', 'number');
					
					var isError= Module.ccall('emu_is_error', 'number', ['number'], [status]);
					if (isError) {
						this.fillEmpty(outSize, output1, output2);	
						if (this.onError) this.onError();
						return;
					} else {
						var isWaiting= Module.ccall('emu_is_waiting', 'number', ['number'], [status]);
											
						if (isWaiting) {
							this.fillEmpty(outSize, output1, output2);	
							// eventually the "file" will be loaded and normal processing will resume
							this.isWaitingForFile= true;
							return;
						} else {
							if (Module.ccall('emu_is_track_change', 'number', ['number'], [status])) {
								this.updateTrackInfos();
								
								if (this.onTrackChange) this.onTrackChange();	
							} else if (Module.ccall('emu_is_loop', 'number', ['number'], [status])) {
								if (this.onLoop) this.onLoop();						
							
							} else if ( Module.ccall('emu_is_end', 'number', ['number'], [status])) {
								// lets presume this really means "end" - as in "stopped"
								this.fillEmpty(outSize, output1, output2);
								this.isPaused= true;						
								if (this.onEnd) this.onEnd();
							}
						}
					}
					
					this.numberOfSamplesToRender = Module.ccall('emu_get_audio_buffer_length', 'number');
					this.sourceBufferIdx=0;			
				}
				
				var srcBufI16= this.sourceBuffer>>1;	// 2 x 16 bit samples
				if (this.numberOfSamplesRendered + this.numberOfSamplesToRender > outSize) {
					var availableSpace = outSize-this.numberOfSamplesRendered;
					
					var i;
					for (i= 0; i<availableSpace; i++) {
						var r16= Module.HEAP16[srcBufI16 + (this.sourceBufferIdx++)];
						var l16= Module.HEAP16[srcBufI16 + (this.sourceBufferIdx++)];
						output1[i+this.numberOfSamplesRendered]= r16/0x7fff;
						output2[i+this.numberOfSamplesRendered]= l16/0x7fff;
					}				
					this.numberOfSamplesToRender -= availableSpace;
					this.numberOfSamplesRendered = outSize;
				} else {
					var i;
					for (i= 0; i<this.numberOfSamplesToRender; i++) {
						var r16= Module.HEAP16[srcBufI16 + (this.sourceBufferIdx++)];
						var l16= Module.HEAP16[srcBufI16 + (this.sourceBufferIdx++)];
						output1[i+this.numberOfSamplesRendered]= r16/0x7fff;
						output2[i+this.numberOfSamplesRendered]= l16/0x7fff;
					}						
					this.numberOfSamplesRendered += this.numberOfSamplesToRender;
					this.numberOfSamplesToRender = 0;
				} 
			}  
		}	
	},
	fillEmpty: function(outSize, output1, output2) {
		var availableSpace = outSize-this.numberOfSamplesRendered;
		for (i= 0; i<availableSpace; i++) {
			output1[i+this.numberOfSamplesRendered]= 0;
			output2[i+this.numberOfSamplesRendered]= 0;
		}				
		this.numberOfSamplesToRender = 0;
		this.numberOfSamplesRendered = outSize;			
	}
};