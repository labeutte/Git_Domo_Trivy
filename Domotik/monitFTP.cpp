/* 
 * File:   monitFTP.cpp
 * Version du 25/11/2020
 */
#include "monitFTP.h"
#define Debug 0

//PROTOTYPES
int remove_directory(char const *name);

/**
 * FONCTION qui retoune une string en minuscule
 */
std::string str_tolower(std::string s) {
   std::transform(s.begin(), s.end(), s.begin(),
         [](unsigned char c) {
            return std::tolower(c); }
   );
   return s;
}

monitFTP::monitFTP() {
   posGeo = "";
   tpsMms = 0;
   trouvCam = false;
}

monitFTP::~monitFTP() {
}

void monitFTP::setAdSem(sem_t* ads) {
   semIntrus = ads;
}

/**
 * 
 * @param ads
 */
void monitFTP::setAdSim(SIM*ads) {
   maSim = ads;
}

/**
 * 
 * @param adb
 */
void monitFTP::setAdBdd(accesBDD* adb) {
   databdd = adb;
}

void monitFTP::setAdGpd(gestPipe* g) {
   gpd = g;
}

void monitFTP::setAdAlarme(alarme433* ad) {
   alarme = ad;
}

/**
 * Information sur le type d'evenement declencheur
 * @param inf
 */
void monitFTP::info(char *inf) {
   string chemIm, token;
   stXm stX;
   int gr;
   chemIm = string(inf);
   int pn = chemIm.find("-");
   if (pn < 2 || pn > 10)
      pn = 0;
   token = chemIm.substr(0, pn);
   databdd->alertcam(databdd->getidcam(token.c_str()), token);

   string act = "AlerteCam : ";
   act += posGeo;
   //act += " ";
   //act += dateL();
   act += " ->";
   act += string(inf);
   gr = 1;
   chemIm = string(repDestination);
   chemIm += "/";
   chemIm += string(inf);
   databdd->putLastImg(string(inf));

   //On declenche alarme sonore
   sem_post(semIntrus);

   //On diffuse le SMS en le mettant dans la list
   stX.type = 0; //sms
   stX.message = act;
   stX.groupe = gr;
   maSim->pushListXms(stX);

   //On diffuse le MMS en le mettant dans la list
   stX.type = 1; //MMS
   stX.message = string(inf);
   stX.groupe = gr;
   stX.nomIm = string(inf);
   stX.chemIm = chemIm;
   maSim->pushListXms(stX);

   //On publie sur Cayenne
   ech.type = 2;
   gpd->writePipeSort(&ech);
}

/**
 * 
 * @param source
 * @param destination
 * @return 
 */
void monitFTP::copier_fichier(char const * const source, char const * const destination) {
   string cmd;
   cmd = "mv " + string(source) + "  " + string(destination);
   //if (Debug)
   //    printf("cmd %s\n", cmd.c_str());
   system(cmd.c_str());
   usleep(200000);
   remove(source);
}

/**
 * 
 * @param repT
 */
void monitFTP::traitRep(char* repT) {
   DIR *rt;
   string pG = "inconnue";
   string buf;

   rt = opendir(repT);
   if (rt != NULL) {
      struct dirent *ent;
      usleep(200 * 1000);
      //cout << "monitFTP rep recu =" << (string) repT << endl;
      //On regarde si c'est un nom CamXX
      if (!trouvCam) {
         buf = str_tolower((string) (repT));
         int lgb = buf.find_last_of(repT, buf.length() - 5);
         buf = buf.substr(lgb, 5);
         if (buf.find("cam") == 0) {
            //printf("\t->On trouve sous rep:%s\n", buf.c_str());
            nomCam = buf;
            pG = databdd->getcamGeo(nomCam);
            if (Debug)
               cout << "monitFTP sous rep : pG=" << pG << "  rep=" << buf << endl;
            if (pG != "inconnue") {
               posGeo = pG;
            }
            trouvCam = true;
         }
      }
      //On parcours le rep
      do {
         ent = readdir(rt);
         usleep(1000);
         if (ent != NULL) {
            if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
               //printf("\t\tConsultation : %s\n", ent->d_name);
               if (ent->d_type == 4) {
                  if (Debug)
                     printf("sousRep (4) : %s\n", ent->d_name);
                  char repTemp[512];
                  strcpy(repTemp, repT);
                  strcat(repTemp, "/");
                  strcat(repTemp, ent->d_name);
                  adetruire.push_back(string(repTemp));
                  traitRep(repTemp); //Recursivite
               }
               if (ent->d_type == 8) {
                  if (Debug)
                     printf("ttRep sousFichier (8): %s\n", ent->d_name);
                  char repTemp[512];
                  strcpy(repTemp, repT);
                  strcat(repTemp, "/");
                  strcat(repTemp, ent->d_name);
                  traitFic(repTemp, ent->d_name);
               }

            }
         }
      } while (ent != NULL);
   }
   closedir(rt);
}

