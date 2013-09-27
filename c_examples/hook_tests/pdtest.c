#include <stdio.h>
#include "z_libpd.h"

/*
 * This is a minimal test for client code using a different hook
 * - in this case, printhook - than the one declared in at
 * libpd.so, the dynamic library and position independent code,
 * that declares the hook and initializes then.
 *
 * The test prints the location of libpd_printhook in client code
 * memory.
 *
 * z_libpd.c was also modified to print the location of its own
 * libpd_printhook.
 *
 * The solution for the different hooks (copied?) was writing
 * interface functions to set the hooks inside z_libpd.h from
 * client code.
 *
 * The purpose of this test is finding the real problem, and
 * also a way to solve it in a better way.
 *
 */
void pdprint ( const char * s ) 
{
  printf ( "%s\n", s );
}
int main(int argc, char **argv)
{
    /* Setting hook with patch functions. */
    libpd_set_printhook ( (t_libpd_printhook) &pdprint );
    printf ( "pdtest.c: libpd_printhook located at %p\n", libpd_printhook );
    /* 
     * The following function call will set up the pd hooks with our
     * function set at libpd_printhook, and print libpd_printhook
     * location in program memory (in the scope of libpd.so).
     *
     */
    libpd_init ( );
    return 0;
}
