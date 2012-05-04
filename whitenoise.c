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

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include "filter.h"
#include "audio.h"


#ifdef HAS_FFTW3
#include <fftw3.h>
#include "plot.h"
#endif

#define SAMPLE_SIZE 1024
#define COMMAND_SIZE 20


#define DEFAULT_CUTOFF      0.3
#define DEFAULT_RATE        22050
#define DEFAULT_FILTER      BLACKMAN
#define DEFAULT_FILTER_LEN  25
#define DEFAULT_RUN_TIME    (-1)
#define DEFAULT_FADE_TIME   (-1)
#define DEFAULT_PLOT_WIDTH  320
#define DEFAULT_LATENCY     200


volatile int shutdown = 0;
void catchSIGINT( int signal )
{
    shutdown = 1;
}


/* Parse command-line flag to read the attached argument.  Allows
 * for optional whitespace between the flag and the arg. */
const char * get_flag_val(int argc, char *argv[], int *p_currarg)
{
    if (strlen(argv[*p_currarg]) > 2)
    {
        return argv[*p_currarg] + 2;
    }
    else if ((*p_currarg) + 1 < argc)
    {
        (*p_currarg)++;
        return argv[*p_currarg];
    }

    return NULL;
}


int main(int argc, char* argv[]) 
{
    int i;
    const char* flag_val;
    double* coeff = NULL; 
    unsigned char* data = NULL;
    unsigned char* filteredData = NULL;
    
    int filterLength = DEFAULT_FILTER_LEN;
    double cutoff = DEFAULT_CUTOFF;
    int rate = DEFAULT_RATE;
    int filterType = DEFAULT_FILTER;
    int acount;
    time_t startTime;
    int runTime = DEFAULT_RUN_TIME;
    int fadeTime = DEFAULT_FADE_TIME;
    double dy;
    double dtemp;
    double ddata;

#ifdef HAS_FFTW3
    double* fft_in = NULL;
    fftw_complex* fft_out = NULL;
    fftw_plan fft_plan = NULL;
    int do_plot = 0;
    int plotWidth = DEFAULT_PLOT_WIDTH;
#endif

    int use_arts = 0;
    audio_dev_handle audio_handle;

    int latency = DEFAULT_LATENCY;
    int read_stdin = 0;
    int readerr;
    char readbuf;
    char command[COMMAND_SIZE];
    int command_chars = 0;


    signal( SIGINT, catchSIGINT );  /* Exit cleanly on ^C */
        

    /* Parse through command-line options */    
    argv++;
    argc--;

    acount = 0;
    while( acount < argc )
    {
        /* Set frequency cutoff */
        if (strncmp( argv[acount], "-c", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) cutoff = atof(flag_val);

            if (cutoff <= 0.0 || cutoff >= 1.0)
            {
                fprintf(stderr, "\nError: Frequency cutoff must be in the range (0, 1).\n");
                fprintf(stderr, "Setting cutoff = %f.\n", DEFAULT_CUTOFF);

                cutoff = DEFAULT_CUTOFF;
            }
        }
        /* Set samplerate */
        else if (strncmp( argv[acount], "-r", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) rate = atoi(flag_val);
            
            if (rate != 11025 && rate != 22050)
            {
                fprintf(stderr, "\nError: Sample rate must be 11025 or 22050.\n");
                fprintf(stderr, "Setting rate = %d.\n", DEFAULT_RATE);

                rate = DEFAULT_RATE;
            }
        }
        /* Set filter type */
        else if (strncmp( argv[acount], "-F", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) filterType = atof(flag_val);
            
            if (filterType < 0 || filterType > 4)
            {
                fprintf(stderr, "\nError: FILTNUM must be in the range [0, 4].\n");
                fprintf(stderr, "Setting FILTNUM=%d.\n", DEFAULT_FILTER);

                filterType = DEFAULT_FILTER;
            }
        }        
        /* Set filter length */        
        else if (strncmp( argv[acount], "-l", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) filterLength = atoi(flag_val);
            
            if (filterLength <= 0 || filterLength > 100)
            {
                fprintf(stderr, "\nError: Filter length must be in the range [1, 100].\n");
                fprintf(stderr, "Setting filter length = %d.\n", DEFAULT_FILTER_LEN);
                
                filterLength = DEFAULT_FILTER_LEN;
            }
        }      
        /* Set run time */
        else if (strncmp( argv[acount], "-t", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) runTime = 60 * atoi(flag_val);
            
            if (runTime <= 0)
            {
                runTime = DEFAULT_RUN_TIME;
            }
        }
        /* Set fade time */
        else if (strncmp( argv[acount], "-f", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) fadeTime = atoi(flag_val);
            
            if (fadeTime <= 0)
            {
                fadeTime = DEFAULT_FADE_TIME;
            }
        }              
#ifdef HAS_FFTW3
        /* Generate a frequency response plot */
        else if (strncmp( argv[acount], "-p", 2 ) == 0)
        {
            do_plot = 1;

            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) plotWidth = atoi(flag_val);
            
            if (plotWidth < 0)
            {
                plotWidth = DEFAULT_PLOT_WIDTH;
            }
        }              
#endif
        else if (strncmp( argv[acount], "-L", 2 ) == 0)
        {
            flag_val = get_flag_val(argc, argv, &acount);
            if (flag_val != NULL) latency = atoi(flag_val);
            
            if (latency < 100)
            {
                latency = 100;
            }
            else if (latency > 10000)
            {
                latency = DEFAULT_LATENCY;
            }
        }              
#ifdef HAS_ARTS
        /* Use aRts */
        else if (strcmp( argv[acount], "-a" ) == 0)
        {
            use_arts = 1;
        }
#endif
        /* Output version information */
        else if (strcmp( argv[acount], "-v" ) == 0 ||
                 strcmp( argv[acount], "--version" ) == 0)
        {
            printf("\nwhitenoise v%s  --  A command-line ambient random noise generator.\n", PACKAGE_VERSION);
            printf("Copyright (C) 2001, 2002, 2004, 2010 Paul Pelzl\n\n");
            printf("whitenoise comes with ABSOLUTELY NO WARRANTY.\n");
            printf("This is free software, and you are welcome to redistribute it\n");
            printf("under certain conditions; see 'COPYING' for details.\n\n");
            return(0);
        }
        /* Choose to read commands from stdin */
        else if (strcmp( argv[acount], "-s" ) == 0)
        {
            read_stdin = 1;
            /* set stdin to nonblocking, so it can be used for control */
            if (fcntl(0, F_SETFL, O_NONBLOCK) == -1) 
            {
                fprintf(stderr, "Error: could not set stdin to nonblocking.\n");
                fprintf(stderr, "Not accepting input from stdin.\n\n");
                read_stdin = 0;
            }
            else 
            {
                printf("Now accepting commands from stdin:\n");
            }
        }
        /* View help screen */
        else if (strcmp( argv[acount], "--help" ) == 0  ||
                 strcmp( argv[acount], "-?" ) == 0)
        {
            printf("\nwhitenoise v%s\n", PACKAGE_VERSION);
            printf("-----------------------------------------------------------------------\n");
            printf("USAGE:  whitenoise [option(s)]\n\n");
            printf("OPTIONS:\n");
            printf("    -c CUTOFF           Sets the lowpass frequency cutoff.\n");
            printf("                        'CUTOFF' is a number in the range (0, 1),\n");
            printf("                        with a default value of 0.3.\n\n");
            printf("    -r RATE             Sets the samplerate for the sound card, in Hz.\n");
            printf("                        Acceptable values are 11025 and 22050,\n");
            printf("                        with a default value of 22050.\n\n");
            printf("    -F FILTNUM          Use a filter of type 'FILTNUM', which\n");
            printf("                        may take the following values:\n");
            printf("                           0:  Blackman-windowed FIR lowpass (default)\n");
            printf("                           1:  Bartlett-windowed FIR lowpass\n");
            printf("                           2:  Hanning-windowed FIR lowpass\n");
            printf("                           3:  Hamming-windowed FIR lowpass\n");            
            printf("                           4:  Rectangular-windowed FIR lowpass\n\n");
            printf("    -l LENGTH           Sets the FIR filter length.\n");
            printf("                        'LENGTH' is an integer in the range [1 100],\n");
            printf("                        with a default value of 25.\n\n"); 
            printf("    -t TIME             Sets the length of time to generate\n");
            printf("                        noise, in minutes.\n\n");
            printf("    -f FADETIME         Fade the noise out over 'FADETIME'\n");
            printf("                        seconds.  Valid only when used along\n");
            printf("                        with the '-t' flag.\n\n");
#ifdef HAS_FFTW3
            printf("    -p WIDTH            Output a plot of the filter frequency response,\n");
            printf("                        'WIDTH' is the horizontal resolution of the\n");
            printf("                        PNG image, with default 320.  The image will\n");
            printf("                        have filename ~/.whitenoise/filter.png .\n\n");
#endif
            printf("    -L LATENCY          Configure the audio buffers for approximately\n");
            printf("                        'LATENCY' milliseconds of delay, with default\n");
            printf("                        200.  Increase the value to alleviate\n");
            printf("                        problems with skipping.\n\n");
#ifdef HAS_ARTS
            printf("    -a                  Interface with aRts instead of opening\n");
            printf("                        /dev/dsp directly.\n\n");
#endif
            printf("    -s                  Read commands from stdin in realtime.\n\n");
            printf("    -v, --version       Print version information.\n\n");    
            printf("    -?, --help          This help page.\n\n");
            return(0);
        }
        
        acount++;
    }
       
    // (Either succeeds or aborts the program)
    audio_init(&audio_handle, rate, latency, use_arts);
    
    /* Create the lowpass filter for a given length */
    if ((coeff = (double *) calloc(filterLength, sizeof(double))) == NULL)
    {
        fprintf(stderr, "Error: could not allocate filter of length %d.\n", filterLength);
        goto cleanup;
    }
    getFilterCoeff( filterType, coeff, filterLength, cutoff );  

