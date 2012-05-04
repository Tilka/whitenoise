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


/* audio.c
 * a wrapper which abstracts the audio-related library calls away from the
 * underlying implementations.
 */

#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include "audio.h"


/* initialize ALSA */
static void alsa_init(audio_dev_handle* handle)
{
    int err;

    /* Set up the sound card */
    if ( (err = snd_pcm_open(&handle->alsa_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0 )
    {
        printf("snd_pcm_open failed: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    if ( (err = snd_pcm_set_params(handle->alsa_handle,
                                   handle->format,
                                   SND_PCM_ACCESS_RW_INTERLEAVED,
                                   handle->channels,
                                   handle->rate,
                                   1, /* allow resampling */
                                   handle->latency * 1000)) < 0) {
        printf("snd_pcm_set_params failed: %s\n", snd_strerror(err));
	exit(EXIT_FAILURE);
    }
}


/* initialize the sound card or connect to aRts server */
void audio_init(audio_dev_handle* handle, int rate, int latency, int try_arts)
{
#ifdef HAS_ARTS
    int artserr = 0;

    handle->arts_handle = NULL;
    handle->use_arts    = 0;
#endif
    handle->alsa_handle = NULL;
    handle->channels    = 1;  /* mono */
    handle->format      = SND_PCM_FORMAT_U8;
    handle->rate        = rate;
    handle->latency     = latency; /* in ms */

#ifdef HAS_ARTS    
    if( try_arts )
    {
        artserr = arts_init();
        if( artserr < 0 )
        {
            fprintf(stderr, "Error initializing aRts: %s\n", arts_error_text(artserr));
            exit(-1);
        }
        handle->arts_handle = arts_play_stream( *rate, 8, handle->channels, "arts-whitenoise" );
        arts_stream_set(handle->arts_handle, ARTS_P_BUFFER_TIME, *latency);
        handle->use_arts = 1;
    }
    else
#endif
    {
        alsa_init(handle);
    }
}



/* close sound device or disconnect from aRts server */
void audio_exit(audio_dev_handle* handle)
{
#ifdef HAS_ARTS
    if(handle->use_arts)
    {
        arts_close_stream(handle->arts_handle);
        arts_free();
    }
    else
#endif
    {
        snd_pcm_close(handle->alsa_handle);
    }
}



/* send audio to soundcard */
void audio_write(audio_dev_handle* handle, unsigned char* buffer, int size)
{
#ifdef HAS_ARTS
    if(handle->use_arts)
    {
        arts_write(handle->arts_handle, buffer, size);
    }
    else
#endif
    {
        snd_pcm_writei(handle->alsa_handle, buffer, size);
    }
}



/* close the sound device and reopen with the requested rate */
void audio_set_rate(audio_dev_handle* handle, int rate)
{
    handle->rate = rate;
#ifdef HAS_ARTS
    if( handle->use_arts )
    {
        arts_close_stream(handle->arts_handle);
        handle->arts_handle = arts_play_stream(rate, 8, handle->channels+1, "arts-whitenoise");
    }
    else
#endif
    {
        snd_pcm_close(handle->alsa_handle);
        alsa_init(handle);
    }
}



/* configure the audio buffer sizes.  The latency parameter is in millisec. */
void audio_set_latency(audio_dev_handle* handle, int latency)
{
    handle->latency = latency;
#ifdef HAS_ARTS
    if(handle->use_arts)
    {
        arts_close_stream(handle->arts_handle);
        handle->arts_handle = arts_play_stream(handle->rate, 8, handle->channels+1, "arts-whitenoise");
        arts_stream_set(handle->arts_handle, ARTS_P_BUFFER_TIME, latency);
    }
    else
#endif
    {
        snd_pcm_close(handle->alsa_handle);
        alsa_init(handle);
    }
}

/* arch-tag: DO_NOT_CHANGE_83580ce5-5d4c-4c21-9852-d05141de6afa */
