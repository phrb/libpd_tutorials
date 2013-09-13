/*
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See http://gitorious.org/pdlib/pages/Libpd for documentation
 *
 */

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "z_libpd.h"
#include "x_libpdreceive.h"
#include "s_stuff.h"
#include "m_imp.h"
#include "g_all_guis.h"

/* Why is pd_init prototyped here? Where is its definition? */
void pd_init(void);

t_libpd_printhook libpd_printhook = NULL;
t_libpd_banghook libpd_banghook = NULL;
t_libpd_floathook libpd_floathook = NULL;
t_libpd_symbolhook libpd_symbolhook = NULL;
t_libpd_listhook libpd_listhook = NULL;
t_libpd_messagehook libpd_messagehook = NULL;

t_libpd_noteonhook libpd_noteonhook = NULL;
t_libpd_controlchangehook libpd_controlchangehook = NULL;
t_libpd_programchangehook libpd_programchangehook = NULL;
t_libpd_pitchbendhook libpd_pitchbendhook = NULL;
t_libpd_aftertouchhook libpd_aftertouchhook = NULL;
t_libpd_polyaftertouchhook libpd_polyaftertouchhook = NULL;
t_libpd_midibytehook libpd_midibytehook = NULL;

static t_atom *argv = NULL, *curr;
static int argm = 0, argc;

static void *get_object(const char *s) {
  t_pd *x = gensym(s)->s_thing;
  return x;
}

/* this is called instead of sys_main() to start things */
void libpd_init(void) {
  signal(SIGFPE, SIG_IGN);
  libpd_start_message(32); // allocate array for message assembly
  /* DEBUG: Printing lipd_printhook location in memory. */
  printf ( "libpd.so: libpd_printhook located at %p\n", libpd_printhook );
  /* DEBUG END */
  sys_printhook = (t_printhook) libpd_printhook;
  sys_soundin = NULL;
  sys_soundout = NULL;
  // are all these settings necessary?
  sys_schedblocksize = DEFDACBLKSIZE;
  sys_externalschedlib = 0;
  sys_printtostderr = 0;
  sys_usestdpath = 0; // don't use pd_extrapath, only sys_searchpath
  sys_debuglevel = 0;
  sys_verbose = 0;
  sys_noloadbang = 0;
  sys_nogui = 1;
  sys_hipriority = 0;
  sys_nmidiin = 0;
  sys_nmidiout = 0;
  sys_time = 0;
  pd_init();
  libpdreceive_setup();
  sys_set_audio_api(API_DUMMY);
  sys_searchpath = NULL;
}

void libpd_clear_search_path(void) {
  namelist_free(sys_searchpath);
  sys_searchpath = NULL;
}

void libpd_add_to_search_path(const char *s) {
  sys_searchpath = namelist_append(sys_searchpath, s, 0);
}

int libpd_init_audio(int inChans, int outChans, int sampleRate) {
  int indev[MAXAUDIOINDEV], inch[MAXAUDIOINDEV],
       outdev[MAXAUDIOOUTDEV], outch[MAXAUDIOOUTDEV];
  indev[0] = outdev[0] = DEFAULTAUDIODEV;
  inch[0] = inChans;
  outch[0] = outChans;
  sys_set_audio_settings(1, indev, 1, inch,
         1, outdev, 1, outch, sampleRate, -1, 1, DEFDACBLKSIZE);
  sched_set_using_audio(SCHED_AUDIO_CALLBACK);
  sys_reopen_audio();
  return 0;
}

int libpd_process_raw(const float *inBuffer, float *outBuffer) {
  size_t n_in = sys_inchannels * DEFDACBLKSIZE;
  size_t n_out = sys_outchannels * DEFDACBLKSIZE;
  t_sample *p;
  size_t i;
  for (p = sys_soundin, i = 0; i < n_in; i++) {
    *p++ = *inBuffer++;
  }
  memset(sys_soundout, 0, n_out * sizeof(t_sample));
  sched_tick(sys_time + sys_time_per_dsp_tick);
  for (p = sys_soundout, i = 0; i < n_out; i++) {
    *outBuffer++ = *p++;
  }
  return 0;
}

static const t_sample sample_to_short = SHRT_MAX,
                   short_to_sample = 1.0 / (t_sample) SHRT_MAX;

