/* 
 * File:   Alarme433.h
 */
#ifndef ALARME433_H
#define ALARME433_H
#include <iostream>
#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <tinyxml2.h>
#include <wiringPi.h>
#include <semaphore.h>
#include "Log.h"

using namespace tinyxml2;
using namespace std;

class alarme433 {
    Log *pLog;
    short autorizRecep;
    RCSwitch leSwitch;
    short basc;
    int PinAlim; //alimente la carte d'emission
    int PinTX; //Pin Data
    int PinRelai;
    RCSwitch mySwitch;
    XMLDocument xmlDoc;
    XMLElement *element, *item;
    XMLError eResult;
    int fMarche;
    int fArret;
    char bufMAR[20], bufARR[20];
    const char* BUF;

public:
    alarme433();
    void getAdLog(Log*);
    void sonore();
    short getautorize();
    bool emmiss(int);
};
#endif /* ALARME433_H */

