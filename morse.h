#ifndef MORSE_H
#define MORSE_H

/* imports */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* constants */
#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define BUFF_SIZE 8192
#define FILENAME "morse.wav"
#define PAD 10
#define LETTER_PAD 1.5

/* audio structs */
typedef struct _formatchunk
{
    unsigned char   ID[4];
    unsigned int    size;
    unsigned short  compression;
    unsigned short  channels;
    unsigned int    sampleRate;
    unsigned int    byteRate;
    unsigned short  blockAlign;
    unsigned short  bitsPerSample;
} formatchunk;

typedef struct _datachunk
{
    unsigned char   ID[4];
    unsigned int    size;
} datachunk;

typedef struct _waveheader
{
    unsigned char   ID[4];
    unsigned int    size;
    unsigned char   format[4];
    formatchunk     formatChunk;
    datachunk       dataChunk;
} waveheader;

typedef struct _audiodata
{
    waveheader  header;
    short       *lchannel;
    short       *rchannel;
    int         size;
} audiodata;

/* function prototypes */
audiodata *create_file(char *morse, audiodata *dash, audiodata *dot);
unsigned int get_new_size(char *morse, unsigned int dashSize, unsigned int dotSize);
char *ascii_to_morse(char *phrase);
void read_file(audiodata *audio, char *filename);
void write_file(const audiodata *audio, char *filename);
void read_line(char *buffer);

#endif
