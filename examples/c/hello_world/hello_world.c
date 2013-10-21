#include <stdio.h>
#include <stdlib.h>
#include "../libpd_wrapper/z_libpd.h"

/*
 * A simple libpd "Hello, World!" example.
 */
void pdprint ( const char * s ) 
{
    printf ( "%s\n", s );
}
int main(int argc, char **argv) {
    if (argc < 3) 
    {
        fprintf( stderr, "usage: %s file folder\n", argv[ 0 ] );
        return -1;
    }
    int i;
    int srate = 44100;
    int block_size = 64;
    void * patch;
    libpd_set_printhook ( (t_libpd_printhook) pdprint );
    /*
     * These function calls set up libpd and Pd. 
     * In this example, pd will open with one input
     * channel, and two output channels.
     *
     */
    libpd_init ( );
    libpd_init_audio ( 1, 2, srate );
    /* Buffer, to and from Pd. */
    float * inbuf;
    float * outbuf;
    inbuf = malloc ( sizeof ( float ) * 64 );
    outbuf = malloc ( sizeof ( float ) * 128 );
    for ( i = 0; i < 64; i++ )
    {   
        inbuf[ i ] = i;
    }
    /* Message to turn pd dsp on. */
    libpd_start_message ( 1 ); 
    libpd_add_float ( 1.0f );
    libpd_finish_message ( "pd", "dsp" );
    /* 
     * Opens the patch named argv[ 1 ], at the
     * argv[ 2 ] directory.
     *
     */
    patch = libpd_openfile( argv[ 1 ], argv[ 2 ] );
    /* 
     * A single Pd processing tick.
     *
     */
    libpd_process_float ( 1, inbuf, outbuf );
    libpd_closefile ( patch );
    return 0;
}
