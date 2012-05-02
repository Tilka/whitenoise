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


/* filter.c
 * Defines a number of functions used to define and implement FIR filters.
 */ 

#include "filter.h"
#include <stdio.h>



/* Get the filter coefficients.  The user can choose between five different
   standard windowed-FIR filter models.
 */
void getFilterCoeff( int filter_type, double* filt, int M, double cutoff )
{
    int i;
    double temp;
    double temp2;

    printf("\nFrequency cutoff:  %g*pi", cutoff);
    cutoff *= M_PI;
    
    switch( filter_type )
    {
        case BARTLETT:
            printf("\nFilter is Bartlett-windowed FIR lowpass, %d coefficients.\n", M);
            for(i=0; i<M; i++)
            {
                temp = 2.0 * ((double)i)/((double)(M-1));        
                temp2 = ((double)i) - ((double)(M-1))/2.0;
                if( temp2 != 0.0 )
                {
                    if( ((double)i) <= ((double)(M-1))/2.0 )
                    {
                        *(filt+i) = temp * (sin(cutoff*temp2) / M_PI / temp2);
                    }
                    else
                    {
                        *(filt+i) = (2.0 - temp) * 
                                (sin(cutoff*temp2) / M_PI / temp2);
                    }
                }
                else
                {
                    *(filt+i) = cutoff / M_PI;
                }
            }
            break;

        case HANNING:
            printf("\nFilter is Hanning-windowed FIR lowpass, %d coefficients.\n", M);
            for(i=0; i<M; i++)
            {
                temp = ((double)i)/((double)(M-1));
                temp2 = ((double)i) - ((double)(M-1))/2.0;
                if( temp2 != 0.0 )
                {
                    *(filt+i) = (0.5 - 0.5*cos(2.0*M_PI*temp)) *
                            (sin(cutoff*temp2) / M_PI / temp2);
                }
                else
                {
                    *(filt+i) = cutoff / M_PI;
                }
            }
            break;
            
        case HAMMING:
            printf("\nFilter is Hamming-windowed FIR lowpass, %d coefficients.\n", M);
            for(i=0; i<M; i++)
            {
                temp = ((double)i)/((double)(M-1));
                temp2 = ((double)i) - ((double)(M-1))/2.0;
                if( temp2 != 0.0 )
                {
                    *(filt+i) = (0.54 - 0.46*cos(2.0*M_PI*temp)) *
                            (sin(cutoff*temp2) / M_PI / temp2);
                }
                else
                {
                    *(filt+i) = cutoff / M_PI;
                }
            }
            break;
            
        case RECTANGULAR:
            printf("\nFilter is Rectangular-windowed FIR lowpass, %d coefficients.\n", M);
            for(i=0; i<M; i++)
            {
                temp2 = ((double)i) - ((double)(M-1))/2.0;
                if( temp2 != 0.0 )
                {
                    *(filt+i) = sin(cutoff*temp2) / M_PI / temp2;
                }
                else
                {
                    *(filt+i) = cutoff / M_PI;
                }
            }
            break;
            
                                
        default:
        case BLACKMAN:
            printf("\nFilter is Blackman-windowed FIR lowpass, %d coefficients.\n", M);
            for(i=0; i<M; i++)
            {
                temp = ((double)i)/((double)(M-1));
                temp2 = ((double)i) - ((double)(M-1))/2.0;
                if( temp2 != 0.0 )
                {
                    *(filt+i) = (0.42 - 0.5 * cos(2.0*M_PI*temp) + 
                                0.08 * cos(4.0*M_PI*temp)) *
                                (sin(cutoff*temp2) / M_PI / temp2);
                }
                else
                {
                    *(filt+i) = cutoff / M_PI;
                }
            }
            break;
    }
}



/* Filter the data with a previously computed FIR filter.
 * 'data' points to the data to be filtered, length 2*N
 * 'scratch' points to some workspace where the output will go, length N
 * 'N' is the number of samples taken as output from filter
 * 'filt' points to the filter coefficients
 * 'M' is the length of the filter 
 */
void filter( unsigned char* data, unsigned char* output, long N, 
             double* filt, int M )
{
    long n, k;
    double sum;
    
    /* Convolve the first half of the input 'data' with the filter 'filt'. */
    for(n=M-1; n<N+M-1; n++)
    {
        sum = 0.0;
        for(k=0; k<M; k++)
        {
            sum += ((double)(*(data+n-k))) * (*(filt+k));
        }
        
        /* Watch out for unsigned char overflow (clicking sounds) */
        if( sum > 255.0 )
        {
            sum = 255.0;
        }
        else if( sum < 0.0 )
        {
            sum = 0.0;
        }
        
        
        *(output+n-M+1) = sum;                   
    }
    
    /* Copy the second half of 'data' over the first half.  This will
     * ensure continuity of the convolution even though it is broken up
     * into two-second blocks.
     */
    for(n=0; n<N; n++)
    {
        *(data+n) = *(data+n+N);
    }
}



/* arch-tag: FIR filtering */
