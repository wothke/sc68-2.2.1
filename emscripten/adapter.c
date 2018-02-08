/*
* This file adapts sc68 to the interface expected by my generic JavaScript player..
*
* Copyright (C) 2014 Juergen Wothke
*
* LICENSE
* 
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or (at
* your option) any later version. This library is distributed in the hope
* that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <config68.h>

#ifdef EMSCRIPTEN
#define EMSCRIPTEN_KEEPALIVE __attribute__((used))
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "api68/api68.h"
#include "io68/ymemul.h"


static api68_t * sc68 = 0;
static api68_init_t init68;

#ifndef ONDEMAND
void register_players();
#endif

extern int calc_current_ms(api68_t * api);

const static unsigned int BUF_SIZE= 256; 	// 512;
static int outBuffer4[BUF_SIZE];				// avoid potential alignment issues
static char *outBuffer= (char*)outBuffer4;

int emu_init(int reinit, int sampleRate, int maxPlaytime, char *buffer, int siz) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_init(int reinit, int sampleRate, int maxPlaytime, char *buffer, int siz) {
	for (int i=0; i<BUF_SIZE; i++) {
		outBuffer4[i]= 0;
	}
	
	if (reinit) {
		api68_stop(sc68);
		api68_shutdown(sc68);
#ifdef ONDEMAND
		api68_set_replay(0,0,0,0);
#endif
	} else {
#ifndef ONDEMAND
		register_players() ;
#endif
	}
	memset(&init68, 0, sizeof(init68));
	init68.alloc = malloc;
	init68.free = free;
	
	if (sampleRate < SAMPLING_RATE_MIN) sampleRate= SAMPLING_RATE_MIN;
	if (sampleRate > SAMPLING_RATE_MAX) sampleRate= SAMPLING_RATE_MAX;	
	
	init68.sampling_rate= sampleRate;	// override whatever might be configured
	
	sc68 = api68_init(&init68);
	if (!sc68) {
		return -1;
	}
	
	// reset whatever config defaults have been used..
	if (maxPlaytime >0) api68_override_max_playtime(maxPlaytime);


	// set song binary
	if (api68_verify_mem(buffer, siz) < 0) {
	  return -1;
	}

	if (api68_load_mem(sc68, buffer, siz)) {
	  return -1;
	}
	return 0;
}

int emu_get_sample_rate() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_get_sample_rate() {
	return init68.sampling_rate;
}

#ifdef ONDEMAND
int emu_set_binary_data(char *key, int keyLen, char *buffer, int siz) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_set_binary_data(char *key, int keyLen, char *buffer, int siz) {
	return api68_set_replay(key, keyLen, buffer, siz);
}
#endif

static char* infoTexts[7];
static char tracks_str[15];
static char time_str[15];

char** emu_get_track_info(int track) __attribute__((noinline));
char** EMSCRIPTEN_KEEPALIVE emu_get_track_info(int track) {
    api68_music_info_t info;
    if (!api68_music_info(sc68,&info,track,0)) {
        // more emscripten friendly structure..
        infoTexts[0]= (char*)info.title;
        infoTexts[1]= (char*)info.author;
        infoTexts[2]= (char*)info.composer;
        infoTexts[3]= (char*)info.replay;
        infoTexts[4]= (char*)info.hwname;

        sprintf(time_str, "%d", info.time_ms);
        infoTexts[5]= (char*)time_str;
		
        sprintf(tracks_str, "%d", info.tracks);
        infoTexts[6]= (char*)tracks_str;
    }
    return infoTexts;
}


int emu_change_subsong(int track) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_change_subsong(int track) {
	if (track == 0) {
		if (api68_play(sc68, 1)) {	// all
			return -1;
		}
	} else if (track > 0) {
		if (api68_play(sc68, track)) {
			return -1;
		}
	}
	
	emu_get_track_info(track);
	
	return 0;
}
int emu_get_audio_buffer_length() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length() {
	return BUF_SIZE;//  sizeof(outBuffer) >> 2;
}

char* emu_get_audio_buffer() __attribute__((noinline));
char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer() {
	return outBuffer;
}

int emu_compute_audio_samples() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_compute_audio_samples() {
	return api68_process(sc68, outBuffer, BUF_SIZE /*sizeof(outBuffer) >> 2*/);
}

