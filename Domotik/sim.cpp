/* 
 * File:   sim.cpp
 * Author: Aucaigne
 * Version 2/11/2021
 */
#include <algorithm> // pour std::transform
#include "sim.h"

#define Debug 0
#define smsDeb 0
#define couleur(param) fprintf(stderr,"\033[%sm",param)

SIM::SIM() {
   int l = 0, c = 0;
   string ligne; // déclaration d'une chaîne qui contiendra la ligne lue
   string token;
   string donnees[3];
   etat = 0;
   tps = 0;
   tpsSMS = 0;
   etatReseau = true;

   comp = new compress();

   eResult = xmlDoc.LoadFile("/home/pi/Domotik/confAppli.xml");
   if (eResult != XML_SUCCESS) {
      std::cerr << "sim :Erreur de chargement du fichier XML" << std::endl;
   }

   XMLNode* rootNode = xmlDoc.FirstChildElement("Domotik");
   if (rootNode == nullptr) {
      if (Debug)
         puts("Pas de noeud XML Domotik");
      pLog->wLine("sim", "", "Err chgt XML");
   } else {
      element = rootNode->FirstChildElement("Param");
      if (element != nullptr) {
         item = element->FirstChildElement("num_carte");
         stg = item->GetText();
         strcpy(my_phone_number, stg.c_str());
         item = element->FirstChildElement("mms_APN");
         stg = item->GetText();
         strcpy(mmsAPN, stg.c_str());
         item = element->FirstChildElement("mms_Centre");
         stg = item->GetText();
         strcpy(mmsCentre, stg.c_str());
         item = element->FirstChildElement("mms_CentreH");
         stg = item->GetText();
         strcpy(mmsCentreH, stg.c_str());
         item = element->FirstChildElement("mms_CentreIP");
         stg = item->GetText();
         strcpy(mmsCentreIP, stg.c_str());
         item = element->FirstChildElement("mms_Proxy");
         stg = item->GetText();
         strcpy(mmsProxy, stg.c_str());
         item = element->FirstChildElement("mms_Port");
         stg = item->GetText();
         mmsPort = stoi(stg);
         item = element->FirstChildElement("mdpSMS");
         stg = item->GetText();
         strcpy(mdpSms, stg.c_str());
         item = element->FirstChildElement("IP_Interrupt");
         stg = item->GetText();
         strcpy(ipRelaiESP, stg.c_str());
         item = element->FirstChildElement("cle3G");
         stg = item->GetText();
         cle3G = stoi(stg);
      }
   }
   nomPortSerie = "";
   numReqSms = "+33651980787"; //valeur par défaut au démarage
   memset(imageMMS, '\0', 307999);
   memset(textMMS, '\0', 15359);
}

SIM::~SIM() {
}

/**
 * @return 
 */
string SIM::getNumReqSms() {
   return numReqSms;
}

/**
 * 
 * @param snp
 */
void SIM::setNomPort(string snp, string snp2) {
   nomPortSerie = snp;
   nomPortSerie2 = snp2;
}

/**
 * 
 * @param ad
 */
void SIM::setAdBDD(accesBDD* ad) {
   adbdd = ad;
   reseau.setAdBDD(ad);
}

/**
 * 
 * @param exmut
 */
void SIM::setAdMutex(pthread_mutex_t* exmut) {
   myMutex = exmut;
}

void SIM::setAdMosquito(struct mosquitto* adm, string te) {
   leMosq = adm;
   topicEmiss = te;
}

/**
 *
 */
