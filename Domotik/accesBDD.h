#ifndef ACCESBDD_H
#define ACCESBDD_H
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <time.h>
#include <mysql/mysql.h>
#include <tinyxml2.h>
#include "Log.h"

using namespace tinyxml2;
using namespace std;

class accesBDD {
private:
    Log *pLog;
    string host;
    string dbname;
    string login;
    string pwd;
    int port;
    string dt;
    MYSQL_ROW row;
    MYSQL_FIELD *mfield;
    MYSQL_RES *result;
    XMLDocument xmlDoc;
    XMLElement *element, *item;
    XMLError eResult;

public:
    accesBDD();
    ~accesBDD();
    void getAdLog(Log*);
    string dateL();
    string getnum(string); // récuperer tous les numéros d'un groupe   
    void updatetemp(unsigned long, float); // modifie la temperature d'un capteur    
    void majtemp(string, string); // modifie la temperature d'un certain capteur
    void alertcapt(string, string); // ajoute un evenement dans la table alerte
    void alertcam(string, string); // ajout un evenement dans la table alerte
    float gettemp(int); // recupere la temperature d'un capteur donné
    string gettempToutes(); // recupere toutes les temperatures
    string getidcapt(int); // récupere le ID d'un capteur en fonction de son code
    string getTypeCapt(unsigned long); // récupere le type d'un capteur en fonction de son code
    string getidcam(string); // récupere le ID d'une caméra
    bool testpwd(string); // test la validité du mdp passé en parametre
    bool testpwdSMS(string); // test la validité du mdp SMS
    int getstate(); // recupere l'état du systeme
    float gettempalert(int); // récupere la temperature d'alerte d'un certain capteur
    void updatestate(int); // modifie l'état du systeme
    void updateEtatRelai(int);
    unsigned char testEtatRelai();
    string getcamRep(string id); //récupère le repertoire d'accueil de la cam
    int getNbCam(); // recupere le nombre de cameras
    string getNomSite();
    string getcamGeo(string id); //récupère le champ PosGeo
    string getcaptname(int); // recupere le nom d'un capteur en fonction de son code
    string getcamname(string);
    string getIndexImage();
    string getLastImg(string numImg = "0");
    void incIx();
    void putLastImg(string);
    void coupure(); // Ajoute un evenement coupure de courant
    void retour(); // Ajoute un evenement retour du courant
    void sortErr(int); //affiche le num d'erreur
    void initTemperature(); //Initialise les températures à 99
};
#endif
