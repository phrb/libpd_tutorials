#include <stdio.h>
#include <stdlib.h>
#include "../libpd_wrapper/z_libpd.h"

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
void dump_buffers ( float * in, float * out )
{
    int i;
    printf ( "in_buffer= [" );
    for ( i = 0; i < 64; i++ )
    {
        printf ( " %f", in[ i ] );
        if ( i % 4 == 0 )
        {
            printf ( "\n" );
        }
    }
    printf ( " ]\n" );  
    printf ( "out_buffer= [" );
    for ( i = 0; i < 128; i++ )
    {
        printf ( " %f", out[ i ] );
        if ( i % 4 == 0 )
        {
            printf ( "\n" );
        }
    }
    printf ( " ]\n" );  
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
    const char * SWITCH_TARGET          = "send_switch";
    const char * HELLO_WORLD_TARGET     = "ask_hello_world";
    const char * MESSAGE_BINDING_TARGET = "message_test";
    const char * BANG_BINDING_TARGET    = "bang_test";
    const char * FLOAT_BINDING_TARGET   = "float_test";
    const int    FLOAT_VALUE            = 222;
    int srate = 44100;
    int block_size = 64;
    /* Arbitrary number of processing seconds. */
    int real_pd_seconds = 6;
    int pd_ticks = ( real_pd_seconds * srate ) / block_size;
    int i = 0;
    void * patch;
    /* 
     * Bindings for the written functions, that will
     * be called by pd when targets receive
     * bangs, numbers or messages.
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
     * Bindings for the symbols to wich
     * we wish to be subscribed to.
     *
     * Pd will send us the information
     * sent to those targets via our
     * callback functions.
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
     * A single Pd processing tick.
     *
     */
   libpd_process_float ( 1, inbuf, outbuf );
   /*
    * Dumping buffers to stdout.
    *
    */
   dump_buffers ( inbuf, outbuf );
    /*
     * In case you want pd to compute for a certain
     * ammount of time, uncomment this loop,
     * and set the real_pd_seconds int.
     *
    for ( i = 0; i < pd_ticks; i++ )
    {
        libpd_process_float ( 1, inbuf, outbuf );
         *
         * A simple way to check buffer
         * exchange: dumping to stdout!
         *
         *
        dump_buffers ( inbuf, outbuf );
    }*/
    libpd_closefile ( patch );
    return 0;
}