/**
 * Donne un nom a l'image avec la date
 */
string nomImageDate() {
   const char* nomMois[] = {"janv", "fev", "mar", "avr", "mai", "juin", "juil", "aout", "sept", "oct", "nov", "dec"};
   char locTime[30];
   time_t TH;
   struct tm* t;
   TH = time(NULL);
   t = localtime(&TH);
   sprintf(locTime, "%02u%s%04u_%02u:%02u:%02u.jpg", t->tm_mday, nomMois[t->tm_mon], 1900 + t->tm_year, t->tm_hour, t->tm_min, t->tm_sec);
   return (string(locTime));
}

/**
 * Copie l'image dans le rep de destination si on est en
 * mode "Marche"
 * chem : nom complet (image comprise)
 * nomFic : nom dse l'image 
 */
void monitFTP::traitFic(char* chem, char* nomFic) {
   int etatM;
   char dest[512];
   char nouveauNom[70];
   strcpy(dest, repDestination);
   strcat(dest, "/");
   strcpy(nouveauNom, nomCam.c_str());
   strcat(nouveauNom, "-");
   strcat(nouveauNom, nomImageDate().c_str());
   strcat(dest, nouveauNom);
   etatM = databdd->getstate();
   //	nomCam = ""; //pour les appels suivants
   if (etatM) {
      if (std::time(0) - tpsMms > 5) { // Au moins 5s entre 2 envois
         tpsMms = std::time(0); // a cause des fichiers multiples
         //			printf("\nCopie du fichier = %s\n", nomFic);
         //			cr = copier_fichier(chem, dest);
         info(nouveauNom);
      }
      printf("\nCopie du fichier = %s\n", nomFic);
      copier_fichier(chem, dest);
   } else {
      remove(chem);
   }
   //	posGeo = "";
   //effaceRep();
}

/**
 * efface les sous-repertoires de celui qui est Ã  monitorer
 */
void monitFTP::effaceRep() {
   //printf("==>> monitFTP adetruire taille: %d\n", adetruire.size());
   for (int i = adetruire.size(); i > 0; i--) {
      //printf("destruction: %s\n", adetruire[i - 1].c_str());
      //rmdir(adetruire[i - 1].c_str());
      //La fonction suivante efface rep meme non vide
      remove_directory(adetruire[i - 1].c_str());
      adetruire.pop_back();
   }
}

/**
 * 
 * @param repT
 */
void monitFTP::effaceRepDeb(const char* repT) {
   DIR* rt;

   rt = opendir(repT);
   if (rt != NULL) {
      struct dirent* ent;
      usleep(300 * 1000);
      while ((ent = readdir(rt)) != NULL) {
         if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
            if (ent->d_type == 4) {
               //printf("sousRep : %s\n", ent->d_name);
               char repTemp[512];
               strcpy(repTemp, repT);
               strcat(repTemp, "/");
               strcat(repTemp, ent->d_name);
               adetruire.push_back(string(repTemp));
               effaceRepDeb(repTemp);
            }
            if (ent->d_type == 8) {
               //printf("sousFichier : %s\n", ent->d_name);
               char repTemp[512];
               strcpy(repTemp, "sudo rm ");
               strcat(repTemp, repT);
               strcat(repTemp, "/*");
               //fprintf(stderr, "effaceRepDeb : %s\n", repTemp);
               system(repTemp);
            }
            effaceRep();
         }
      }
   }
}

/**
 * Thread de monitoring d'un repertoire
 * @param path = chemin a monitorer
 */
