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


/* directly open the /dev/dsp device */
void open_dev_dsp(audio_dev_handle* handle)
{
    /* Set up the sound card */
    if ( (handle->dev_dsp_handle = open("/dev/dsp", O_WRONLY)) == -1 ) 
    {
        perror("Opening /dev/dsp");
        exit(-1);
    }
    if ( ioctl(handle->dev_dsp_handle, SNDCTL_DSP_SETFRAGMENT, &(handle->fragment)) == -1 ) 
    {
        perror("fragment fragment size");
        exit(errno);
    }
    if ( ioctl(handle->dev_dsp_handle, SNDCTL_DSP_CHANNELS, &(handle->channels)) == -1 ) 
    {
        perror("fragment stereo");
        exit(errno);
    }
    if ( ioctl(handle->dev_dsp_handle, SNDCTL_DSP_SETFMT, &(handle->format)) == -1 ) 
    {
        perror("fragment format");
        exit(errno);
    }
    if ( ioctl(handle->dev_dsp_handle, SNDCTL_DSP_SPEED, &(handle->rate)) == -1 ) 
    {
        perror("fragment sample rate");
        exit(errno);
    }
}


/* initialize the sound card or connect to aRts server */
audio_dev_handle audio_init(int* rate, int* latency, int try_arts)
{
    audio_dev_handle handle;
    double samples;
#ifdef HAS_ARTS
    int artserr = 0;

    handle.arts_handle    = NULL;
    handle.use_arts       = 0;
#endif
    handle.dev_dsp_handle = -1;
    handle.channels       = 1;  /* mono */
    handle.format         = AFMT_U8;
    handle.rate           = *rate;

    samples = ((double) *latency) / 1000.0 * ((double) handle.rate);
    samples /= 2.0;
    handle.fragment = ((int) (log(samples) / log(2.0))) + 2*65536;

#ifdef HAS_ARTS    
    if( try_arts )
    {
        artserr = arts_init();
        if( artserr < 0 )
        {
            fprintf(stderr, "Error initializing aRts: %s\n", arts_error_text(artserr));
            exit(-1);
        }
        handle.arts_handle = arts_play_stream( *rate, 8, handle.channels+1, "arts-whitenoise" );
        arts_stream_set(handle.arts_handle, ARTS_P_BUFFER_TIME, *latency);
        handle.use_arts = 1;
    }
    else
#endif
    {
        open_dev_dsp(&handle);
    }

    return handle;
}



/* close sound device or disconnect from aRts server */
void audio_exit( audio_dev_handle handle )
{
#ifdef HAS_ARTS
    if( handle.use_arts )
    {
        arts_close_stream( handle.arts_handle );
        arts_free();
    }
    else
#endif
    {
        close(handle.dev_dsp_handle);
    }
}



/* send audio to soundcard */
void audio_write(audio_dev_handle handle, unsigned char* buffer, int size)
{
#ifdef HAS_ARTS
    if( handle.use_arts )
    {
        arts_write(handle.arts_handle, buffer, size);
    }
    else
#endif
    {
        write(handle.dev_dsp_handle, buffer, size);
    }
}



/* close the sound device and reopen with the requested rate */
void audio_set_rate( audio_dev_handle* handle, int rate )
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
        close(handle->dev_dsp_handle);
        open_dev_dsp(handle);
    }
}



/* configure the audio buffer sizes.  The latency parameter is in millisec. */
void audio_set_latency(audio_dev_handle* handle, int latency)
{
    double samples;
#ifdef HAS_ARTS
    if( handle->use_arts )
    {
        arts_close_stream(handle->arts_handle);
        handle->arts_handle = arts_play_stream(handle->rate, 8, handle->channels+1, "arts-whitenoise");
        arts_stream_set(handle->arts_handle, ARTS_P_BUFFER_TIME, latency);
    }
    else
#endif
    {
        samples = ((double) latency) / 1000.0 * ((double) handle->rate);
        samples /= 2.0;

        handle->fragment = ((int) (log(samples) / log(2.0))) + 2*65536;
        close(handle->dev_dsp_handle);
        open_dev_dsp(handle);
    }
}





/* arch-tag: DO_NOT_CHANGE_83580ce5-5d4c-4c21-9852-d05141de6afa */