string SIM::dateL() {
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

string SIM::nivGsm() {
   string nGsm;
   nGsm = maCom.sendAT("AT+CSQ\r", "OK", 5000); // Niveau de reseau
   sleep(1);
   nGsm = nGsm.substr(7);
   int fdCsq = nGsm.find("+csq");
   if (fdCsq > 1 && fdCsq < 255) {
      return (nGsm.substr(fdCsq, 10));
   } else
      return (" mauvais");
}

/**
 * @param lePort
 * @return 
 */
int SIM::ouvertureSerie(string lePort) {
   string niveauGSM, seuil;
   string etatM = "";
   string nivo = "req CSQ non trouvée";
   string leSite = "";
   int nivoEnt = -1;
   stXm stX;
   fdPort = maCom.openTTY(lePort.c_str());
   if (Debug)
      printf("\n%s Ouvert= %d\n", lePort.c_str(), fdPort);
   ostst.str("");
   ostst << lePort.c_str() << "Ouvert = " << fdPort;
   pLog->wLine("sim.ouvertureSerie", "", ostst.str());
   maCom.sendAT("AT+IFC=0,0;+IPR=0;&W\r", "IFC", 10000); // Pas de poignee de main et vitesse auto.
   //maCom.sendAT("AT+IFC=1,1;+IPR=0;&W\r", "IFC", 30000); // Xon/XOFF
   maCom.sendAT("AT+CMEE=2\r", "OK", 10000); //Active code d'erreur
   //maCom.sendAT("AT+CPIN=\"1234\"\r", "OK", 20000);
   ////maCom.sendAT("AT+CREG?\r", "OK", 20000); //liste des r�seaux support�s
   /*
   2eme parametre recu :
   Possible values for access technology are,
   0 GSM
   1 GSM Compact
   2 UTRAN
   3 GSM w/EGPRS
   4 UTRAN w/HSDPA
   5 UTRAN w/HSUPA
   6 UTRAN w/HSDPA and HSUPA
   7 E-UTRAN
    */
   niveauGSM = maCom.sendAT("AT+CSQ\r", "OK", 10000); // Niveau de reseau
   sleep(1);
   //si <10 Mauvais
   //si >10 OK
   //si >15 Bon
   //si >20 Excellent    

   //On diffuse le SMS en le mettant dans la list
   niveauGSM = niveauGSM.substr(7);
   int foundCsq = niveauGSM.find("+csq");
   if (Debug)
      printf("Niveau de GSM (< 10 Mauvais, >15 Bon) = ");
   if (foundCsq > 1 && foundCsq < 255) {
      nivo = niveauGSM.substr(foundCsq, 10);
      seuil = niveauGSM.substr(foundCsq + 6, 4);
      int virg = seuil.find(",");
      string vv = seuil.substr(0, virg);
      nivoEnt = stoi(vv);
      if (Debug)
         printf(" %s, donc %d\n", seuil.c_str(), nivoEnt);
      ostst.str("");
      ostst << "Niveau SIM = " << seuil.c_str();
      pLog->wLine("sim.ouvertureSerie", "", ostst.str());
   } else {
      if (Debug)
         printf("Mauvais CSQ : %s, donc %d\n", seuil.c_str(), nivoEnt);
   }
   /*** Traitement si niveau csq < 10 ***/
   /****On ne fait pas ce test pour la cle 3G
   if (nivoEnt < 10) {
           etatReseau = false; //On est en mode dégradé sans SMS ni MMS
           printf("==> Mode degrade PAS de MMS\n");
   }
    */
   leSite = adbdd->getNomSite();
   etat = adbdd->getstate();
   if (etat)
      etatM = ", " + leSite + ": En marche";
   else
      etatM = ", " + leSite + ": A l'arret";

   if (Debug && smsDeb) {
      stX.type = 0; //sms
      stX.message = nivo + etatM + ", \n" + dateL();
      stX.groupe = 1;
      pushListXms(stX);
   }
   return fdPort;
}

/**
 * @param lePort
 * @return 
 */
int SIM::ouvertureSerie2(string lePort) {
   stXm stX;
   fdPort2 = maCom2.openTTY(lePort.c_str());
   if (Debug)
      printf("%s Ouvert2= %d\n", lePort.c_str(), fdPort2);
   ostst.str("");
   ostst << lePort.c_str() << "Ouvert2 = " << fdPort2;
   pLog->wLine("sim.ouvertureSerie2", "", ostst.str());
   maCom2.sendAT("AT+IFC=0,0;+IPR=0;&W\r", "IFC", 10000); // Pas de poignee de main et vitesse auto.
   //maCom.sendAT("AT+IFC=1,1;+IPR=0;&W\r", "IFC", 30000); // Xon/XOFF
   maCom2.sendAT("AT+CMEE=2\r", "OK", 8000); //Active code d'erreur
   return fdPort2;
}

std::string url_encode(const std::string &value) {
   std::ostringstream escaped;
   escaped.fill('0');
   escaped << std::hex;

   for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
      std::string::value_type c = (*i);

      // Keep alphanumeric and other accepted characters intact
      if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
         escaped << c;
         continue;
      }

      // Any other characters are percent-encoded
      escaped << std::uppercase;
      escaped << '%' << std::setw(2) << int((unsigned char) c);
      escaped << std::nouppercase;
   }

   return escaped.str();
}

/**
 * Envoyer SMS de notification avec curl par internet
 * @param numero : NE SET PAS !!!
 * @param message
 * @return 
 */
