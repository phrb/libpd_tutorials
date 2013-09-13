#include <stdio.h>
#include "z_libpd.c"
#include "z_libpd.h"

/*
 * This is a simple test written for libpd functionalities.
 * It covers writing and binding hook functions that will
 * be called by Pd during the patch's execution. The matching
 * patch is included with the code.
 *
 * The functions cover printing, sending and receiving floats, 
 * bangs and messages.
 *
 */
void pdprint ( const char * s ) 
{
  printf ( "%s\n", s );
}
void pdfloat ( const char * recv, float x )
{
    printf ( "Val=%f\n", x );
}
void pdbang ( const char * recv )
{
    printf ( "banged=%s\n", recv );
}
void pdmessage ( const char * source, const char * symbol, int argc, t_atom * argv )
{
    int i;
    printf ( "Received message from patch: \n" );
    printf ( "Source=%s\n", source );
    printf ( "Symbol=%s\n", symbol );
    printf ( "Number of Arguments=%d\n", argc );
    /* TODO t_atom management. */
}
int main(int argc, char **argv) {
    if (argc < 3) 
    {
        fprintf( stderr, "usage: %s file folder\n", argv[ 0 ] );
        return -1;
    }
    /* Definitions of targets and values. */
    const char * BANG_TARGET            = "send_bang";
    const char * MESSAGE_TARGET         = "send_message_bang";
    const char * FLOAT_TARGET           = "send_float";
    const char * HELLO_WORLD_TARGET     = "ask_hello_world";
    const char * MESSAGE_BINDING_TARGET = "message_test";
    const char * BANG_BINDING_TARGET    = "bang_test";
    const char * FLOAT_BINDING_TARGET   = "float_test";
    const int    FLOAT_VALUE            = 222;
    int srate = 44100;
    int block_size = 64;
    int real_seconds = 3;
    int pd_ticks = ( real_seconds * srate ) / block_size;
    int i = 0;
    void * patch;
    /* 
     * z_libpd.h  was modified to include this set of functions,
     * since simply setting the hooks wasn't working.
     * There certainly is a better way to do it, but such way
     * is yet undiscovered by me.
     *
     */
    libpd_set_printhook ( (t_libpd_printhook) pdprint );
    libpd_set_floathook ( (t_libpd_floathook) pdfloat );
    libpd_set_banghook ( (t_libpd_banghook) pdbang );
    libpd_set_messagehook ( (t_libpd_messagehook) pdmessage ); 
    /* 
     * These function calls set up libpd and Pd. 
     * In this example, pd will open with one input
     * channel, and two output channels.
     *
     */
    libpd_init ( );
    libpd_init_audio ( 1, 2, srate );
    float inbuf[ 64 ], outbuf[ 128 ]; 
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
     * Bindings for the written functions, that will
     * be called by pd when targets receive
     * bangs, numbers or messages.
     *
     * Note that "print" messages from pd are subscribed to
     * automatically.
     *
     */
    libpd_bind ( BANG_BINDING_TARGET );
    libpd_bind ( FLOAT_BINDING_TARGET );
    libpd_bind ( MESSAGE_BINDING_TARGET );
    /* 
     * Series of "setting up" bangs,
     * so pd calls the appropriated functions
     * to each binded target.
     *
     */
    libpd_bang ( HELLO_WORLD_TARGET );
    libpd_bang ( BANG_TARGET );
    libpd_bang ( MESSAGE_TARGET );
    libpd_float ( FLOAT_TARGET, FLOAT_VALUE );
    /* 
     * This single pd processing tick, makes sure
     * pd will call the functions.
     *
     */
    libpd_process_raw ( inbuf, outbuf );
    /*
     * In case you want pd to compute for a certain
     * ammount of time, uncomment this for loop,
     * and set the real_seconds int.

    for ( i = 0; i < pd_ticks; i++ )
    {
        libpd_process_raw ( inbuf, outbuf );   
    }
    
    */
    libpd_closefile ( patch );
    return 0;
}
