#ifndef PRIV_TYPES_H
#define PRIV_TYPES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#define GRANULE_SIZE  576

#include "bitstream.h"

/* TODO: Assembler code did not seem to like
 * moving all static variables to allocated struct..
 * Needs fixing, so disabled for now. However, it does
 * not seem to make much of a difference anyway..
 
#if defined(__arm__)
#include "mult_sarm_gcc.h"
#else
*/

#include "mult_noarch_gcc.h"

/* #endif */

/* #define DEBUG if you want the library to dump info to stdout */

#define PI          3.14159265358979
#define PI4         0.78539816339745
#define PI12        0.26179938779915
#define PI36        0.087266462599717
#define PI64        0.049087385212
#define SQRT2       1.41421356237
#define LN2         0.69314718
#define LN_TO_LOG10 0.2302585093
#define BLKSIZE     1024
#define HAN_SIZE    512 /* for loop unrolling, require that HAN_SIZE%8==0 */
#define SCALE_BLOCK 12
#define SCALE_RANGE 64
#define SCALE       32768
#define SBLIMIT     32

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 2
#endif

#ifndef MAX_GRANULES
#define MAX_GRANULES 2
#endif

/*
  A BitstreamElement contains encoded data
  to be written to the bitstream.
  'length' bits of 'value' will be written to
  the bitstream msb-first.
*/
typedef struct {
    unsigned long int value;
    unsigned int length;
} BF_BitstreamElement;

/*
  A BitstreamPart contains a group
  of 'nrEntries' of BitstreamElements.
  Each BitstreamElement will be written
  to the bitstream in the order it appears
  in the 'element' array.
*/
typedef struct {
    unsigned long int nrEntries;
    BF_BitstreamElement *element;
} BF_BitstreamPart;

/*
  This structure contains all the information needed by the
  bitstream formatter to encode one frame of data. You must
  fill this out and provide a pointer to it when you call
  the formatter.
  Maintainers: If you add or remove part of the side
  information, you will have to update the routines that
  make local copies of that information (in formatBitstream.c)
*/

typedef struct BF_FrameData {
    BF_BitstreamPart *header;
    BF_BitstreamPart *frameSI;
    BF_BitstreamPart *channelSI[MAX_CHANNELS];
    BF_BitstreamPart *spectrumSI[MAX_GRANULES][MAX_CHANNELS];
    BF_BitstreamPart *scaleFactors[MAX_GRANULES][MAX_CHANNELS];
    BF_BitstreamPart *codedData[MAX_GRANULES][MAX_CHANNELS];
    BF_BitstreamPart *userSpectrum[MAX_GRANULES][MAX_CHANNELS];
    BF_BitstreamPart *userFrameData;
} BF_FrameData;

typedef struct BF_PartHolder {
    int max_elements;
    BF_BitstreamPart *part;
} BF_PartHolder;

typedef struct {
    int  channels;
    long samplerate;
} priv_shine_wave_t;

typedef struct {
    int    version;
    int    layer;
    int    granules_per_frame;
    int    mode;      /* + */ /* Stereo mode */
    int    bitr;      /* + */ /* Must conform to known bitrate - see Main.c */
    int    emph;      /* + */ /* De-emphasis */
    int    padding;
    long   bits_per_frame;
    long   bits_per_slot;
    double frac_slots_per_frame;
    double slot_lag;
    int    whole_slots_per_frame;
    int    bitrate_index;     /* + */ /* See Main.c and Layer3.c */
    int    samplerate_index;  /* + */ /* See Main.c and Layer3.c */
    int    crc;
    int    ext;
    int    mode_ext;
    int    copyright;  /* + */
    int    original;   /* + */
} priv_shine_mpeg_t;

typedef struct {
  BF_PartHolder *headerPH;
  BF_PartHolder *frameSIPH;
  BF_PartHolder *channelSIPH[MAX_CHANNELS];
  BF_PartHolder *spectrumSIPH[MAX_GRANULES][MAX_CHANNELS];
} MYSideInfo;

typedef struct {
    int        BitCount;
    int        BitsRemaining;
    MYSideInfo side_info;
} formatbits_t;

