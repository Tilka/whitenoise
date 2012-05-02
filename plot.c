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


/* plot.c
 * Functions associated with plotting filters in both time and frequency domains.
 */ 


#include "plot.h"

#ifdef HAS_FFTW3
#include <fftw3.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>
#include "math.h"


#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define PLOT_RELPATH        "/.whitenoise/plot-data"
#define COMMANDS_RELPATH    "/.whitenoise/plot-commands"


char * path_join(char *buf, const char *home_path, const char *rel_path)
{
    strcpy(buf, home_path);
    strcat(buf, rel_path);
    return buf;
}


void plotFilter( double* coeff, int M, double* fft_in, fftw_complex* fft_out, 
        fftw_plan p, int rate, int width )
{
    int i, err;
    double a, b, f;
    double fft_mag[FFT_SIZE];
    FILE* gnuplot_data;
    FILE* gnuplot_commands;
    double scale;
    const char* home_path;
    char* file_path = NULL;
    int file_path_chars = 0;
    char cwd[PATH_MAX];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "Could not get working directory.\n");
        fprintf(stderr, "Aborting frequency response plot.\n");
        return;
    }
    
    /* gnuplot hangs on very small plots... */
    if (width < 100)
    {
       width = 100;
    }
    /* ...and large plots consume require unreasonable memory */
    else if (width > 3000)
    {
       width = 3000;
    }
    scale = ((double) width) / 640.0;

    /* the FFT input is a zero-padded version of the filter coefficients */
    for (i=0; i<M; i++)
    {
        fft_in[i] = coeff[i];
    }
    for (i=M; i<FFT_SIZE; i++)
    {
        fft_in[i] = 0.0;
    }

    fftw_execute(p);

    /* get the FFT magnitude (dB) */
    for (i=0; i<FFT_SIZE/2 + 1; i++)
    {
        a = fft_out[i][0];
        b = fft_out[i][1];

        fft_mag[i] = 10.0*log10(sqrt(a*a + b*b));
    }

    home_path = getenv("HOME");
    if (home_path == NULL)
    {
        home_path = ".";
    }
    
    file_path_chars = strlen(home_path) + MAX(strlen(PLOT_RELPATH), strlen(COMMANDS_RELPATH));
    if ((file_path =
            (char *) malloc((file_path_chars + 1) * sizeof(char))) == NULL)
    {
        fprintf(stderr, "Out of memory.  Aborting frequency response plot.\n");
        goto cleanup;
    }
    
    if (mkdir(dirname(path_join(file_path, home_path, PLOT_RELPATH)), 0744) < 0)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "Could not create \"~/.whitenoise\".\n");
            fprintf(stderr, "Aborting frequency response plot.\n");
            goto cleanup;
        }
    }

    if ((gnuplot_data = fopen(path_join(file_path, home_path, PLOT_RELPATH), "w")) == NULL)
    {
        fprintf(stderr, "Could not open \"~/%s\" for writing.\n", PLOT_RELPATH);
        fprintf(stderr, "Aborting frequency response plot.\n");
        goto cleanup;
    }

    for (i=0; i<FFT_SIZE/2 + 1; i++)
    {
        f = ((double) rate * (double) i) / ((double) (FFT_SIZE/2 + 1)) / 2.0;
        /* a -inf value will cause gnuplot to hang */
        if (fft_mag[i] > -1e200)
        {
            fprintf(gnuplot_data, "%g     %g\n", f, fft_mag[i]);
        }
    }
    fclose(gnuplot_data);

    if ((gnuplot_commands = fopen(path_join(file_path, home_path, COMMANDS_RELPATH), "w")) == NULL)
    {
        fprintf(stderr, "Could not open \"~%s\" for writing.\n", COMMANDS_RELPATH);
        fprintf(stderr, "Aborting frequency response plot.\n");
        goto cleanup;
    }
    fprintf(gnuplot_commands, "set terminal png small\n");
    fprintf(gnuplot_commands, "set size %g,%g\n", scale, scale);
    fprintf(gnuplot_commands, "set output \"filter.png\"\n");
    fprintf(gnuplot_commands, "set title \"Frequency response of lowpass filter\"\n");
    fprintf(gnuplot_commands, "set xlabel \"frequency (Hz)\"\n");
    fprintf(gnuplot_commands, "set ylabel \"gain (dB)\"\n");
    fprintf(gnuplot_commands, "set nokey\n");
    fprintf(gnuplot_commands, "plot \"plot-data\" with lines\n");
    fclose(gnuplot_commands);
    
    if (chdir(dirname(path_join(file_path, home_path, COMMANDS_RELPATH))) < 0)
    {
        fprintf(stderr, "Could not set working directory.\n");
        fprintf(stderr, "Aborting frequency response plot.\n");
        goto cleanup;
    }
    
    if ((err = system("gnuplot < plot-commands")) == -1)
    {
        fprintf(stderr, "Error: failed to launch gnuplot.\n");
        fprintf(stderr, "You must have gnuplot installed in order to generate frequency response plots.\n");
    }

    /* Restore working directory */
    if (chdir(cwd) == -1)
    {
        fprintf(stderr, "Error: could not restore working directory.\n");
    }

cleanup:
    free(file_path);
}

#endif /* HAS_FFTW3 */


/* arch-tag: DO_NOT_CHANGE_fd023c12-c1ad-464a-a565-fcfaa4c031e2 */
