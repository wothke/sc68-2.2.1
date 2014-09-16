/*
 *     sc68 - atari st and amiga music emulator - command line player
 *      Copyright (C) 1998-2001 Ben(jamin) Gerard <ben@sasghipa.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


/* generated config include */
#include <config68.h>

/* Standard Includes */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/* sc68 includes */
#include "api68/api68.h"

static api68_t * sc68 = 0;

/** Display version number. */
static int Version(void)
{
  puts(PACKAGE68 " version " VERSION68 "\n");
  return 0;
}

/** Display usage message. */
static int Usage(void)
{
  puts(
    "sc68 - Atari ST and Amiga music emulator\n"
    "Copyright (C) 1998-2003 Benjamin Gerard <ben@sasghipa.com>\n"
    "\n"
    "This program is free software.\n"
    "\n"
    "usage : sc68 [options] <file.sc68>\n"
    "\n"
    "options:\n"
    "  --help         : Display this message and exit\n"
    "  --version      : Display sc68 version x.y.z and exit\n"
    "  --quiet        : Do not display music info\n"
    "  --track=#      : Choose track to play [0=all tracks]\n"
    "\n"
    );
  return 1;
}

static int quiet = 0;

void Message(const char * fmt, ...)
{
  va_list list;

  if (quiet) {
    return;
  }
  va_start(list, fmt);
  vfprintf(stderr, fmt, list);
  va_end(list);
}

/** Display to output debug statcked error messages.
 */
static void spool_error_message(void)
{
  const char * s;

  if (s = api68_error(), s) {
    fprintf(stderr, "Stacked Error Message:\n");
    do {
      fprintf(stderr, "%s\n", s);
    } while (s = api68_error(), s != NULL);
  }
}

static void DisplayInfo(int track)
{
  api68_music_info_t info;
  if (!api68_music_info(sc68,&info,track,0)) {
    Message("Track      : %d/%d\n",info.track, info.tracks);
    Message("Title      : %s\n",info.title);
    Message("Author     : %s\n",info.author);
    Message("Composer   : %s\n",info.composer);
    Message("Replay     : %s\n",info.replay);
    Message("Hardware   : %s\n",info.hwname);
    Message("Start time : %u:%02u\n",
	    info.start_ms/60000u, (info.start_ms/1000u)%60u);
    Message("Duration   : %s\n", info.time);
  }
}


static int PlayLoop(FILE * out, int all)
{
  static char buffer[512 * 4];

  if (all) {
    api68_play(sc68, 1);
  }

  DisplayInfo(0);

  for (;;) {
    int code;

    code = api68_process(sc68, buffer, sizeof(buffer) >> 2);
    if (code == API68_MIX_ERROR) {
      return -1;
    }

    /* Send audio PCM to stdout. */
    if (fwrite(buffer, sizeof(buffer), 1, out) != 1) {
      perror("sc68");
      return -1;
    }

    if (code & API68_CHANGE) {
      DisplayInfo(-1);
    }

    /* Check for end. */
    if ( (code & API68_END) || (!all && (code & API68_LOOP))) {
      return 0;
    }
  }
  return 0;
}

static int IsIntParam(const char *parm, const char *what, int *res)
{
  if (strstr(parm, what) == parm) {
    int len = strlen(what);
    if (isdigit(parm[len])) {
      *res = atoi(parm+len);
      return 1;
    }
  }
  return 0;
}

int main(int na, char **a)
{
  int i;
  int help = 0;
  char * fname = 0;
  int track = -1;
  int err = 1;
  api68_init_t init68;

  for (i=1; i<na; ++i) {
    if (!strcmp(a[i],"--help")) {
      help = 1;
      break;
    } else if (!strcmp(a[i],"--version")) {
      return Version();
    } else if (!strcmp(a[i],"--quiet")) {
      quiet = 1;
    }
  }

  if (na<2 || help) {
    return Usage();
  }

  for (i=1; i<na; ++i) {
    if (!IsIntParam(a[i],"--track=", &track)) {
      if (fname) {
	fprintf(stderr, "Invalid parameters \"%s\"\n", a[i]);
	return 2;
      } else {
	fname = a[i];
      }
    }
  }

  if (!fname) {
    fprintf (stderr, "Missing input file. try --help\n");
    return 3;
  }
  
  fname = a[1];

  memset(&init68, 0, sizeof(init68));
  init68.alloc = malloc;
  init68.free = free;
#ifdef _DEBUG
  init68.debug = vfprintf;
  init68.debug_cookie = stderr;
#endif

  sc68 = api68_init(&init68);
  if (!sc68) {
    goto error;
  }
  
  /* Verify sc68 file. */
  if (api68_verify_file(fname) < 0) {
    goto error;
  }
  
  if (api68_load_file(sc68, fname)) {
    goto error;
  }
    
  /* Set track number : command line is prior to config force-track */
  if (track > 0) {
    if (api68_play(sc68, track)) {
      goto error;
    }
  }

  /** @todo  Set out stream mode to binary. */

  /* Loop */
  if (PlayLoop(stdout, track == 0) < 0) {
    goto error;
  }

  err = 0;

 error:
  api68_shutdown(sc68);
  if (err) {
    spool_error_message();
    return -1;
  }

  return 0;
}