#define PROCESS(_x, _y) \
  int i, j, k; \
  t_sample *p0, *p1; \
  for (i = 0; i < ticks; i++) { \
    for (j = 0, p0 = sys_soundin; j < DEFDACBLKSIZE; j++, p0++) { \
      for (k = 0, p1 = p0; k < sys_inchannels; k++, p1 += DEFDACBLKSIZE) { \
        *p1 = *inBuffer++ _x; \
      } \
    } \
    memset(sys_soundout, 0, sys_outchannels*DEFDACBLKSIZE*sizeof(t_sample)); \
    sched_tick(sys_time + sys_time_per_dsp_tick); \
    for (j = 0, p0 = sys_soundout; j < DEFDACBLKSIZE; j++, p0++) { \
      for (k = 0, p1 = p0; k < sys_outchannels; k++, p1 += DEFDACBLKSIZE) { \
        *outBuffer++ = *p1 _y; \
      } \
    } \
  } \
  return 0;

int libpd_process_short(int ticks, const short *inBuffer, short *outBuffer) {
  PROCESS(* short_to_sample, * sample_to_short)
}

int libpd_process_float(int ticks, const float *inBuffer, float *outBuffer) {
  PROCESS(,)
}

int libpd_process_double(int ticks, const double *inBuffer, double *outBuffer) {
  PROCESS(,)
}
 
#define GETARRAY \
  t_garray *garray = (t_garray *) pd_findbyclass(gensym(name), garray_class); \
  if (!garray) return -1; \

int libpd_arraysize(const char *name) {
  GETARRAY
  return garray_npoints(garray);
}

#define MEMCPY(_x, _y) \
  GETARRAY \
  if (n < 0 || offset < 0 || offset + n > garray_npoints(garray)) return -2; \
  t_word *vec = ((t_word *) garray_vec(garray)) + offset; \
  int i; \
  for (i = 0; i < n; i++) _x = _y;

int libpd_read_array(float *dest, const char *name, int offset, int n) {
  MEMCPY(*dest++, (vec++)->w_float)
  return 0;
}

int libpd_write_array(const char *name, int offset, float *src, int n) {
  MEMCPY((vec++)->w_float, *src++)
  return 0;
}

void libpd_set_float(t_atom *v, float x) {
  SETFLOAT(v, x);
}

void libpd_set_symbol(t_atom *v, const char *sym) {
  SETSYMBOL(v, gensym(sym));
}

int libpd_list(const char *recv, int n, t_atom *v) {
  t_pd *dest = get_object(recv);
  if (dest == NULL) return -1;
  pd_list(dest, &s_list, n, v);
  return 0;
}

int libpd_message(const char *recv, const char *msg, int n, t_atom *v) {
  t_pd *dest = get_object(recv);
  if (dest == NULL) return -1;
  pd_typedmess(dest, gensym(msg), n, v);
  return 0;
}

int libpd_start_message(int max_length) {
  if (max_length > argm) {
    t_atom *v = realloc(argv, max_length * sizeof(t_atom));
    if (v) {
      argv = v;
      argm = max_length;
    } else {
      return -1;
    }
  }
  argc = 0;
  curr = argv;
  return 0;
}

#define ADD_ARG(f) f(curr, x); curr++; argc++;

void libpd_add_float(float x) {
  ADD_ARG(SETFLOAT);
}

void libpd_add_symbol(const char *s) {
  t_symbol *x = gensym(s);
  ADD_ARG(SETSYMBOL);
}

int libpd_finish_list(const char *recv) {
  return libpd_list(recv, argc, argv);
}

int libpd_finish_message(const char *recv, const char *msg) {
  return libpd_message(recv, msg, argc, argv);
}

void *libpd_bind(const char *sym) {
  return libpdreceive_new(gensym(sym));
}

void libpd_unbind(void *p) {
  pd_free((t_pd *)p);
}

int libpd_symbol(const char *recv, const char *sym) {
  void *obj = get_object(recv);
  if (obj == NULL) return -1;
  pd_symbol(obj, gensym(sym));
  return 0;
}

int libpd_float(const char *recv, float x) {
  void *obj = get_object(recv);
  if (obj == NULL) return -1;
  pd_float(obj, x);
  return 0;
}