int SIM::envSMScurl(string numero, string message) {
   string mesC = "curl -s -G \"https://smsapi.free-mobile.fr/sendmsg?user=14176774&pass=D6oPKEGUD0U6sp&msg=(curl) ";
   mesC += url_encode(message);
   mesC += "\"";
   system(mesC.c_str());
   return (1);
}

/**
 * 
 * @param numero string numéro GSM reception
 * @param message contenu du sms
 * @return 1
 */
int SIM::envSMS(string numero, string message) {
   string answer;
   char aux_string[350];
   char numR[25];
   //cout << "SMS demande SC" << endl;
   pthread_mutex_lock(myMutex);

   memset(aux_string, '\0', 118);
   maCom.sendAT("AT+CMGF=1\r", "OK", 7000); // Activation du mode texte pour les SMS.
   strcpy(numR, numero.c_str());
   sprintf(aux_string, "AT+CMGS=\"%s\"\r", numR);
   answer = maCom.sendAT(aux_string, ">", 10000); // Envoi du numero de tel au module GSM.
   if (answer != "") {
      memset(textSms, '\0', 143);
      // On insere le corps du message.
      strcpy(textSms, message.c_str());
      int lgSms = strlen(textSms);
      textSms[lgSms] = 0x1A; //OBLIGATOIRE !!!
      textSms[lgSms + 1] = '\0';
      usleep(5000);
      maCom.sendAT(textSms, textSms, 10000);
      usleep(lgSms * 1600);
   } else {
      fprintf(stderr, "Erreur sur CMGS (emiss message) !");
   }
   sleep(1);
   //cout << "SMS va sortir SC" << endl;
   pthread_mutex_unlock(myMutex);
   if (Debug)
      printf("\nSMS = %s : %s\n", numero.c_str(), message.c_str());
   ostst.str("");
   ostst << "SMS : " << numero << " : " << message;
   pLog->wLine("\nsim.envSMS", "", ostst.str());
   return (1);
}

/**
 * 
 * @param numero du destinataire
 * @param txMMS message du mms
 * @param nomImage nom de l'image
 * @param cheminImage chemin d'acces à l'image
 * @return 1
 */