void monitFTP::metScan(string path, string destin) {
   int fd, wd;
   fd_set fds;
   char buffer[512];
   struct inotify_event *event;
   DIR* rac;
   string dt;
   std::ostringstream ostst;

   strcpy(repOrigine, path.c_str());
   strcpy(repDestination, destin.c_str());
   dt = dateL();
   printf("\033[32m %s -Monitoring : \033[00m \033[31m %s \033[00m\n", dt.c_str(), repOrigine);
   ostst.str("");
   ostst << "\033[32m Monitoring : \033[00m \033[31m" << repOrigine << "\033[00m";
   pLog->wLine("thread monitFTP", "", ostst.str());

   fd = inotify_init();
   if (fd < 0) {
      if (Debug)
         perror("inotify Initialisation : ");
   }
   // Surveillance du fichier/repertoire passe en parametre
   wd = inotify_add_watch(fd, repOrigine, IN_CREATE);
   if (wd < 0) {
      if (Debug)
         perror("inotify Watch : ");
   }
   /////
   while (1) {
      FD_ZERO(&fds);
      FD_SET(fd, &fds);
      if (select(fd + 1, &fds, NULL, NULL, 0) <= 0) {
         continue;
      }
      if (read(fd, buffer, sizeof (buffer)) < 0)
         if (Debug)
            perror("monitFTP.metScan - err read : ");
      event = (struct inotify_event *) buffer;
      ostst.str("");
      if (Debug) {
         ostst << "\n\033[36m==>Creation : " << event->name << "\033[00m";
         pLog->wLine("\nthread monitFTP", dateL(), ostst.str());
         printf("\n\033[36m==>Creation : %s\033[00m\n", event->name);
      }
      rac = opendir((const char*) repOrigine);
      if (rac != NULL) {
         struct dirent* ent;
         //int b = 1;
         while ((ent = readdir(rac)) != NULL) {
            if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
               //printf("->%d : %s, type : %d\n", b++, ent->d_name, ent->d_type);
               if (ent->d_type == 4) { //REPERTOIRE
                  //printf("metScan Repertoire : %s\n", ent->d_name);
                  char repTemp[512];
                  strcpy(repTemp, repOrigine);
                  strcat(repTemp, "/");
                  strcat(repTemp, ent->d_name);
                  adetruire.push_back(string(repTemp));
                  traitRep(repTemp);
               }
               if (ent->d_type == 8) { //FICHIER
                  if (Debug)
                     printf("metScan ->Fichier sous la racine: %s\n", ent->d_name);
                  char repTemp[512];
                  strcpy(repTemp, repOrigine);
                  strcat(repTemp, "/");
                  strcat(repTemp, ent->d_name);
                  traitFic(repTemp, ent->d_name);
               }
            }
         }
      }
      closedir(rac);
      nomCam = "";
      posGeo = "";
      trouvCam = false;
      effaceRep();
   }
   pthread_exit(0);
}

/**
 * date au format francais
 */
string monitFTP::dateL() {
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

/**
 * Efface un rep non vide !!!
 * @param name
 * @return 
 */
int remove_directory(char const *name) {
   DIR *directory; /* pointeur de répertoire */
   struct dirent *entry; /* représente une entrée dans un répertoire. */
   struct stat file_stat; /* informations sur un fichier. */

   /* Ce tableau servira à stocker le chemin d'accès complet
    * des fichiers et dossiers. Pour simplifier l'exemple,
    * je le définis comme un tableau statique (avec une taille
    * a priori suffisante pour la majorité des situations),
    * mais on pourrait l'allouer dynamiquement pour pouvoir
    * le redimensionner si jamais on tombait sur un chemin
    * d'accès démesurément long. */
   char buffer[1024] = {0};

   /* On ouvre le dossier. */
   directory = opendir(name);
   if (directory == NULL) {
      fprintf(stderr, "cannot open directory %s\n", name);
      return 0;
   }

   /* On boucle sur les entrées du dossier. */
   while ((entry = readdir(directory)) != NULL) {

      /* On "saute" les répertoires "." et "..". */
      if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
         continue;
      }

      /* On construit le chemin d'accès du fichier en
       * concaténant son nom avec le nom du dossier
       * parent. On intercale "/" entre les deux.
       * NB: '/' est aussi utilisable sous Windows
       * comme séparateur de dossier. */
      snprintf(buffer, 1024, "%s/%s", name, entry->d_name);

      /* On récupère des infos sur le fichier. */
      stat(buffer, &file_stat);
      /* J'ai la flemme de tester la valeur de retour, mais
       * dans un vrai programme il faudrait le faire :D */

      if (S_ISREG(file_stat.st_mode)) {
         /* On est sur un fichier, on le supprime. */
         remove(buffer);
      } else if (S_ISDIR(file_stat.st_mode)) {
         /* On est sur un dossier, on appelle cette fonction. */
         remove_directory(buffer);
      }
   }

   /* On ferme le dossier. */
   closedir(directory);

   /* Maintenant le dossier doit être vide, on le supprime. */
   remove(name);
   return 1;
}

void monitFTP::getAdLog(Log* pl) {
   pLog = pl;
}
