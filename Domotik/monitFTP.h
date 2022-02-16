/* 
 * File:   monitFTP.h
 */
#ifndef MONITFTP_H
#define MONITFTP_H
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <dirent.h>
#include <vector>
#include <semaphore.h>
#include "accesBDD.h"
#include "sim.h"
#include "gestPipe.h"
#include "alarme433.h"
#include "structEch.h"
#include "Log.h"

using namespace std;

class monitFTP {
    Log *pLog;
    sem_t *semIntrus;
    std::vector<std::string> adetruire;
    char repOrigine[512];
    char repDestination[512];
    string posGeo;
    string nomCam;
    std::time_t tpsMms;
    SIM* maSim;
    accesBDD* databdd;
    gestPipe* gpd;
    alarme433* alarme;
    bool trouvCam;
    sEch ech;

public:
    monitFTP();
    virtual ~monitFTP();
    void getAdLog(Log*);
    void setAdSem(sem_t*);
    void info(char*);
    string dateL();
    void metScan(string, string);
    void traitFic(char*, char*);
    void traitRep(char*);
    void copier_fichier(char const *, char const *);
    void effaceRep();
    void effaceRepDeb(const char*);
    void setAdSim(SIM*);
    void setAdBdd(accesBDD*);
    void setAdGpd(gestPipe*);
    void setAdAlarme(alarme433*);
    //string getCheminMonit();
};

#endif /* MONITFTP_H */