#ifdef HAS_FFTW3
    /* Initialize FFTW */
    if ((fft_in = (double *) fftw_malloc(FFT_SIZE * sizeof(double))) == NULL ||
        (fft_out = (fftw_complex *) fftw_malloc((FFT_SIZE/2 + 1) * sizeof(fftw_complex))) == NULL)
    {
        fprintf(stderr, "Error: could not allocate FFT memory.\n");
        goto cleanup;
    }
    
    if ((fft_plan = fftw_plan_dft_r2c_1d(FFT_SIZE, fft_in, fft_out, FFTW_ESTIMATE)) == NULL) {
        fprintf(stderr, "Error: could not create FFT plan.\n");
        goto cleanup;
    }
    
    if (do_plot)
    {
        plotFilter(coeff, filterLength, fft_in, fft_out, fft_plan, rate, plotWidth);
    }
#endif
    
    if ((data = (unsigned char *) malloc(SAMPLE_SIZE * 2)) == NULL ||
        (filteredData = (unsigned char *) malloc(SAMPLE_SIZE)) == NULL)
    {
        fprintf(stderr, "Error: could not allocate filter memory.\n");
        goto cleanup;
    }

    startTime = time(NULL); 
    if (runTime > 0)
    {  
        printf("Generating noise for %d minutes", runTime/60);    
        if (fadeTime >= 0)
        {
            printf(", then fading for %d seconds.\n", fadeTime);
        }
        else
        {
            printf(".\n");
        }
    }

    /* Generate uniform random noise, and lowpass filter it. */
    for(i=0; i<SAMPLE_SIZE; i++)
    {
        *(data+i) = rand();
    }
    
    
    memset(command, 0, sizeof(command));
    while(!shutdown && (time(NULL)-startTime < runTime || runTime < 0))
    {
        if (read_stdin)
        {
            /* Read commands from stdin */
            readerr = read(0, &readbuf, 1);
            if (readerr != -1) 
            {
                if (readbuf != '\n') 
                {
                    /* newline indicates a complete command */ 
                    if (command_chars < COMMAND_SIZE-1) 
                    {
                        command[command_chars] = readbuf;
                        command_chars++;
                    }
                    else 
                    {
                        /* if the buffer is overflowed, just throw it away and start over */
                        command_chars = 0;
                        memset(command, 0, sizeof(command));
                    }
                }
                else if (command_chars > 0) 
                { /* A complete command has been read; process it */

                    /* change cutoff */
                    if (command[0] == 'c') 
                    {
                        cutoff = atof(&command[1]);
                        if (cutoff <= 0.0 || cutoff >= 1.0)
                        {
                            cutoff = DEFAULT_CUTOFF;
                        }
                        getFilterCoeff( filterType, coeff, filterLength, cutoff );  
                    }
                    /* set samplerate */
                    else if (command[0] == 'r')
                    {
                        rate = atoi(&command[1]);
                        if (rate != 11025 && rate != 22050)
                        {
                            rate = DEFAULT_RATE;
                        }
                        audio_set_rate(&audio_handle, rate);
                    }
                    /* change filter type */
                    else if (command[0] == 'F') 
                    {
                        filterType = atoi(&command[1]);
                        if (filterType < 0 || filterType > 4)
                        {
                            filterType = DEFAULT_FILTER;
                        }
                        getFilterCoeff( filterType, coeff, filterLength, cutoff );  
                    }
                    /* change filter length */
                    else if (command[0] == 'l')
                    {
                        filterLength = atoi(&command[1]);
                        if (filterLength <= 0 || filterLength > 100)
                        {
                            filterLength = DEFAULT_FILTER_LEN;
                        }
                        free(coeff);
                        if ((coeff = (double *) calloc(filterLength, sizeof(double))) == NULL)
                        {
                            goto cleanup;
                        }
                        getFilterCoeff( filterType, coeff, filterLength, cutoff );  
                    }      
                    /* set run time, in minutes */
                    else if (command[0] == 't')
                    {
                        runTime = 60*atoi(&command[1]);
                        startTime = time(NULL);
                        if (runTime <= 0)
                        {
                            runTime = DEFAULT_RUN_TIME;
                        }
                    }
                    /* set fade time, in seconds */
                    else if (command[0] == 'f')
                    {
                        fadeTime = atoi(&command[1]);
                        if (fadeTime <= 0)
                        {
                            fadeTime = DEFAULT_FADE_TIME;
                        }
                    }
#ifdef HAS_FFTW3
                    /* generate a frequency reponse plot */
                    else if (command[0] == 'p')
                    {
                        plotWidth = atoi(&command[1]);
                        if (plotWidth < 0)
                        {
                            plotWidth = DEFAULT_PLOT_WIDTH;
                        }
                        plotFilter(coeff, filterLength, fft_in, fft_out, fft_plan, rate, plotWidth);
                    }
#endif
                    /* set the latency */
                    else if (command[0] == 'L')
                    {
                        latency = atoi(&command[1]);
                        if (latency < 100 || latency > 10000)
                        {
                            latency = DEFAULT_LATENCY;
                        }
                        audio_set_latency(&audio_handle, latency);
                    }
                    /* quit */
                    else if (command[0] == 'q')
                    {
                        goto cleanup;
                    }

                    memset(command, 0, sizeof(command));
                    command_chars = 0;
                }
            }
        }


        for (i = SAMPLE_SIZE; i < SAMPLE_SIZE * 2; i++) 
        {
            *(data+i) = rand();
        }

        filter(data, filteredData, SAMPLE_SIZE, coeff, filterLength); 
        /* Output the filtered noise to the sound card. */
        audio_write(&audio_handle, filteredData, SAMPLE_SIZE);
    }


    /* Fade the noise out for 'fadeTime' secs */
    if (!shutdown && runTime >= 0 && fadeTime >= 0)
    {
        printf("Beginning fade...\n");
        startTime = time(NULL);
        /* Use a linear function to dampen the amplitude.  */
        /* 'dtemp' is the dampening coefficient, and 'dy'  */
        /* is the constant amount by which it is decreased */
        /* every iteration.                                */
        dtemp = 1.0;
        dy    = 1.0 / (((double) rate) * ((double) fadeTime));
        
        while(time(NULL)-startTime <= fadeTime)
        {
            for (i = SAMPLE_SIZE; i < SAMPLE_SIZE * 2; i++) 
            {
                ddata = (double) ((unsigned char) rand());
                ddata -= 128.0;
                ddata *= dtemp;
                ddata += 128.0;
                
                *(data+i) = (unsigned char) ddata;                
                dtemp -= dy;
                if (dtemp < 0.0)
                {
                    dtemp = 0.0;
                }
            }

            filter(data, filteredData, SAMPLE_SIZE, coeff, filterLength); 
            /* Output the filtered noise to the sound card. */
            audio_write(&audio_handle, filteredData, SAMPLE_SIZE);
        }
    }
            
    
cleanup:
    /* Clean up */
    free(coeff);
    free(data);
    free(filteredData);

#ifdef HAS_FFTW3
    if (fft_plan != NULL) fftw_destroy_plan(fft_plan);
    if (fft_in != NULL) fftw_free(fft_in);
    if (fft_out != NULL) fftw_free(fft_out);
    fftw_cleanup();
#endif

    audio_exit(&audio_handle);
    return(0);
}


/* arch-tag: main loop */