int emu_is_error(int code) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_is_error(int code) {
    return code == API68_MIX_ERROR ? 1:0;
}

int emu_is_waiting(int code) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_is_waiting(int code) {
    return !emu_is_error(code) && (code & API68_WAIT_REPLAY) ? 1:0;
}

int emu_is_track_change(int code) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_is_track_change(int code) {
    return !emu_is_error(code) && (code & API68_CHANGE) ? 1:0;
}

int emu_is_end(int code) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_is_end(int code) {
    return !emu_is_error(code) && (code & API68_END) ? 1:0;
}

int emu_is_loop(int code) __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_is_loop(int code) {
    return !emu_is_error(code) && (code & API68_LOOP) ? 1:0;
}


int emu_get_current_position() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_get_current_position() {
	return calc_current_ms(sc68);		// position in overall file
}

void emu_seek_position(int pos) __attribute__((noinline));
void EMSCRIPTEN_KEEPALIVE emu_seek_position(int pos) {
	api68_seek(sc68, pos);	// pos in overall file (not the current track)
}

int emu_get_max_position() __attribute__((noinline));
int EMSCRIPTEN_KEEPALIVE emu_get_max_position() {
	int retVal= -1;
    api68_music_info_t info;
    if (!api68_music_info(sc68,&info,0,0)) {
			retVal= info.time_ms;
    }
    return retVal;
}

// ---------------------------------- just for Antoine -------------------
unsigned char getVolume(int voice) {
  // thx to Ben G Han for the below impl
  unsigned char vol;
  
  /* Get the mixer for all voices */
  u8 mixer = ym.data[7];
  /* read volume */
  vol = ym.data[8+voice] & 31;
  if (vol < 16) {
    /* Not envelop controlled: check if either
     * pulse or noise is enable else it's
     * silenced (unless it's a digit or
     * something like that but that's another
     * problem. */
    if ( ((mixer>>voice) & 9) == 9 )
      /* nothing enabled: */
      vol = 0;
  } else {
    /* controlled by envelop */
    unsigned int shape = ym.data[13] & 15;
    vol = 0;
    if ((1<<shape) & 0x5500) {
      /* One of the envelop used for buzz ? We
       * could also test the envelop period to
       * be sure it's fast enough to actually
       * generate a wavform but nevermind */
      /* Buzz are maxed out */
      vol = 15;
      if ( ! ( (mixer>>voice) & 1 ) ) {
        /* Pulse is enable: get pulse period */
        unsigned int per = ym.data[voice*2] | ((ym.data[voice*2+1]&15) << 8);
        /* Specific case where the pulse is
         * used at 125Khz which lead the filter
         * to roughly divide the output level
         * by 2. It's equivalent to -2 volume
         * level. */
        if (per <= 1)
          vol = 13;
      }
    }
  }
  return vol;
}
unsigned char emu_getVolVoice1() __attribute__((noinline));
unsigned char EMSCRIPTEN_KEEPALIVE emu_getVolVoice1() {
	return getVolume(0);
//	return YM_readreg(8, 0);
}

unsigned char emu_getVolVoice2() __attribute__((noinline));
unsigned char EMSCRIPTEN_KEEPALIVE emu_getVolVoice2() {
	return getVolume(1);
//	return YM_readreg(9, 0);
}

unsigned char emu_getVolVoice3() __attribute__((noinline));
unsigned char EMSCRIPTEN_KEEPALIVE emu_getVolVoice3() {
	return getVolume(2);
//	return YM_readreg(10, 0);
}