int SIM::envMMS(string numero, string txMMS, string nomImage, string cheminImage) {
   FILE *fd;
   int ix = 0;
   string scr;
   memset(imageMMS, '\0', 307999);
   memset(textMMS, '\0', 15359);
   couleur("36");
   if (Debug) {
      printf("\nMMS : %s : %s\n", numero.c_str(), txMMS.c_str());
   }

   //puts("Chargement de l'image ...");
   fd = fopen(cheminImage.c_str(), "rb");

   while (!feof(fd)) {
      fread(&imageMMS[ix++], 1, 1, fd);
      //fprintf(stderr,"%x ",image[ix-1]);
   }
   //fprintf(stderr,"\nix=%d car =%x\n",ix, image[ix-2]);
   //    image[ix-1]=0x1A;
   imageMMS[ix - 1] = '\r';
   imageMMS[ix] = '\0';
   int lgIm = ix; //Le dernier caractere est le D9 de fin de JPG
   if ((lgIm) >= 307200)
      fprintf(stderr, "ERREUR, l'image est trop grosse !");
   fclose(fd);
   //puts("---> fin de chargement\n");

   //cout << "MMS demande SC" << endl;
   pthread_mutex_lock(myMutex);

   //puts("PREPA Reseau\n");
   maCom.sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r", "OK", 7000); //GPRS possible
   sprintf(aux_string, "AT+SAPBR=3,1,\"APN\",\"%s\"\r", mmsAPN); //APN );
   maCom.sendAT(aux_string, "OK", strlen(aux_string)*500);
   usleep(100000);
   int cb = 0;
   do {
      scr = maCom.sendAT("AT+SAPBR=1,1\r", "OK", 8000); //Open porteuse
      if (scr.find("error") != std::string::npos) {
         if (Debug) {
            couleur("31");
            printf("-->\t\tERREUR sapbr\n");
            couleur("36");
         }
         sleep(1);
      }
   } while (++cb < 2 && scr.find("error") != std::string::npos);
   usleep(50000);
   maCom.sendAT("AT+SAPBR=2,1\r", "OK", 8000); //Query porteuse
   usleep(50000);
   scr = maCom.sendAT("AT+CMMSINIT\r", "OK", 8000);
   if (Debug && (scr.find("error") != std::string::npos)) {
      couleur("31");
      printf("-->\t\tERREUR init\n");
      couleur("36");
   }
   usleep(50000);
   //maCom.sendAT("AT+CMMSTIMEOUT=120,120\r", "OK", 40000); //en secondes
   //PROTO=Proxy et free n'en utilise pas mais ...
   sprintf(aux_string, "AT+CMMSPROTO=\"%s\",%d\r", mmsProxy, mmsPort);
   maCom.sendAT(aux_string, "OK", strlen(aux_string)*500);

   sprintf(aux_string, "AT+CMMSCURL=\"%s\"\r", mmsCentreIP); //sans le http:// ou mmsCentre
   maCom.sendAT(aux_string, "OK", strlen(aux_string)*500);
   maCom.sendAT("AT+CMMSCID=1\r", "OK", 7000); // celui du SAPBR !
   //maCom.sendAT("AT+CMMSSENDCFG=6,3,0,0,2,4\r", "OK", 30000); //CMMSSENDCFG: <valid>,<pri>,<sendrep>,<readrep>,<visible>,<class>,<subctrl>,<notifyskip>
   maCom.sendAT("AT+CMMSSENDCFG=1,1,0,0,1,4,2,1\r", "OK", 20000); //CMMSSENDCFG: <valid 12h>,<pri normal>,<sendrep Pas rappot>,<readrep Pas de rapport>,<visible Oui>,<class 4 Not Set>,<subctrl 2=english>,<notifyskipHTTP 0=wait, 1=skip>
   /////maCom.sendAT("AT+CMMSSENDCFG?\r", "OK", 30000);

   //****DEBUT***
   scr = maCom.sendAT("AT+CMMSEDIT=0\r", "OK", 20000); //Pour vider ce qu'il peut rester
   usleep(50000);
   maCom.sendAT("AT+CMMSEDIT=1\r", "OK", 20000);
   if (Debug) {
      maCom.sendAT("AT+CPAS\r", "OK", 10000); // statut equipement
      maCom.sendAT("AT+COPS?\r", "OK", 10000); // Opérateur
      maCom.sendAT("AT+CREG?\r", "OK", 10000); // Info registr network
      maCom.sendAT("AT+CMMSVIEW\r", "OK", 20000);
   }

   //****l'image***
   sprintf(aux_string, "AT+CMMSDOWN=\"PIC\",%d,%d,\"%s\"\r", lgIm, lgIm * 400, nomImage.c_str());
   maCom.sendAT(aux_string, "OK", strlen(aux_string)*500);
   if (Debug)
      puts("Depot de l'image du MMS ...");
   //maCom.writeTTY((char*) imageMMS, lgIm); usleep(lgIm * 200);
   maCom.writeImTTY((char*) imageMMS, lgIm);
   usleep(20000);

   //le TEXTE
   strcpy(textMMS, txMMS.c_str());
   int lgMms = strlen(textMMS);
   sprintf(aux_string, "AT+CMMSDOWN=\"TEXT\",%d,50000,\"%s\"\r", lgMms, textMMS);
   maCom.sendAT(aux_string, "OK", strlen(aux_string)*500);
   if (Debug)
      puts("Depot du texte du MMS ...");
   maCom.writeTTY((char*) textMMS, lgMms + 1);
   usleep(100000);

   ////maCom.sendAT("AT+CMMSSTATUS?\r", "OK", 30000);
   sprintf(aux_string, "AT+CMMSRECP=\"%s\"\r", numero.c_str());
   maCom.sendAT(aux_string, "OK", strlen(aux_string)*500);
   scr = maCom.sendAT("AT+CMMSVIEW\r", "OK", 20000);
   //cout << "view " << scr << endl;
   /***SEND***/
   int tpIm = (lgIm + lgMms) * 120; //100 semble bonne valeur
   if (Debug) {
      printf("SEND : Tempo=%d\n", tpIm);
   }
   scr = maCom.sendAT("AT+CMMSSEND\r", "OK", tpIm);
   // A raison de 11520 caracteres par seconde:
   if (scr.find("error") != std::string::npos) {
      if (Debug) {
         couleur("31");
         printf("-->\t\tERREUR SEND, on ressaye\n");
         couleur("36");
      }
      scr = maCom.sendAT("AT+CMMSVIEW\r", "OK", 6000);
      scr = maCom.sendAT("AT+CMMSSEND\r", "OK", 20000);
   }

   if (Debug)
      printf("\n=> fermeture MMS\n");
   scr = maCom.sendAT("AT+CMMSEDIT=0\r", "OK", 10000);
   //cout << "edit " << scr << endl;
   scr = maCom.sendAT("AT+CMMSTERM\r", "OK", 10000);
   if (Debug) {
      printf("\nsim - MMS envoye: %s : %s\n", numero.c_str(), txMMS.c_str());
   }
   //cout << "MMS va sortir SC" << endl;
   pthread_mutex_unlock(myMutex);
   couleur("0");
   return (1);
}