typedef struct {
  BF_FrameData   frameData;

  BF_PartHolder *headerPH;
  BF_PartHolder *frameSIPH;
  BF_PartHolder *channelSIPH[ MAX_CHANNELS ];
  BF_PartHolder *spectrumSIPH[ MAX_GRANULES ][ MAX_CHANNELS ];
  BF_PartHolder *scaleFactorsPH[ MAX_GRANULES ][ MAX_CHANNELS ];
  BF_PartHolder *codedDataPH[ MAX_GRANULES ][ MAX_CHANNELS ];
  BF_PartHolder *userSpectrumPH[ MAX_GRANULES ][ MAX_CHANNELS ];
  BF_PartHolder *userFrameDataPH;
} l3stream_t;

typedef struct {
  long *xr;                    /* magnitudes of the spectral values */
  long xrsq[GRANULE_SIZE];     /* xr squared */
  long xrabs[GRANULE_SIZE];    /* xr absolute */
  long xrmax;                  /* maximum of xrabs array */
  long en_tot[MAX_GRANULES];   /* gr */
  long en[MAX_GRANULES][21];
  long xm[MAX_GRANULES][21];
  long xrmaxl[MAX_GRANULES];
  double steptab[128]; /* 2**(-x/4)  for x = -127..0 */
  long steptabi[128];  /* 2**(-x/4)  for x = -127..0 */
  long int2idx[10000]; /* x**(3/4)   for x = 0..9999 */
} l3loop_t;

typedef struct {
  long ca[8];
  long cs[8];
  long cos_l[18][36];
} mdct_t;

typedef struct {
  int off[MAX_CHANNELS];
  long fl[SBLIMIT][64];
  long x[MAX_CHANNELS][HAN_SIZE];
  long z[MAX_CHANNELS][HAN_SIZE];
  long ew[HAN_SIZE];
} subband_t; 

/* Side information */
typedef struct {
        unsigned part2_3_length;
        unsigned big_values;
        unsigned count1;
        unsigned global_gain;
        unsigned scalefac_compress;
        unsigned table_select[3];
        unsigned region0_count;
        unsigned region1_count;
        unsigned preflag;
        unsigned scalefac_scale;
        unsigned count1table_select;

        unsigned part2_length;
        unsigned sfb_lmax;
        unsigned address1;
        unsigned address2;
        unsigned address3;
        int quantizerStepSize;
        unsigned slen[4];
} gr_info;

typedef struct {
    unsigned private_bits;
    int resvDrain;
    unsigned scfsi[MAX_CHANNELS][4];
    struct {
        struct {
            gr_info tt;
        } ch[MAX_CHANNELS];
    } gr[MAX_GRANULES];
} shine_side_info_t;

typedef struct {
    double  l[MAX_GRANULES][MAX_CHANNELS][21];
} shine_psy_ratio_t;

typedef struct {
    double  l[MAX_GRANULES][MAX_CHANNELS][21];
} shine_psy_xmin_t;

typedef struct {
    int l[MAX_GRANULES][MAX_CHANNELS][22];            /* [cb] */
    int s[MAX_GRANULES][MAX_CHANNELS][13][3];         /* [window][cb] */
} shine_scalefac_t;


typedef struct shine_global_flags { 
  priv_shine_wave_t    wave;
  priv_shine_mpeg_t    mpeg;
  bitstream_t    bs;
  shine_side_info_t side_info;
  int            sideinfo_len;
  int            mean_bits;
  shine_psy_ratio_t ratio;
  shine_scalefac_t  scalefactor;
  int16_t       *buffer[MAX_CHANNELS];
  double         pe[MAX_GRANULES][MAX_CHANNELS];
  int            l3_enc[MAX_GRANULES][MAX_CHANNELS][GRANULE_SIZE];
  long           l3_sb_sample[MAX_CHANNELS][MAX_GRANULES+1][18][SBLIMIT];
  long           mdct_freq[MAX_GRANULES][MAX_CHANNELS][GRANULE_SIZE];
  int            ResvSize;
  int            ResvMax;
  formatbits_t   formatbits;
  l3stream_t     l3stream;
  l3loop_t       l3loop;
  mdct_t         mdct;
  subband_t      subband;
} shine_global_config;

#endif
