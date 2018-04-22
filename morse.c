/****************************************************
*
*   Project Name:   morse
*   Decription:     Turns an ascii phrase into a morse code string, then generates a morse code wav file
*   Filenames:      morse.c, morse.h, dash.wav, dot.wav
*   Author:         rory
*
****************************************************/

/* imports */
#include "morse.h"

/*
    main:       main function of the program

    params:     none

    returns:    0 - clean exit
*/
int main()
{
    audiodata *dash = malloc(sizeof(audiodata)); // struct to hold wav data of a dash
    audiodata *dot = malloc(sizeof(audiodata)); // struct to hold wav data of a dot
    audiodata *audio; // struct to hold the morse wave file
    char buffer[BUFF_SIZE];
    char *morse;

    read_file(dash, "dash.wav"); // read in dash wav file
    read_file(dot, "dot.wav"); // read in dot wav file

    printf("Enter phrase: "); // ask for user input
    read_line(buffer); // read in user input

    morse = ascii2morse(buffer); // convert user input to morse
    printf("%s\n", morse); // print out the converted phrase as dashes and dots (morse code)

    audio = create_file(morse, dash, dot); // create the morse audio file
    write_file(audio, FILENAME); // write the morse audio file (with filename "morse.wav")

    free(dash); // free dash
    free(dot); // free dot
    free(audio); // free audio

    return 0; // clean exit
}

/*
    create_file:    creates the morse wav file

    params:         *morse - the morse string that will be looped through
                    *dash - a pointer to the dash audio struct, we copy the dash audio from this
                    *dot - a pointer to the dot audio struct, we copy the dot audio from this

    returns:        audio - a pointer to the morse wav file
*/
audiodata *create_file(char *morse, audiodata *dash, audiodata *dot)
{
    audiodata *audio = malloc(sizeof(audiodata)); // allocate struct to hold the morse code of the phrase being passed in
    memcpy(audio, dash, sizeof(waveheader)); // copy over waveheader info from dash, as most of it will be the same, so then we just update the size (see code below)
    audio->header.dataChunk.size = get_new_size(morse, dash->header.dataChunk.size, dot->header.dataChunk.size); // update dataChunk size (utilizes GET_NEW_SIZE)
    audio->header.size = (dash->header.size - dash->header.dataChunk.size) + audio->header.dataChunk.size; // the header size will be the header size of dash, minus the dataChunk size of dash, plus the new dataChunk size
    audio->size = audio->header.dataChunk.size / 4; // update channel size

    audio->lchannel = (short *)malloc(sizeof(short) * audio->size); // allocate space for left channel
    audio->rchannel = (short *)malloc(sizeof(short) * audio->size); // allocate space for right channel

    int i = 0; // index for morse string
    int j = 0; // index for audio channels
    int k = 0; // index for dot and dash
    while (morse[i] != '\0') { // loop through the entire morse string
        if (morse[i] == '.') { // if the character is a '.'...
            // copy over audio of dot wav to morse wav, adjust j properly
            for (k = 0; k < dot->size; k++) {
                audio->lchannel[j] = dot->lchannel[k];
                audio->rchannel[j] = dot->rchannel[k];
                j++;
            }
            // pad end of dot with zeroes, so audio doesn't run together, adjust j properly
            for (k = 0; k < dash->size / PAD; k++) {
                audio->lchannel[j] = 0;
                audio->rchannel[j] = 0;
                j++;
            }
        } else if (morse[i] == '-') { // if the character is a '-'...
            // copy over audio of dash wav to morse wav, adjust j properly
            for (k = 0; k < dash->size; k++) {
                audio->lchannel[j] = dash->lchannel[k];
                audio->rchannel[j] = dash->rchannel[k];
                j++;
            }
            // pad end of dash with zeroes, so audio doesn't run together, adjust j properly
            for (k = 0; k < dash->size / PAD; k++) {
                audio->lchannel[j] = 0;
                audio->rchannel[j] = 0;
                j++;
            }
        } else if (morse[i] == '/') { // if the character is a '/' (word seperation)...
            // pad this part of the array with zeroes (the same length as a dash)
            for (k = 0; k < dash->size; k++) {
                audio->lchannel[j] = 0;
                audio->rchannel[j] = 0;
                j++;
            }
        } else if (morse[i] == ' ') { // if the character is a ' ' (letter seperation in same word)...
            // pad space between letters with zeroes (same length as dash->size / LETTER_PAD) (LETTER_PAD = 1.5)
            for (k = 0; k < dash->size / LETTER_PAD; k++) {
                audio->lchannel[j] = 0;
                audio->rchannel[j] = 0;
                j++;
            }
        }
        i++; // increment i
    }
    return audio; // return the morse audio
}