/**
 * 
 * @param messAdif
 * @param gr
 */
void SIM::diffSMS(string messAdif, string gr) {
   int i = 0, nbN, cr;
   string token;
   string sgr;
   sgr = adbdd->getnum(gr); // retourne les numeros du groupe "gr"
   // dans une string séparés par un tiret "-"

   //cout<<"liste: "<<sgr<<endl;
   istringstream iss(sgr);
   while (getline(iss, token, '-')) // sépare les numéros et les placent dans un tableau
   {
      contact[i] = token;
      //cout << contact[i] << endl;
      i++;
   }
   nbN = i;
   for (i = 0; i <= (nbN - 1); i++) // faire un envoi pour chaque contact trouvé
   {
      cr = envSMS(contact[i], messAdif);
      //cr = envSMScurl(contact[i], messAdif);
      //cout << "Diffusion, SMS envoye a : " << contact[i] << " : " << messAdif << endl;
   }
}

/**
 * Diffusion de MMS
 * @param messAdif
 * @param gr
 */
void SIM::envImgFtp(string cheminImage) {
   // On envoi l'image sur le serveur FTP public s'il y a du réseau
   if (Debug)
      cout << "sim.envImgFtp : " << cheminImage << endl;
   reseau.putFTP(cheminImage);
}

/**
 * Diffusion de MMS
 * @param messAdif
 * @param gr
 */
void SIM::diffMMS(string messAdif, string gr, string nomImage, string cheminImage) {
   string reduite = "/home/pi/Images/0A-reduite.jpg";
   string imgTmp = "/home/pi/Images/0B-tempo.jpg";
   int i = 0, nbN, cr;
   string token;
   string sgr;

   sgr = adbdd->getnum(gr); // retourne les numeros du groupe "gr"
   // dans une string séparés par un tiret "-"

   //cout<<"liste: "<<sgr<<endl;
   istringstream iss(sgr);
   while (getline(iss, token, '-')) // sépare les numéros et les placent dans un tableau
   {
      contact[i] = token;
      //cout << contact[i] << endl;
      i++;
   }
   nbN = i;
   //preparation d l'image envoyee (reduction)

   cv::Mat src = cv::imread(cheminImage.c_str());
   cv::Mat dest(600, 400, CV_8UC2); //, cv::Scalar(0, 0, 0));
   //dest = src.clone();
   cv::Size size(600, 400);
   cv::resize(src, dest, size, 1, 1, cv::INTER_LINEAR);
   cv::imwrite(imgTmp.c_str(), dest);

   comp->compression(imgTmp, reduite, 15); //compression 20%

   //cout<<"Emission im"<<endl;
   //Envoi de l'image réduite
   for (int i = 0; i <= (nbN - 1); i++) // faire un envoi pour chaque contact trouvé
   {
      cr = envMMS(contact[i], messAdif, nomImage, reduite);
      sleep(2);
   }
}

/**
 * 
 * @return le message reçu
 */
