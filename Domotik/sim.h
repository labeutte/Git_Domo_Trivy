/**
 * File:   SIM.h
 * Author: aucaigne
 */
#ifndef SIM_H
#define SIM_H
#include <iostream>
#include <string>     // std::string, std::to_string
#include <iomanip>
#include <time.h>
#include <ctime>
#include <string.h> 
#include <pthread.h>
#include <vector>
#include <sstream> 
#include "RCSwitch.h"
#include "serieLin.h"
#include "accesBDD.h"
#include "internet.h"
#include "compress.h"
#include <list>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <cstring>
#include <mosquitto.h>
#include <tinyxml2.h>
#include "Log.h"

using namespace std;
using namespace tinyxml2;

struct infoXms {
    unsigned char type; //0=SMS, 1=MMS, 2=SMS avec 1 seul destinat
    string message;
    string nomIm;
    string chemIm;
    int groupe;
    string destin;
};
typedef struct infoXms stXm;

class SIM {
private:
    Log *pLog;
    XMLDocument xmlDoc;
    XMLElement *element, *item;
    XMLError eResult;
    string stg;
    //    const char attribText[50];
    serieLin maCom, maCom2;
    int fdPort, fdPort2;
    string nomPortSerie, nomPortSerie2;
    internet reseau;
    accesBDD* adbdd;
    compress *comp;
    int etat;
    int cle3G;
    list <stXm> listXMS;
    std::time_t tps, tpsSMS;
    char my_phone_number[14];
    char mmsAPN[20];
    char mmsCentre[30];
    char mmsCentreH[30];
    char mmsCentreIP[30];
    char mmsProxy[17];
    char mdpSms[6];
    char ipRelaiESP[17];
    unsigned int mmsPort;
    char aux_string[356];
    char textSms[145];
    unsigned char imageMMS[308000]; //taille maxi d'un MMS
    char textMMS[15360];
    string contact[10];
    string numReqSms;
    struct timeval tv;
    pthread_mutex_t* myMutex;
    string dt;
    bool etatReseau;
    struct mosquitto *leMosq;
    string topicEmiss;
    std::ostringstream ostst;

public:
    SIM();
    virtual ~SIM();
    void getAdLog(Log*);
    void setAdLog();
    string dateL();
    void setNomPort(string, string);
    string getNumReqSms();
    void setAdMutex(pthread_mutex_t*);
    void setAdMosquito(struct mosquitto*, string);
    int ouvertureSerie(string);
    int ouvertureSerie2(string);
    int envSMScurl(string, string);
    int envSMS(string, string);
    int envMMS(string, string, string, string);
    void recepSMS();
    void diffSMS(string, string);
    void diffMMS(string, string, string, string);
    void envImgFtp(string);
    void setAdBDD(accesBDD*);
    void traitRecepSms(string);
    void SendXMS();
    void pushListXms(stXm);
    void repositRelai(int);
    int pingOK(char*);
    string nivGsm();
};
#endif /* SIM_H */
