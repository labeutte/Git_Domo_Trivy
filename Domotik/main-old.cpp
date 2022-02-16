/* 
 * File:  main.cpp
 * Author: Aucaigne
 * Version ci-dessous
 */
#define Debug 0
#define VERSION "26-01-2021" 

#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <mosquitto.h>
#include "RCSwitch.h"
#include "sim.h"
#include "accesBDD.h"
#include "monitFTP.h"
#include "gestPipe.h"
#include "alarme433.h"
#include "structEch.h"
#include "Log.h"
#include <sched.h>  /* for scheduling calls/constants */

#define MY_HIGH_PRIORITY 60
#define MY_LOW_PRIORITY 4

using namespace std;

/***PROTOTYPES***/
void traitement433(long unsigned);
void compareTemp(string, string);
void traitTemperature(float, int);
//void mqtt_Initialise(string host = MQTT_HOSTNAME, int port = MQTT_PORT, string nom = MQTT_USERNAME, string pwd = MQTT_PASSWORD);
void mqtt_Initialise(string, string, string, string);

/***GLOBAL***/
static pthread_mutex_t exMut;
string ttyPort1, ttyPort2;
static Log mlog;
accesBDD dataBase;
monitFTP myMonit;
SIM maSim;
RCSwitch* mySwitch;
fd_set fds;
//ft RCSwitch::pft; // = alert433;
FILE* flog;
int bascAlim = 0;
int etat = 1; // état du système
static int basc_a = 0;
std::time_t tpsRebond = 0, tempoCoupure=0;
struct mosquitto *objMosq = NULL;
//static sEch ssE;
sEch sd;
gestPipe *gpd;
static alarme433 Alarme;
static sem_t semK;
XMLDocument xmlDoc;
XMLElement *element, *item;
XMLError eResult;
char MQTT_HOSTNAME[50];
int MQTT_PORT;
char MQTT_USERNAME[20];
char MQTT_PASSWORD[20];
char MQTT_TOPIC_RECEP[50];
char MQTT_TOPIC_EMISS[50];
int fMarche;
int fArret;
int PinDefAlim = 21; //Repositionné par XML
int PinRF = 2;
int PinAlim = 31;
int alimSecours = 0;
int klaxFil = 0;
int em433 = 0;
int cle3G = 0;
int cay = 0;
string stg;
const char* attribText;
char attribT[30];
static short basc_teta[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //pour mémoriser alerte 
//sur capteur de température ; commun MQTT et 433
/*************/

/**
 * Retourne la date au format français
 */
string dateL() {
    const char* nomJourSem[] = {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
    const char* nomMois[] = {"janv", "fev", "mar", "avr", "mai", "juin", "juil", "aout", "sept", "oct", "nov", "dec"};

    char locTime[30];
    time_t TH;
    struct tm* t;
    TH = time(NULL);
    t = localtime(&TH);
    sprintf(locTime, "%3s %02u %s %04u %02u:%02u:%02u", nomJourSem[t->tm_wday], t->tm_mday, nomMois[t->tm_mon], 1900 + t->tm_year, t->tm_hour, t->tm_min, t->tm_sec);
    return (string(locTime));
}

/*
 * Callback sur reception d'un message MQTT 
 */
void mon_message_callback(struct mosquitto *objMosq, void *obj, const struct mosquitto_message *message) {
    string suj, ms;
    suj = string((char *) message->topic);
    ms = string((char *) message->payload);
    compareTemp(suj, ms);
}

void mqtt_Initialise(string host, int port, string nomUser, string pwd) {
    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto runtime instance with a random client ID,
    //  and no application-specific callback data.  
    objMosq = mosquitto_new(NULL, true, NULL);
    if (!objMosq) {
        if (Debug)
            fprintf(stderr, "Can't initialize Mosquitto library\n");
    }
    //	mosquitto_username_pw_set(objMosq, nomUser.c_str(), pwd.c_str());

    // Establish a connection to the MQTT server. Do not use a keep-alive ping
    int ret = mosquitto_connect(objMosq, host.c_str(), port, 0);
    //	int ret = mosquitto_connect(objMosq, host.c_str(), port, 60);
    if (ret) {
        if (Debug) {
            fprintf(stderr, "Can't connect to Mosquitto server\n");
            perror("Err Connexion Serveur ");
        }
    }
}

/**
 * Appelé sur reception d'un message mosquitto d'un capt de temp
 * @param top
 * @param mess
 */
void compareTemp(string top, string mess) {
    float temper, t_alert;
    int ID, p;
    string cod;
    stXm stX;
    std::ostringstream ostst;

    p = top.find("Capt");
    if (p > 0 && p < 8) {
        cod = top.substr(p + 4, p + 6);
        temper = stof(mess);
        ID = stoi(cod);

        if (Debug)
            cout << "\nmain.compareTemp - Temper recu (Mqtt) Capt:" << ID << " -> " << temper << endl;
        dataBase.majtemp(cod, mess); // maj temper de ce capt dans base

        if (dataBase.getstate() == 1 && ID >= 20 && ID <= 29) {
            ostst.str("");
            ostst << "Alert température, ID:" << ID << "   Temp:" << temper;
            mlog.wLine("\nmain.compareTemp", dateL(), ostst.str());
            traitTemperature(temper, ID);
        }
    }
}

/**
 * Thread de surveillance d'arrivee
 * d'une RF 433
 */
void* th_alert433(void*) {
    if (Debug) {
        printf("\033[33m->Thread (p8) alert433 lancé\033[00m\n");
        printf("\n\t PinRF=%d PinAlim=%d\n", PinRF, PinAlim);
    }
    if (wiringPiSetup() == -1 && Debug) {
        printf("Erreur wiring PI");
    }

    pinMode(PinAlim, OUTPUT);
    digitalWrite(PinAlim, LOW); //pour ne pas alimenter la carte d'emission

    mySwitch->enableReceive(PinRF);
    while (1) {
        if (mySwitch->available()) {
            int value = mySwitch->getReceivedValue();
            if (Debug)
                printf("\033[32m\nRecep RF 433 : %8X\033[00m\n", value);
            if (Alarme.getautorize()) {
                traitement433((unsigned long) value);
            }
            mySwitch->resetAvailable();
        }
        usleep(500000);
    }
    pthread_exit(NULL);
}

/**
 * Traitement sur capteur de température
 * @param teta
 * @param idTeta
 */
void traitTemperature(float teta, int idTeta) {
    float tp_alert;
    stXm stX;
    string texto = "";
    std::ostringstream ostst;

    dataBase.updatetemp(idTeta, teta);
    tp_alert = dataBase.gettempalert(idTeta);
    if (teta < tp_alert && basc_teta[idTeta - 20] == 0) {
        if (Debug)
            printf("\n%s : Alerte temp, Code: %d  Temp: %f\n\n", dateL().c_str(), idTeta, teta);
        stringstream ss;
        ss << tp_alert;
        string str = ss.str();
        texto = dataBase.getcaptname(idTeta);
        dataBase.alertcapt(dataBase.getidcapt(idTeta), texto);
        texto += ": Temp inferieur a ";
        texto += str;
        texto += " deg.\n ";
        texto += dateL();
        texto += '\0';

        stX.type = 0; //SMS
        stX.message = texto;
        stX.groupe = 1;
        maSim.pushListXms(stX);
        basc_teta[idTeta - 20] = 1;
    }
    if (teta >= tp_alert) {
        basc_teta[idTeta - 20] = 0;
    }
}

/**
 * Traitement quand il y a alerte intrusion (mouvement)
 * @param pc
 */
void traitIntrusion(unsigned long i_p, bool ibasc) {
    stXm stX;
    string texto = "";
    std::ostringstream ostst;
    if (ibasc == false) { //pour déclencher au 2ème coup
        ibasc = true;
    } else { //2eme = on déclenche
        ibasc = false;
        //cout << "time433 :" << std::time(0) << " - " << tpsRebond << endl;
        if (std::time(0) - tpsRebond > 300) { // On attend au minimum Xms (rebond)
            tpsRebond = std::time(0);
            if (Debug)
                printf("%s : Intrusion Capteur mouvement: %x\n\n", dateL().c_str(), i_p);
            ostst.str("");
            ostst << "Intrusion Capteur mouvement:" << std::hex << i_p;
            mlog.wLine("\ntraitement433", dateL(), ostst.str());
            // Envoi SMS
            texto = dataBase.getcaptname(i_p);
            dataBase.alertcapt(dataBase.getidcapt(i_p), texto);
            texto += ": MOUVEMENT! ";
            texto += dateL();
            texto += '\0';
            stX.type = 0; //SMS
            stX.message = texto;
            stX.groupe = 1;
            maSim.pushListXms(stX);
            //On déclenche Klaxon si emetteur 433
            if (em433 || klaxFil)
                sem_post(&semK); // V()
            //On publie sur Cayenne
            sd.type = 1;
            gpd->writePipeSort(&sd);
        }
    }
}

/**
 * Traitement quand il y a alerte ouverture
 * @param pc
 */
void traitOuverture(unsigned long i_p) {
    stXm stX;
    string texto = "";
    std::ostringstream ostst;
    if (Debug)
        printf("%s : Intrusion Capteur ouverture: %x\n\n", dateL().c_str(), i_p);
    ostst.str("");
    ostst << "Intrusion Capteur ouverture:" << std::hex << i_p;
    mlog.wLine("\ntraitement433", dateL(), ostst.str());
    // Envoi SMS
    texto = dataBase.getcaptname(i_p);
    dataBase.alertcapt(dataBase.getidcapt(i_p), texto);
    texto += ": OUVERTURE! ";
    texto += dateL();
    texto += '\0';
    stX.type = 0; //SMS
    stX.message = texto;
    stX.groupe = 1;
    maSim.pushListXms(stX);
    //On déclenche Klaxon
    if (em433 || klaxFil)
        sem_post(&semK); // V()
    //On publie sur Cayenne
    sd.type = 1;
    gpd->writePipeSort(&sd);

}

/**
 * traitement433
 * @param p valeur reçu du recepteur 433
 */
void traitement433(unsigned long p) {
    int cr;
    stXm stX;
    string texto = "";
    bool exeptCle = true;
    static bool basc = false; //pour les capteurs de mouvement
    unsigned char test;
    std::ostringstream ostst;
    string typeCapteur = "";

    typeCapteur = dataBase.getTypeCapt(p);
    if (typeCapteur == "PorteClef" || p == (unsigned long) fMarche || p == (unsigned long) fArret)
        exeptCle = false; // C'est un porte cle 433 ou M/A relai donc on ne traite pas
    if (dataBase.getstate() == 1 && exeptCle) {
        float temp;
        int IDc;
        if (Debug)
            printf("\n=> RFreq=%lx \n", p);
        if (typeCapteur == "Ouverture") { //C'est un capteur d'ouverture non programmable
            traitOuverture(p);
        } else if (typeCapteur == "Mouvement") { //C'est un capteur de mouvement non programmable
            traitIntrusion(p, &basc);
        } else {
            test = (unsigned char) (p >> 16);
            switch (test) {
                case 0xC2:
                case 0xC3:
                { // C'est un capteur de mouvement
                    traitIntrusion(p, &basc);
                }
                    break;

                case 0xC4:
                { // C'est un capteur d'ouverture
                    traitOuverture(p);
                }
                    break;

                case 0xFF:
                { // C'est un capteur de température ******* TEMPERATURE **********
                    IDc = (p >> 11)&0x0000001F; //On elimine les poids forts
                    temp = ((p & 2047) / 10.0) - 56;
                    if (temp != 85) {
                        if (Debug)
                            printf("(RF) Id: %d  Temp: %5.2f\n\n", IDc, temp);
                        ostst.str("");
                        ostst << "Alerte temp, Code:" << IDc << "  Temp:" << temp;
                        mlog.wLine("\ntraitement433", dateL(), ostst.str());
                        traitTemperature(temp, IDc);
                    }
                }
                    break;
                default:
                {
                    if (Debug)
                        printf("433 : freq inconnue\n");
                    mlog.wLine("\ntraitement433", dateL(), "freq inconnue");
                }
            }
        }
    }
}

/**
 * Thread d'activation du Klaxon 433
 */
void* KLAXON(void*) {
    int vals;
    if (Debug)
        printf("\033[33m->Thread (p7) Klaxon OU em433 lancé (Voir confAppli.xml)\033[00m\n");
    mlog.wLine("thread KLAXON", "", "\033[33m->Thread (p7) KLAXON lancé (emet433 ou fil dans confAppli.xml)\033[00m");
    while (1) {
        sem_wait(&semK); // P(semK)
        if (klaxFil || em433) {
            if (Debug)
                printf("\nlancement Klaxon\n");
            Alarme.sonore();
        }
        sem_getvalue(&semK, &vals);
        if (vals > 0) { //On efface les lancements pd que ca klaxonnait
            sem_init(&semK, 0, 0); //pour ne pas klaxonner plusieurs fois
        }
    }
    pthread_exit(NULL);
}

/**
 * Thread de surveillance des cameras
 */
void* scan(void*) {
    if (Debug)
        printf("\033[33m->Thread (p2) Surv Caméras lancé\033[00m\n");
    myMonit.metScan("/home/pi/Surveillance", "/home/pi/Images"); //Contient une boucle infinie
    //Pas de '/' de fin dans les noms de repertoire
    pthread_exit(NULL);
}

/**
 * Thread de surveillance de reception de SMS
 */
void* appel(void*) {
    if (Debug)
        printf("\033[33m->Thread (p1) Recep SMS lancé\033[00m\n");
    maSim.recepSMS();
    pthread_exit(0);
}

/**
 * Thread de surveillance pour arrivée messages MQTT
 * sert si présence ESP avec température
 */
void* recepMqtt(void*) {
    if (Debug)
        printf("\033[33m->Thread (p5) recep MQTT lancé\033[00m\n");
    mosquitto_subscribe(objMosq, NULL, MQTT_TOPIC_RECEP, 0);
    // Specify the function to call when a new message is received
    mosquitto_message_callback_set(objMosq, mon_message_callback);
    // Wait for new messages
    int loop = mosquitto_loop_forever(objMosq, -1, 1); //-1=defaut=1000ms
    //	int loop= mosquitto_loop_start(objMosq); //Fonctionnemnet idem ligne précédente si keepalive=60
    if (loop != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to start loop: %i\n", loop);
    }
    printf("FIN main-recepMQTT\n");
    pthread_exit(0);
}

/**
 * Thread de surveillance des Pipes de comm avec Cayenne
 */
void* communicPipe(void*) {
    stXm stX;
    string texto, str;
    stringstream ss;

    if (Debug)
        printf("\033[33m->Thread (p6) comm Pipe lancé\033[00m\n");
    while (1) {
        gpd->readPipeEnt(&sd);
        ss = (stringstream) ("");
        if (Debug)
            printf("\nmain - Pipe recu relai: %hd - capt : %hd - marche : %hd\n", sd.relai, sd.capt, sd.marche);
        if (sd.type == 11) { //pilotage relai
            if (Debug)
                printf("%s : main - Changement etat relai\n", dateL().c_str());
            mlog.wLine("\nmain.communicPipe", dateL(), "Changement etat relai");
            //Pilotage du relai
            maSim.repositRelai(sd.relai);
            ss << sd.relai;
            str = ss.str();
            texto = "CayennePilotRelai = ";
            texto += str;
            texto += "\n- ";
            texto += dateL();
            texto += '\0';
            stX.type = 0; //SMS
            stX.message = texto;
            stX.groupe = 1;
            maSim.pushListXms(stX);
        }
        if (sd.type == 12) { //Marche/Arret appli
            if (Debug)
                printf("%s: main - M/A appli\n", dateL().c_str());
            mlog.wLine("\nmain.communicPipe", dateL(), "M/A appli");
            dataBase.updatestate(sd.marche);
            ss << sd.marche;
            str = ss.str();
            texto = "ChgtEtatCayenne = ";
            texto += str;
            texto += "\n- ";
            texto += dateL();
            texto += '\0';
            stX.type = 0; //SMS
            stX.message = texto;
            stX.groupe = 1;
            maSim.pushListXms(stX);
        }
    }
}

/**
 * Thread de surveillance pour l'emission de SMS ou MMS
 */
void* sendXms(void*) {
    if (Debug)
        printf("\033[33m->Thread (p3) send XMS lancé\033[00m\n");
    maSim.SendXMS(); //contient boucle infinie
    pthread_exit(0);
}

/**
 * Thread de surveillance de coupure d'alim
 */
void* defAlim(void*) {
    int group = 1;
    int deb = 0;
    stXm stX;
    printf("\033[33m->Thread (p4) DEFAUT ALIM lancé (alimSec de confAppli.xml)\033[00m\n");
    mlog.wLine("thread defAlim", "", "\033[33m->Thread (p4) DEFAUT ALIM lancé (alimSec de confAppli.xml)\033[00m");
    if (wiringPiSetup() == -1 && Debug) {
        printf("Erreur wiring PI");
    }

    pinMode(PinDefAlim, INPUT);
    while (1) {
        if (dataBase.getstate()) {
            if (digitalRead(PinDefAlim) == 1 && bascAlim == 0 && deb == 0) {
                tempoCoupure = std::time(0);
                deb = 1;
            }
            //Debugage
            //printf("PinAlim=%d\n", digitalRead(PinDefAlim));
            //printf("Tempo=%d  - %d\n",tempoCoupure, std::time(0));

            if (digitalRead(PinDefAlim) == 1 && (std::time(0) - tempoCoupure > 45) && bascAlim == 0 && deb == 1) { 
                // On attend au minimum 45s (microcoupures)
                if (Debug)
                    printf("Coupure courant\n");
                mlog.wLine("\nmain.defAlim", dateL(), "Coupure courant");
                dataBase.coupure();
                string texto = "Coupure de courant. ";
                texto += dateL();
                texto += '\0';
                stX.type = 0; //sms
                stX.message = texto;
                stX.groupe = group;
                maSim.pushListXms(stX);
                bascAlim = 1;
                deb = 0;
            }

            if (digitalRead(PinDefAlim) == 0 && std::time(0) - tempoCoupure <= 6 && bascAlim == 0 && deb == 1) {
                //retour avant 4s
                deb = 0;
            }

            if (digitalRead(PinDefAlim) == 0 && bascAlim == 1) {
                if (Debug)
                    printf("Retour courant\n");
                mlog.wLine("\nmain.defAlim", dateL(), "Retour courant");
                dataBase.retour();
                string texto = "Retour du courant. ";
                texto += dateL();
                texto += '\0';
                //On diffuse le SMS en le mettant dans la list
                stX.type = 0; //sms
                stX.message = texto;
                stX.groupe = group;
                //maSim.pushListXms(stX);
                bascAlim = 0;
            }
        }
        sleep(1);
    }
    pthread_exit(0);
}

/*************************************************************
 * ***********************************************************
 * main
 */
int main() {
    int nId, af = 0, cpt = 0, anim;
    int ferr, err;
    int p1, p2, p3, p4, p5, p6, p7, p8;
    int clef3G = 0;
    mySwitch = new RCSwitch();
    tpsRebond = std::time(0);
    pthread_attr_t thAttr;
    struct sched_param shParam;
    pthread_t recep;
    pthread_t survMonit;
    pthread_t alim;
    pthread_t sendXMS;
    pthread_t recepMQTT;
    pthread_t comPipe;
    pthread_t klaxon;
    pthread_t alert433;

    sem_init(&semK, 0, 0); //semaphore Klaxon a 0

    //On tue l'eventuel ancien domoCayenne
    system("sudo pkill domoCayenne");
    usleep(200000);

    gpd = new gestPipe();

    attribText = (const char*) attribT;
    dataBase.getAdLog(&mlog);
    myMonit.getAdLog(&mlog);
    maSim.getAdLog(&mlog);
    gpd->getAdLog(&mlog);
    Alarme.getAdLog(&mlog);
    sleep(1);
    maSim.setAdLog();
    sleep(1);

    ifstream ifile("/home/pi/Images");
    if (!ifile) {
        system("sudo mkdir /home/pi/Images");
        usleep(200000);
        system("sudo chown pi.pi /home/pi/Images");
        system("sudo chmod 775 /home/pi/Images");
    }
    ifstream ifile2("/home/pi/Surveillance");
    if (!ifile2) {
        system("sudo mkdir /home/pi/Surveillance");
        usleep(200000);
        system("sudo chown pi.pi /home/pi/Surveillance");
        system("sudo chmod 777 /home/pi/Surveillance");
    }
    ifstream ifile4("/home/pi/Domot.err");
    if (!ifile4) {
        std::ofstream outfile4("/home/pi/Domot.err");
        usleep(200000);
        outfile4.close();
        system("sudo chown pi.pi /home/pi/Domot.err");
        system("sudo chmod 666 /home/pi/Domot.err");
    }

    //Fichier de log
    std::ofstream ficlog("/home/pi/Domot.log", ios::app); //ouverture en ajout
    mlog.setOutput(ficlog);
    if (!ficlog.is_open())
        if (Debug)
            cout << "Impossible d'ouvrir le fichier Domot.log" << endl;

    if (!Debug) {
        //*** REDIRECTION des ports d'E/S ***
        ferr = open("/home/pi/Domot.err", O_CREAT | O_APPEND | O_WRONLY | O_NONBLOCK);
        close(2); //On ferme la sortie d'erreur (stderr)
        dup(ferr); //pour la rediriger sur ce fichier
    }

    wiringPiSetup();
    //dataBase.updatestate(1);

    //printf("Entrez le numero du port COM utilisé : ");
    //scanf("%d", &nId);
    char ver[20];
    strcpy(ver, VERSION);
    printf("\nVersion : %s\n", ver);
    mlog.wLine("\n\n==> main", dateL(), "Lancement appli");
    printf("\nATTENTION lancer en sudo a cause du ping !\n");
    printf("On attend 6s le demarrage des services ...\n\n");
    sleep(6);

    maSim.setAdBDD(&dataBase);
    maSim.setAdMutex(&exMut);
    myMonit.setAdBdd(&dataBase);
    myMonit.setAdSim(&maSim);
    myMonit.effaceRepDeb("/home/pi/Surveillance");
    myMonit.setAdGpd(gpd);
    myMonit.setAdAlarme(&Alarme);
    myMonit.setAdSem(&semK);

    eResult = xmlDoc.LoadFile("/home/pi/Domotik/confAppli.xml");
    if (eResult != XML_SUCCESS) {
        std::cerr << "main : Erreur de chargement du fichier XML" << std::endl;
    }

    XMLNode* rootNode = xmlDoc.FirstChildElement("Domotik");
    if (rootNode == nullptr) {
        printf("Pas de noeud XML Domotik\n");
        mlog.wLine("main", dateL(), "Pas de noeud XML Domotik");
    } else {
        element = rootNode->FirstChildElement("Mqtt");
        if (element != nullptr) {
            attribText = element->Attribute("hostname");
            strcpy(MQTT_HOSTNAME, attribText);
            attribText = element->Attribute("port");
            MQTT_PORT = atoi(attribText);
            attribText = element->Attribute("username");
            strcpy(MQTT_USERNAME, attribText);
            attribText = element->Attribute("password");
            strcpy(MQTT_PASSWORD, attribText);
            attribText = element->Attribute("topicRECEP");
            strcpy(MQTT_TOPIC_RECEP, attribText);
            attribText = element->Attribute("topicEMISS");
            strcpy(MQTT_TOPIC_EMISS, attribText);
        }
        element = rootNode->FirstChildElement("Relai");
        if (element != nullptr) {
            attribText = element->Attribute("marche");
            fMarche = atoi(attribText);
            attribText = element->Attribute("arret");
            fArret = atoi(attribText);
            attribText = element->Attribute("pinalim");
            PinAlim = atoi(attribText);
            attribText = element->Attribute("pinRF");
            PinRF = atoi(attribText);
            attribText = element->Attribute("pindefalim");
            PinDefAlim = atoi(attribText);
        }
        element = rootNode->FirstChildElement("Param");
        if (element != nullptr) {
            item = element->FirstChildElement("emiss433");
            stg = item->GetText();
            em433 = stoi(stg);
            item = element->FirstChildElement("klaxonFil");
            stg = item->GetText();
            klaxFil = stoi(stg);
            item = element->FirstChildElement("alimSec");
            stg = item->GetText();
            alimSecours = stoi(stg);
            item = element->FirstChildElement("cle3G");
            stg = item->GetText();
            cle3G = stoi(stg);
            item = element->FirstChildElement("cayenne");
            stg = item->GetText();
            cay = stoi(stg);
            item = element->FirstChildElement("portSerie");
            ttyPort1 = item->GetText();
        }
    }

    if (cle3G)
        ttyPort1 = "/dev/ttyUSB0";
    //sinon c'est celui du configAppli.xml
    ttyPort2 = "/dev/ttyUSB2";
    maSim.setNomPort(ttyPort1, ttyPort2);
    maSim.ouvertureSerie(ttyPort1);
    if (cle3G)
        maSim.ouvertureSerie2(ttyPort2);
    usleep(500000);

    // mise à 99.9 des capteurs de température
    dataBase.initTemperature();

    //mqtt_Initialise();
    //printf("MQTT : %s %s %d\n", MQTT_HOSTNAME, MQTT_USERNAME, MQTT_PORT);
    mqtt_Initialise(MQTT_HOSTNAME, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
    usleep(500000);
    maSim.setAdMosquito(objMosq, MQTT_TOPIC_EMISS);

    //On efface anciens topics température
    string reqS;
    if (Debug)
        puts("On efface les topics Mqtt");
    for (int i = 0; i < 10; i++) {
        reqS = "mosquitto_pub -h localhost -t \"DOMO/Capt2" + to_string(i) + "\" -r -n";
        system(reqS.c_str());
    }


    //pthread_mutex_init(&exMut, NULL);
    exMut = PTHREAD_MUTEX_INITIALIZER;

    if (pthread_attr_init(&thAttr) != 0) {
        fprintf(stderr, "pthread_attr_init error");
        exit(EXIT_FAILURE);
    }
    //On choisit une politique "Round Robin"
    if ((err = pthread_attr_setschedpolicy(&thAttr, SCHED_RR)) != 0) {
        fprintf(stderr, "setschedpolicy: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    // On déhérite le thread des priorités du parent
    if ((err = pthread_attr_setinheritsched(&thAttr, PTHREAD_EXPLICIT_SCHED)) != 0) {
        fprintf(stderr, "setinheritsched: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    /*
    //pour le rendre détachable
    if (pthread_attr_setdetachstate(&thAttr, PTHREAD_CREATE_DETACHED) != 0) {
            fprintf(stderr, "pthread_attr_setdetachstate error");
            exit(1);
    }*/

    if (Debug)
        printf("Lancement threads\n");
    mlog.wLine("main", dateL(), "Lancement des threads");
    //de 3 à 6 priorité normale (cad SCHED_OTHER )
    p5 = pthread_create(&recepMQTT, NULL, recepMqtt, NULL);
    usleep(100000);
    p3 = pthread_create(&sendXMS, NULL, sendXms, NULL);
    usleep(100000);
    if (alimSecours) {
        p4 = pthread_create(&alim, NULL, defAlim, NULL);
        usleep(100000);
    }
    p6 = pthread_create(&comPipe, NULL, communicPipe, NULL);
    usleep(100000);

    //1,2,8 haute priorité
    shParam.sched_priority = 70; //HAUTE priorite
    if ((err = pthread_attr_setschedparam(&thAttr, &shParam)) != 0) {
        fprintf(stderr, "setschedparam: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    //p2 = pthread_create(&survMonit, &thAttr, scan, NULL);
    p2 = pthread_create(&survMonit, NULL, scan, NULL);
    usleep(100000);
    //p8 = pthread_create(&alert433, &thAttr, th_alert433, NULL);
    p8 = pthread_create(&alert433, NULL, th_alert433, NULL);
    usleep(100000);
    //usleep(500000);
    //p1 = pthread_create(&recep, &thAttr, appel, NULL);
    p1 = pthread_create(&recep, NULL, appel, NULL);
    usleep(100000);
    if (klaxFil || em433) {
        shParam.sched_priority = 4; //faible priorite
        if ((err = pthread_attr_setschedparam(&thAttr, &shParam)) != 0) {
            fprintf(stderr, "setschedparam: %s\n", strerror(err));
            exit(EXIT_FAILURE);
        }
        //p7 = pthread_create(&klaxon, &thAttr, KLAXON, NULL); //ROUND_ROBIN
        p7 = pthread_create(&klaxon, NULL, KLAXON, NULL); //SCHED_OTHER 
        usleep(100000);
    }
    if (p1 || p2 || p3 || p5 || p6 || p8)
        fprintf(stderr, "%s - %s - %s - %s - %s - %s\n", strerror(p1), strerror(p2), strerror(p3), strerror(p5), strerror(p6), strerror(p8));

    //Si coupure courant on remet Relai en adequation avec base
    unsigned char st = dataBase.testEtatRelai();
    if (st == 1)
        maSim.repositRelai(1);
    if (st == 0)
        maSim.repositRelai(0);
    if (st == 2)
        fprintf(stderr, "Erreur lecture etat Relai/VMC ");

    //On lance domoCayenne si cayenne dans XML ET réseau
    char res[20];
    strcpy(res, "www.orange.fr");
    if (cay && maSim.pingOK(res)) {
//    if (cay) {
        if (Debug)
            printf("\033[35m=>Lancement de domoCayenne\033[00m\n");
        mlog.wLine("=>main", "", "\033[35mLancement de domoCayenne\033[00m");
        system("/home/pi/DomoCayenne/domoCayenne &");
        sleep(1);
    }
    if (Debug)
        printf("\nBoucle infinie, animée /5mn\n");
    mlog.wLine("main", dateL(), "Boucle infinie, animée");
    if (Debug)
        anim = 15; //15s
    else
        anim = 1800; //demi heure

    while (1) {
        if (cpt++ == 120) { //tte les 2mn
            //(re)lancement de l'appli de gestion de Cayenne si arrivé du réseau
            if (maSim.pingOK(res) && cay) {
                //On relance domoCayenne s'il n'y est pas
                system("ps -e | grep -q domoCayenne || /home/pi/DomoCayenne/domoCayenne &");
            }
            cpt = 0;
        }
        // Animation
        if (af++ == anim) {
            if (Debug)
                printf(".");
            fflush(stdout);
            mlog.wPoint();
            af = 0;
        }
        etat = dataBase.getstate();
        sleep(1);
    }

    //pthread_join(recep, NULL);
    //pthread_join(alim, NULL);
    //pthread_join(sendXMS, NULL);
    //pthread_join(survMonit, NULL);
    return 0;
}