void SIM::recepSMS() {
   fd_set rfds;
   string repAT, numE, minus, SMS_;
   size_t found = 0, foundNum = 0, foundA = 0;
   int debN, retval, nbrec, port_recep;
   bool trouv = false, clef3G_b = false;
   static unsigned int np = 0;

   //if (adbdd->getcle3G() == 1) {
   if (cle3G) {
      clef3G_b = true;
      port_recep = fdPort2;
   } else {
      clef3G_b = false;
      port_recep = fdPort;
   }

   while (1) {
      sleep(3);
      minus = "";
      repAT = "";
      SMS_ = "";

      if (clef3G_b == false) {
         //cout << "\nRecep demande SC" << endl;
         pthread_mutex_lock(myMutex);
      }

      FD_ZERO(&rfds);
      FD_SET(port_recep, &rfds);
      tv.tv_sec = 2; //attente maxi de 2s
      tv.tv_usec = 0;
      do {
         retval = select(port_recep + 1, &rfds, NULL, NULL, &tv);
      } while (retval < 0);
      //cout << "\nretval=" << retval << "\t  ISSET=" << FD_ISSET(port_recep, &rfds) << endl;
      if ((retval > 0 && FD_ISSET(port_recep, &rfds)) && etatReseau) {
         //il y a des données dispo
         //puts("recep : Des données");
         if (clef3G_b == false) {
            maCom.sendAT("AT+CMGF=1\r", "OK", 15000); //mode SMS
            usleep(500 * 1000);
            repAT = maCom.sendAT("AT+CMGL=\"REC UNREAD\"\r", "OK", 200000); //Affiche messages non lus
            //maCom.sendAT("AT+CMGL=\"ALL\"\r", "OK", 20000); //Affiche tous les messages
         } else {
            maCom2.sendAT("AT+CMGF=1\r", "OK", 10000); //mode SMS
            usleep(500 * 1000);
            repAT = maCom2.sendAT("AT+CMGL=\"REC UNREAD\"\r", "OK", 200000); //Affiche messages non lus
         }
         usleep(700 * 1000);
         cout << "repAT=" << repAT << "<=" << endl;
         minus = repAT;
         nbrec = minus.length();
         std::transform(minus.begin(), minus.end(), minus.begin(), ::tolower);
         found = minus.find("unread");
         if (nbrec > 28 && ((int) found > 2)&& ((int) found < 500)) { //si au moins 1 sms recu
            if (clef3G_b == false) {
               maCom.sendAT("AT+CMGD=1,1\r", "OK", 8000); //supprime SMS lus
            } else {
               maCom2.sendAT("AT+CMGD=1,1\r", "OK", 8000); //supprime SMS lus
            }
            usleep(500 * 1000);
            if (!clef3G_b) {
               //cout << "Recep va sortir SC avec message" << endl;
               pthread_mutex_unlock(myMutex);
            }
            int y = 0;
            trouv = false;
            //cout<<"repAT="<<repAT<<"<|"<<endl;

            stringstream ss(repAT);
            string sousChaine;
            int k = 0;
            bool trouv = false;
            while (getline(ss, sousChaine, '\n')) {
               //cout<<k++<<">"<<sousChaine<<endl;
               //on extrait le numéro du demandeur
               foundNum = sousChaine.find("+33");
               if (foundNum > 10 && foundNum < 255) { // on a trouvé
                  trouv = true;
                  numE = sousChaine.substr(foundNum, 12);
               }
               for (int u = 0; u < (int) sousChaine.length(); u++) {
                  if (sousChaine[u] == 0x40) { // symbole @
                     SMS_ = sousChaine;
                  }
               }
            }
            if (trouv) {
               //cout << "num=" << numE << endl;
               //cout << "SMS=" << SMS_ << endl;
               numReqSms = numE;
               traitRecepSms(SMS_);
            } else {
               traitRecepSms("0000@mauvais");
            }
         }
         if (clef3G_b == false) {
            //cout << "Recep va sortir SC rien lu" << endl;
            pthread_mutex_unlock(myMutex);
         }
      } else {
         if (clef3G_b == false) {
            //cout << "Recep va sortir SC sans message" << endl;
            pthread_mutex_unlock(myMutex);
         }
      }
      //cout << "> S Recep " << np++ << endl;
   }
}

/**
 * 
 * @param ordre
 */
