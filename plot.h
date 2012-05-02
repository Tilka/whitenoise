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

#define FFT_SIZE 2048

#ifdef HAS_FFTW3
#include <fftw3.h>
void plotFilter( double* coeff, int M, double* fft_in, fftw_complex* fft_out, 
      fftw_plan p, int rate, int width );
#endif



/* arch-tag: DO_NOT_CHANGE_9aba1ba5-05b7-4730-a383-f34ff1f02ba1 */