int libpd_bang(const char *recv) {
  void *obj = get_object(recv);
  if (obj == NULL) return -1;
  pd_bang(obj);
  return 0;
}
/* 
 * Begining of added functions, to solve the 
 * hooks being in different parts of program
 * memory.
 *
void libpd_set_printhook ( const t_libpd_printhook hook )
{
    libpd_printhook = hook;
}
void libpd_set_banghook ( const t_libpd_banghook hook )
{
    libpd_banghook = hook;
}
void libpd_set_floathook ( const t_libpd_floathook hook )
{
    libpd_floathook = hook;
}
void libpd_set_symbolhook ( const t_libpd_symbolhook hook )
{
    libpd_symbolhook = hook;
}
void libpd_set_listhook ( const t_libpd_listhook hook )
{
    libpd_listhook = hook;
}
void libpd_set_messagehook ( const t_libpd_messagehook hook )
{
    libpd_messagehook = hook;
}
End of added functions. */
int libpd_blocksize(void) {
  return DEFDACBLKSIZE;
}

int libpd_exists(const char *sym) {
  return get_object(sym) != NULL;
}

#define CHECK_CHANNEL if (channel < 0) return -1;
#define CHECK_PORT if (port < 0 || port > 0x0fff) return -1;
#define CHECK_RANGE_7BIT(v) if (v < 0 || v > 0x7f) return -1;
#define CHECK_RANGE_8BIT(v) if (v < 0 || v > 0xff) return -1;
#define PORT (channel >> 4)
#define CHANNEL (channel & 0x0f)

int libpd_noteon(int channel, int pitch, int velocity) {
  CHECK_CHANNEL
  CHECK_RANGE_7BIT(pitch)
  CHECK_RANGE_7BIT(velocity)
  inmidi_noteon(PORT, CHANNEL, pitch, velocity);
  return 0;
}

int libpd_controlchange(int channel, int controller, int value) {
  CHECK_CHANNEL
  CHECK_RANGE_7BIT(controller)
  CHECK_RANGE_7BIT(value)
  inmidi_controlchange(PORT, CHANNEL, controller, value);
  return 0;
}

int libpd_programchange(int channel, int value) {
  CHECK_CHANNEL
  CHECK_RANGE_7BIT(value)
  inmidi_programchange(PORT, CHANNEL, value);
  return 0;
}

int libpd_pitchbend(int channel, int value) {
  CHECK_CHANNEL
  if (value < -8192 || value > 8191) return -1;
  inmidi_pitchbend(PORT, CHANNEL, value + 8192);
  // Note: For consistency with Pd, we center the output of [pitchin]
  // at 8192.
  return 0;
}

int libpd_aftertouch(int channel, int value) {
  CHECK_CHANNEL
  CHECK_RANGE_7BIT(value)
  inmidi_aftertouch(PORT, CHANNEL, value);
  return 0;
}

int libpd_polyaftertouch(int channel, int pitch, int value) {
  CHECK_CHANNEL
  CHECK_RANGE_7BIT(pitch)
  CHECK_RANGE_7BIT(value)
  inmidi_polyaftertouch(PORT, CHANNEL, pitch, value);
  return 0;
}

int libpd_midibyte(int port, int byte) {
  CHECK_PORT
  CHECK_RANGE_8BIT(byte)
  inmidi_byte(port, byte);
  return 0;
}

int libpd_sysex(int port, int byte) {
  CHECK_PORT
  CHECK_RANGE_7BIT(byte)
  inmidi_sysex(port, byte);
  return 0;
}

int libpd_sysrealtime(int port, int byte) {
  CHECK_PORT
  CHECK_RANGE_8BIT(byte)
  inmidi_realtimein(port, byte);
  return 0;
}

void *libpd_openfile(const char *basename, const char *dirname) {
  return (void *)glob_evalfile(NULL, gensym(basename), gensym(dirname));
}

void libpd_closefile(void *x) {
  pd_free((t_pd *)x);
}

int libpd_getdollarzero(void *x) {
  pd_pushsym((t_pd *)x);
  int dzero = canvas_getdollarzero();
  pd_popsym((t_pd *)x);
  return dzero;
}