void SIM::traitRecepSms(string ordre) {
   //bool autoris;
   bool mdpHS;
   stXm stX;
   int cr;
   char commande[50], value[10];
   size_t p = ordre.find("@");
   float teta = 0;
   string act = "", resul;
   ostringstream os;
   string mdp = ordre.substr(0, p);
   string action = ordre.substr(p + 1, ordre.length());
   //cout << "mdp=" << mdp << "  action=" << action << endl;
   //cout << "action=" << action << endl;

   if (ordre.find("mauvais") > 0 && ordre.find("mauvais") < 8) {
      act = "Ordre non compris";
      stX.type = 2; //sms avec 1 seul destinat
      stX.message = act;
      stX.destin = numReqSms;
      ////pushListXms(stX);
   } else {
      //autoris = adbdd->testpwdSMS(mdp);
      mdpHS = strcmp(mdpSms, mdp.c_str());
      if (!mdpHS) {
         std::transform(ordre.begin(), ordre.end(), ordre.begin(), ::tolower);
         etat = adbdd->getstate();
         //cout << "etat:" << etat << "  autoris=" << autoris << endl;
         if (ordre.find("arret") > 0 && ordre.find("arret") < 8) {
            if (Debug) {
               dt = dateL();
               printf("\n%s - Arret demande\n", dt.c_str());
            }
            pLog->wLine("\nsim.traitRecepSms", dt, "Arret demande");
            adbdd->updatestate(0);
            act = "Arret du Systeme";
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("marche") && ordre.find("marche") < 8) {
            if (Debug) {
               dt = dateL();
               printf("\n%s - Marche demandee\n", dt.c_str());
            }
            pLog->wLine("\nsim.traitRecepSms", dt, "Marche demande");
            adbdd->updatestate(1);
            act = "Mise en marche";
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("statut") && ordre.find("statut") < 8) {
            pLog->wLine("\nsim.traitRecepSms", dateL(), "Statut demande");
            if (etat == 1)
               act = "Systeme en marche";
            else {
               act = "Systeme a l'arret";
            }
            act += "\n";
            act += nivGsm();
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("heure") && ordre.find("heure") < 8) {
            pLog->wLine("\nsim.traitRecepSms", dateL(), "Heure demandee");
            dt = dateL();
            act = "H=" + dt;
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("posh") && ordre.find("posh") < 8) {
            dt = dateL();
            if (Debug)
               printf("\n%s - Chgt heure demande\n", dt.c_str());
            pLog->wLine("\nsim.traitRecepSms", dt, "Chgt d'heure demande");
            string cde = "sudo date -s \"";
            string heur = ordre.substr(ordre.find("posh") + 4, 19);
            cde += heur;
            cde += "\"";
            fprintf(stdout, "\nRemise à l'heure : ");
            fprintf(stdout, heur.c_str());
            fprintf(stdout, "\n");
            system(cde.c_str());
            system("sudo hwclock -w");
            usleep(500000);
            dt = dateL();
            act = "H=" + dt;
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("vmc0") && ordre.find("vmc0") < 8) {
            dt = dateL();
            if (Debug)
               printf("\n%s - Pilotage relai demande = 0\n", dt.c_str());
            pLog->wLine("\nsim.traitRecepSms", dt, "Pilotage relai demande");
            if (reseau.ping(ipRelaiESP)) {
               strcpy(value, "0");
               if (Debug)
                  printf("publication sur %s de 0\n", topicEmiss.c_str());
               cr = mosquitto_publish(leMosq, NULL, topicEmiss.c_str(), strlen(value), value, 0, false);
               if (cr) {
                  if (Debug) {
                     fprintf(stderr, "Can't publish to Mosquitto server : %d\n", cr);
                  }
               }
               adbdd->updateEtatRelai(0);
               act = "Arret VMC";
            } else
               act = "Relai pas present";
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("vmc1") && ordre.find("vmc1") < 8) {
            if (Debug) {
               dt = dateL();
               printf("\n%s - Pilotage relai demande = 1\n", dt.c_str());
            }
            if (reseau.ping(ipRelaiESP)) {
               if (Debug)
                  cout << "ping IP Interrupt OK";
               strcpy(value, "1");
               if (Debug)
                  printf("sim : publication sur %s de 1\n", topicEmiss.c_str());
               cr = mosquitto_publish(leMosq, NULL, topicEmiss.c_str(), strlen(value), value, 0, false);
               if (cr) {
                  if (Debug)
                     fprintf(stderr, "Can't publish to Mosquitto server\n");
               }
               adbdd->updateEtatRelai(1);
               act = "Marche VMC";
            } else
               act = "Relai pas present";
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("temp") && ordre.find("temp") < 8) {
            resul = adbdd->gettempToutes();
            act = "Temperatures :";
            act += resul;
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
         } else if (ordre.find("photo") && ordre.find("photo") < 8) {
            string nIm = ordre.substr(10, 2);
            //cout<<"Image N "<<nIm<<endl;
            pLog->wLine("\nsim.traitRecepSms", dateL(), "Photo demandee");
            if (nIm != " ") {
               string ficImg = adbdd->getLastImg(nIm);
               if (Debug)
                  fprintf(stderr, "img= %s\n", ficImg.c_str());
               string a_env = "/home/pi/Images/" + ficImg;
               stX.type = 1; //MMS
               stX.message = ficImg;
               stX.groupe = 1;
               stX.nomIm = ficImg;
               stX.chemIm = a_env;
               pushListXms(stX);
            } else {
               stX.type = 2; //SMS avec 1 seul destinat
               stX.message = "Pas d'image";
               stX.destin = numReqSms;
               pushListXms(stX);
            }
         } else if (ordre.find("reboot") && ordre.find("reboot") < 8) {
            pLog->wLine("\nsim.traitRecepSms", dateL(), "Reboot demande");
            act = "Reboot demande";
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = act;
            stX.destin = numReqSms;
            pushListXms(stX);
            system("reboot");
         } else {
            cout << dateL() << " : Ordre Inconnu" << endl;
            /*** On fait rien a cause des pub recues 
            stX.type = 2; //sms avec 1 seul destinat
            stX.message = "Ordre inconnu";
            stX.destin = numReqSms;
            pushListXms(stX);
             */
         }
      } else {
         stX.type = 2; //sms avec 1 seul destinat
         stX.message = "Mauvais mot de passe";
         stX.destin = numReqSms;
         pushListXms(stX);
      }
   }
}

