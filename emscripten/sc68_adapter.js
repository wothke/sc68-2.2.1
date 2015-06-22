/*
 sc68_adapter.js: Adapts SC68 backend to generic WebAudio/ScriptProcessor player.
 
 version 1.0
 
 	Copyright (C) 2015 Juergen Wothke

 LICENSE
 
 This library is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or (at
 your option) any later version. This library is distributed in the hope
 that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/
SC68BackendAdapter = (function(){ var $this = function () { 
		$this.base.call(this, backend_SC68.Module, 2);
		this._initialized= false;
		this._currentTrack;
		this._replayCache= new Array();
	}; 
	// SC68's sample buffer contains 2-byte integer sample data (i.e. must be rescaled) 
	// of 2 interleaved channels
	extend(EmsHEAP16BackendAdapter, $this, {  
		getAudioBuffer: function() {
			var ptr=  this.Module.ccall('emu_get_audio_buffer', 'number');			
			// make it a this.Module.HEAP16 pointer
			return ptr >> 1;	// 2 x 16 bit samples			
		},
		getAudioBufferLength: function() {
			var len= this.Module.ccall('emu_get_audio_buffer_length', 'number');
			return len;
		},
		computeAudioSamples: function() {		
			var status = this.Module.ccall('emu_compute_audio_samples', 'number');
			
			var isError= this.Module.ccall('emu_is_error', 'number', ['number'], [status]);
			if (isError) {
				return -1;
			} else {
				var isWaiting= this.Module.ccall('emu_is_waiting', 'number', ['number'], [status]);
												
				if (isWaiting) {
					// eventually the "replay" will be loaded and normal 
					// processing will resume					
					ScriptNodePlayer.getInstance().setWait(true);
					return -1;
				} else {
					//this.Module.ccall('emu_is_track_change', 'number', ['number'], [status]) 
					if (this.Module.ccall('emu_is_loop', 'number', ['number'], [status]) || 
						this.Module.ccall('emu_is_end', 'number', ['number'], [status])) {
						return 1;	// do not distinguish the above cases.. just end current song
					}
					return 0;
				}
			}
		},
		getMaxPlaybackPosition: function() { 
			return this.Module.ccall('emu_get_max_position', 'number');
		},
		getPlaybackPosition: function() {
			return this.Module.ccall('emu_get_current_position', 'number');
		},
		seekPlaybackPosition: function(pos) {
			return this.Module.ccall('emu_seek_position', 'number', ['number'], [pos]);
		},
		getPathAndFilename: function(filename) {
			return ['/', filename];
		},
		registerFileData: function(pathFilenameArray, data) {
			return 0;
		},
		loadMusicData: function(sampleRate, path, filename, data) {
			// load the song's binary data
			var buf = this.Module._malloc(data.length);
			this.Module.HEAPU8.set(data, buf);
				
			var ret = this.Module.ccall('emu_init', 'number', 
							['number', 'number', 'number', 'number', 'number'], 
							[this._initialized, sampleRate, this.songTimeout, buf, data.length]);

			this._initialized= true;
			this.Module._free(buf);

			if (ret == 0) {
				var inputSampleRate = this.Module.ccall('emu_get_sample_rate', 'number');
				this.resetSampleRate(sampleRate, inputSampleRate); 
			}
						
			return ret;			
		},
		evalTrackOptions: function(options) {
			if (typeof options.timeout != 'undefined') {
				ScriptNodePlayer.getInstance().setPlaybackTimeout(options.timeout*1000);
			}
			var track = options.track ?  options.track : 0;	// frontend counts from 0
			this._currentTrack= track +1;					// sc68 starts counting at 1
			
			// for sc68 "0" means "all".. 		
			var ret= this.Module.ccall('emu_change_subsong', 'number', ['number'], [this._currentTrack]);
			
			// it seems that the above doesn't work and that manual seeking has to be used instead..		
			if (this._currentTrack > 1) {
				var o= new Object();
				var seek=0;
				var i;
				for (i= 1; i<this._currentTrack; i++) {
					seek+=this.getSongLength(i);
				}			
				// hack; seeking doesnt seem to work before emu_compute_audio_samples is called
				this.Module.ccall('emu_compute_audio_samples', 'number');	
				this.seekPlaybackPosition(seek);
			}			
			
			return ret;
		},				
		teardown: function() {
		},
		getSongInfoMeta: function() {
			return {title: String,
					author: String,
					composer: String,
					replay: String,
					hwname: String,
					songInMillis: Number,
					numberOfTracks: Number
					};
		},
		getSongLength: function(track) {
			var numAttr= 7;
			ret = this.Module.ccall('emu_get_track_info', 'number', ['number'], [track]);			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+numAttr);
			return parseInt(this.Module.Pointer_stringify(array[5]));
		},
		updateSongInfo: function(filename, result) {
			var numAttr= 7;
			ret = this.Module.ccall('emu_get_track_info', 'number', ['number'], [this._currentTrack]);
			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+numAttr);
			result.title= this.Module.Pointer_stringify(array[0]);
			result.author= this.Module.Pointer_stringify(array[1]);
			result.composer= this.Module.Pointer_stringify(array[2]);
			result.replay= this.Module.Pointer_stringify(array[3]);
			result.hwname= this.Module.Pointer_stringify(array[4]);
			result.songInMillis= parseInt(this.Module.Pointer_stringify(array[5]));
			result.numberOfTracks= parseInt(this.Module.Pointer_stringify(array[6]));
		},
		
		// --------------------------- async file loading stuff -------------------------

		mapBackendFilename: function (name) {
			var input= this.Module.Pointer_stringify(name);
			
			if (input && (input in this._replayCache)) {
				this.setCachedReplay(input);
			}
			
			return "replay/"+input+".bin";	// only sc68 replays are loaded here
		},
		cacheReplay: function(name, data) {
			ScriptNodePlayer.getInstance().trace("cache replay: ["+name+"] length: "+data.length);
			this._replayCache[name]= data;
		},
		registerFileData: function(pathFilenameArray, byteArray) {
			ScriptNodePlayer.getInstance().trace("loaded: ["+pathFilenameArray[1]+"] length: "+byteArray.length);
			
			var name= pathFilenameArray[1];
			var replay= "replay/";

			if (name.substring(0, replay.length) == replay) {
				// only 'replay' filed need to be handled here
				name= name.substring(replay.length);
				name= name.substring(0, name.length-4);	// also crop ".bin" (that we added above)
				
				this.cacheReplay(name, byteArray);				
			}			
			return 1;	// anything but undefined
		},	
		setCachedReplay: function(name) {
			if (name) {
				var replay= this._replayCache[name];
			
				if (replay) {
					ScriptNodePlayer.getInstance().trace(
						"set cached replay: ["+name+"] length: "+replay.length);
					
					var bytes = new Uint8Array(name.length+1);	// we dont have any unicode here
					var i;
					for (i = 0; i < name.length; i++){ 
						var c= name.charCodeAt(i);
						bytes[i]= c & 0xff;
					}
					bytes[i]= 0;
					var keybuf = this.Module._malloc(bytes.length);
					this.Module.HEAPU8.set(bytes, keybuf);
					var buf = this.Module._malloc(replay.length);
					this.Module.HEAPU8.set(replay, buf);
					var ret = this.Module.ccall('emu_set_binary_data', 'number', ['number', 'number', 'number', 'number'], [keybuf, bytes.length, buf, replay.length]);
					this.Module._free(keybuf);
					this.Module._free(buf);				
				}
			}
		}

	});	return $this; })();
	