/*
 * Copyright (c) 2019 Gaurav Kumar Yadav <gaurav712@protonmail.com>
 * For licence and copyright information, see LICENCE file that was distributed with the source
 *
 * A subtitle(*.srt) parser
 * 12:55, 29 March, 2019
 *
 * In case you're wondering what does an *.srt file look like(or what is it) :
 *
 * WHAT IS IT?
 * It's the most popular form of file for loading external subtitles in movies/videos.
 *
 * WHAT DOES IT LOOK LIKE?
 *      _________________________________
 *      |N                              |
 *      |HH:MM:SS,m_s -> HH:MM:SS,m_s   |
 *      |blah blah blah....             |
 *      |...                            |
 *      |...                            |
 *      |                               |
 *      ---------------------------------
 *      'N' is a natural number representing dialogue number
 *      "HH:MM:SS,m_s -> HH:MM:SS,m_s" represents the duration of dialogue 'N'. "m_s" is milliseconds, it's usually 3 digits hence '_'.
 *      "blah blah blah...." is the dialogue.
 *      "..." says "and so on...", i.e  every dialogue will be of the above form.
 *
 * NOTE :
 * Text formatting(bold, italics and stuff) not supported(to make this shit fast, lean most importantly)!
 * Not for video players. It's just a personal fun project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CHAR_IN_LINE 200 /* Maximum num of chars for a single line of dialogue */

struct Time {
    unsigned short hours, minutes, seconds, milliSeconds;
};

/* Read the duration of dialogue and delay after it(as people in movies don't always speak) */
static void getDurationAndDelay(FILE *fp, struct timespec *duration, struct timespec *delay, struct Time *prevDialEndTime);

int main(int argc, char *args[]){

    FILE *fp;
    short shouldExit = 0;
    char ch, dialogueLine[MAX_CHAR_IN_LINE];
    /* Who references structs directly, huh? */
    struct timespec durationVar, delayVar, *duration = &durationVar, *delay = &delayVar;
    struct Time prevDialEndTimeVar, *prevDialEndTime = &prevDialEndTimeVar; /* I need prevDi*** to calculated delay b/w dialogues */

    /* Check for command line arguments */
    if(argc != 2) {
        /* Show help to that moron! */
        fprintf(stderr, "\nUSAGE:\n\tsubtitle_parser [filename]\n\n");
        exit(1);
    }

    /* Opening file for reading */
    if((fp = fopen(args[1], "r")) == NULL) {
        fprintf(stderr, "Error opening file \"%s\"!\n", args[1]);
        exit(1);
    }

    /*
     * Section for setting up dialogue-number details
     * If I ever want to do that
     */

    /* Set all elements in prevDialEndTime to 0 for the first dialogue */
    prevDialEndTime->hours = 0;
    prevDialEndTime->minutes = 0;
    prevDialEndTime->seconds = 0;
    prevDialEndTime->milliSeconds = 0;

    /* Here comes the actual stuff */
    while(1) {

        /* As we aren't doing anything with dialogue numbers, let's just skip that*/
        while((getc(fp)) != '\n');

        /* Calculate delay before dialogue and the duration it stays for */
        getDurationAndDelay(fp, duration, delay, prevDialEndTime);

        /* Sleeping when there's no voice(text, to be honest) */
        nanosleep(delay, NULL);

        /* Displaying the dialogue */

        /* Just to separate it from the previous dialogue */
        putchar('\n');

        while(1) {

            if(fgets(dialogueLine, MAX_CHAR_IN_LINE, fp) == NULL) {
                shouldExit = 1;
                break;
            }

            printf("%s", dialogueLine);

            /* Check if dialogue ended. "13" says 0x0d('\n' in dos encondings, it has 0x0a as well but anyway) */
            if((ch = fgetc(fp)) == 13 || ch == '\n') {
                while((ch = fgetc(fp)) < '0' || ch > '9');
                ungetc(ch, fp);
                break;
            } else {
                ungetc(ch, fp);
            }
        }

        /* Sleep to hold the dialogue for 'duration' */
        nanosleep(duration, NULL);

        if(shouldExit)
            break;
    }

    /* Closing the file */
    fclose(fp);

    return 0;
}

void
getDurationAndDelay(FILE *fp, struct timespec *duration, struct timespec *delay, struct Time *prevDialEndTime) {

    unsigned char ch;
    struct Time startTime, endTime;

    /* Read start time */
    fscanf(fp, "%hd:%hd:%hd,%hd", &startTime.hours, &startTime.minutes, &startTime.seconds, &startTime.milliSeconds);

    /* Skip "-->" */
    while((ch = fgetc(fp)) != '>');
    ch = fgetc(fp);

    /* Read end time */
    fscanf(fp, "%hd:%hd:%hd,%hd\n", &endTime.hours, &endTime.minutes, &endTime.seconds, &endTime.milliSeconds);

    /* Calculate delay before the dialogue */
    delay->tv_sec = (((((startTime.hours - prevDialEndTime->hours) * 60) + (startTime.minutes - prevDialEndTime->minutes)) * 60) + (startTime.seconds - prevDialEndTime->seconds));
    delay->tv_nsec = (startTime.milliSeconds - prevDialEndTime->milliSeconds) * 1000000;

    if(delay->tv_nsec < 0) { /* Time can't be negative mate */
        delay->tv_sec--;
        delay->tv_nsec = 1000000000 + delay->tv_nsec;
    }

    /* Calculate duration of the dialogue */
    duration->tv_sec = (((((endTime.hours - startTime.hours) * 60) + (endTime.minutes - startTime.minutes)) * 60) + (endTime.seconds - startTime.seconds));
    duration->tv_nsec = (endTime.milliSeconds - startTime.milliSeconds) * 1000000;

    if(duration->tv_nsec < 0) { /* As I said, time can't be negative */
        duration->tv_sec--;
        duration->tv_nsec = 1000000000 + duration->tv_nsec;
    }

    /* Set prevDialEndTime for next iteration */
    prevDialEndTime->hours = endTime.hours;
    prevDialEndTime->minutes = endTime.minutes;
    prevDialEndTime->seconds = endTime.seconds;
    prevDialEndTime->milliSeconds = endTime.milliSeconds;
}

