/*  whitenoise -- A command-line ambient random noise generator.
    Copyright (C) 2001, 2002, 2004, 2010 Paul Pelzl

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAS_ARTS
#include <artsc.h>
#endif

#include <alsa/asoundlib.h>

typedef struct 
{
    snd_pcm_t* alsa_handle;
    int latency;
    int channels;
    int format;
    int rate;
#ifdef HAS_ARTS
    arts_stream_t arts_handle;
    int use_arts;
#endif
} audio_dev_handle;


void audio_init(audio_dev_handle* handle, int rate, int latency, int try_arts);
void audio_exit(audio_dev_handle* handle);
void audio_write(audio_dev_handle* handle, unsigned char* buffer, int size);
void audio_set_rate(audio_dev_handle* handle, int rate);
void audio_set_latency(audio_dev_handle* handle, int latency);



/* arch-tag: DO_NOT_CHANGE_30c6cd8d-e90e-4ec5-b26c-59f3b8ab1a63 */