/*
    get_new_size:   runs through morse string and finds out the dataChunk size that will be needed for the morse wav

    params:         *morse - the morse string that will be looped through
                    dashSize - dataChunk size of dash wav file
                    dotSize - dataChunk size of dot wav file

    returns:        size - the datChunk size that the morse wav file will have to be
*/
unsigned int get_new_size(char *morse, unsigned int dashSize, unsigned int dotSize)
{
    int i = 0; // index
    unsigned int size = 0; // keep track of total size
    while (morse[i] != '\0') { // loop through the entire morse string (essentially, we want to count how many dots and dashes we have (in a roundabout way))
        // adjust size appropriately for a dot, dash, '/', or a space
        // we need some extra space than just the size of the dot or dash, as there will be some empty space between each dot and dash
        if (morse[i] == '.') size += (dotSize + (dashSize / 10)); // space for a dot
        else if (morse[i] == '-') size += (dashSize + (dashSize / 10)); // space for a dash
        else if (morse[i] == '/') size += dashSize; // space for seperation of words
        else if (morse[i] == ' ') size += (dashSize / 1.5); // space for seperation of letters in same word
        i++; // inrement i
    }
    return size; // return the total size that will be needed
}

/*
    ascii_to_morse: converts alphabetic ascii characters to morse code (dots, dashes, slashes) (., -, /)

    params:         *phrase - the ascii phrase to be converted

    returns:        *morse - a string in morse code
*/
char *ascii2morse(char *phrase)
{
    // morsetable holds the morse values of each alphabetic ascii character, 'a' at index 0, 'z' at index 25
    char morsetable[26][5] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."};
    char *morse = calloc(BUFF_SIZE, 8); // allocate space for morse, setting it to zeroes, as it will be a string, will need to be the size of the buffer * 8, which accounts for null characters, length of morse code conversions, spaces
    int i = 0; // index
    int value = 0; // value to index into morsetable
    while (phrase[i] != 0) { // loop through until the end of the string
        int c = tolower(phrase[i]); // just convert everything to lower, capitalization doesn't matter in morse
        if ( c >= 'a' && c <= 'z') { // if the character is alphabetic
            value = c - 'a'; // calculate the correct value
            strcat(morse, morsetable[value]); // index into morsetable, and concatenate that value onto morse
        } else if (c == ' ') { // other wise, if there is a space
            strcat(morse, "/"); // concatenate a '/' onto morse
        }
        strcat(morse, " "); // always concatenate a space after each morse code
        i++; // increment i
    }
    return morse; // return morse
}

/*
    read_file:  reads a wav file

    params:     *audio - audiodata struct to store the wav file in
                *filename - name of the file to be read in

    returns:    void
*/
void read_file(audiodata *audio, char *filename)
{
    FILE *file = fopen(filename, "r"); // open the file
    if (file == NULL) { // if the file is null...
        fprintf(stderr, "Could not open file: %s.\n", filename); // print msg to user
        exit(1); // quit the program
    }
    fread(audio, sizeof(waveheader), 1, file); // read in waveheader
    audio->size = (audio->header.dataChunk.size / 4); // the length of each channel array
    audio->lchannel = (short *)(malloc(sizeof(short) * audio->size)); // allocate space for left channel
    audio->rchannel = (short *)(malloc(sizeof(short) * audio->size)); // allocate space for right channel
    int i; // index
    for (i = 0; i < audio->size; i++) { // loop through each channel and read in the values
        audio->lchannel[i] = fgetc(file) | (fgetc(file) << 8);
        audio->rchannel[i] = fgetc(file) | (fgetc(file) << 8);
    }
    fclose(file); // close the file
}

/*
    write_file: writes a wav file

    params:     *audio - audiodata struct that holds the audio and file info to be written
                *filename - name of the file to be written to

    return:     void
*/
void write_file(const audiodata *audio, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) { // if the file is null...
        fprintf(stderr, "Could not save file: %s.\n", filename); // print msg to user
        exit(1); // quit the program
    }
    fwrite(audio, sizeof(waveheader), 1, file); // write the waveheader
    unsigned char temp; // temp variable
    int i; // index
    for (i = 0; i < audio->size; i++) { // loop through each audio channel
        // lchannel operations
        temp = audio->lchannel[i] >> 8;
        fputc((unsigned char)(audio->lchannel[i]), file);
        fputc(temp, file);

        // rchannel operations
        temp = audio->rchannel[i] >> 8;
        fputc((unsigned char)(audio->rchannel[i]), file);
        fputc(temp, file);
    }
    // free(audio->lchannel); // free lchannel
    // free(audio->rchannel); // free rchannel
    fclose(file); // close the file
}

/*
    read_line:  reads text from stdin into buffer, deliminated by a newline ('\n')

    params:     *buffer - string to store text in

    returns:    void
*/
void read_line(char *buffer)
{
    int c = 0;
	int i = 0;
	while( (c = getchar()) != '\n' ) buffer[i++] = c;
	buffer[i] = '\0';
}
