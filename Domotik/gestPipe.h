/* 
 * File:   gestPipe.h
 * Author: Gilles
 */
#ifndef GESTPIPE_H
#define GESTPIPE_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "structEch.h"
#include "Log.h"

class gestPipe {
    int fe, fs;
    sEch linfo;
    char pipE[50], pipS[50];
    Log *pLog;
    
public:
    gestPipe();
    void getAdLog(Log*);
    void writePipeSort(sEch*);
    void readPipeEnt(sEch*);
    void writePipeEnt(sEch*);
    void readPipeSort(sEch*);
};
#endif /* GESTPIPE_H */