void SIM::SendXMS() {
   stXm im;
   string dt;
   while (1) {
      if (!listXMS.empty()) {
         if (im.type == 0 || im.type == 1 || im.type == 2) {
            dt = dateL();
            im = listXMS.front();
            //printf("\n==> XMS");
            if (im.type == 0) { //SMS diffuse à tous
               if (std::time(0) - tpsSMS > 60) { // On ne peut envoyer un MMS que ttes les Xs
                  //printf(" - SMS\n");
                  //cout << "Depile : " << im.message << endl;
                  listXMS.pop_front(); //On dépile
                  tpsSMS = std::time(0);
                  diffSMS(im.message, to_string(im.groupe));
               }
            }
            int delaiMMS;
            if (Debug)
               delaiMMS = 120; //2mn
            else
               delaiMMS = 360; //6mn
            if (im.type == 1) { //MMS 
               if ((std::time(0) - tps) > delaiMMS) { // On ne peut envoyer un MMS que ttes les Xs
                  //printf(" - MMS\n");
                  //cout << "DEpile : " << im.message << endl;
                  listXMS.pop_front(); //On défile
                  tps = std::time(0);
                  if (cle3G == 0) {
                     diffMMS(im.message, to_string(im.groupe), im.nomIm, im.chemIm);
                  } else {
                     if (Debug)
                        printf("Clef 3G donc pas de MMS\n");
                  }
                  envImgFtp(im.chemIm);
               }
            }
            if (im.type == 2) { //SMS avec 1 seul destinat, réponse à requete
               //printf(" - SMS de reponse\n");
               //cout << "DEpile : " << im.message << endl;
               listXMS.pop_front(); //On dépile
               envSMS(im.destin, im.message);
            }
         }
      }
      usleep(500000);
   }
}

void SIM::pushListXms(stXm sti) {
   etat = adbdd->getstate();
   if ((etat == 1 || sti.message == "Arret du Systeme" || sti.message == "Systeme a l'arret" || sti.message.find("A l'arret")) && etatReseau) {
      //cout<<"empile : "<<sti.message<<endl;
      listXMS.push_back(sti);
   }
}

void SIM::repositRelai(int v) {
   char value[3];
   int cr;
   if (Debug)
      printf("sim - repositRelai éventuel : %d\n", v);
   usleep(500000);
   if (v == 1) {
      // on repositionne le relai
      strcpy(value, "1");
      if (ipRelaiESP != "") {
         if (reseau.ping(ipRelaiESP)) {
            if (Debug)
               printf("Publication: %s = %s\n", topicEmiss.c_str(), value);
            cr = mosquitto_publish(leMosq, NULL, topicEmiss.c_str(), strlen(value), value, 0, false);
            if (cr) {
               if (Debug) {
                  fprintf(stderr, "Can't publish to Mosquitto server : %d\n", cr);
                  perror("Err mosquito");
               }
            }
         } else {
            fprintf(stdout, "sim : ESP8266 pas present\n");
         }
         adbdd->updateEtatRelai(1);
      }
   }
   if (v == 0) {
      // on repositionne le relai a 0
      strcpy(value, "0");
      if (ipRelaiESP != "") {
         if (reseau.ping(ipRelaiESP)) {
            if (Debug)
               printf("Publication: %s = %s\n", topicEmiss.c_str(), value);
            cr = mosquitto_publish(leMosq, NULL, topicEmiss.c_str(), strlen(value), value, 0, false);
            if (cr) {
               if (Debug)
                  fprintf(stderr, "Can't publish to Mosquitto server\n");
            }
         }
         adbdd->updateEtatRelai(0);
      }
   }
}

int SIM::pingOK(char* pg) {
   return (reseau.ping(pg));
}

void SIM::getAdLog(Log* pl) {
   pLog = pl;
}

void SIM::setAdLog() {
   reseau.getAdLog(pLog);
   comp->getAdLog(pLog);
}
