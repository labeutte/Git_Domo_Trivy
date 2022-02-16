/**
 * @file domoCayenne.cpp
 * Application domotique.
 * Version 23/01/2021
 * G Aucaigne
 */
#include <iostream>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include "Cayenne/MQTTLinux.h"
#include "Cayenne/CayenneMQTTClient.h"
#include "../Domotik/accesBDD.h"
#include "../Domotik/gestPipe.h"
#include "../Domotik/Log.h"
#include <tinyxml2.h>

#define Debug 0

using namespace tinyxml2;
using namespace std;

XMLDocument xmlDoc;
XMLElement *element, *item;
XMLError eResult;
Log mlog;
std::ostringstream oss;
const char* attribText;
char attribT[30];

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[50]; // = "00f66980-8126-11e9-ace6-4345fcc6b81e";
char password[50]; // = "43a7ae8aabc694f8cb51a526609c5354a8249f3e";
char clientID[50]; // = "911804c0-4b87-11ea-b301-fd142d6c1e6c";
// freq de publication Cayenne (recup dans XML)
   int tpsTemp = 0; //1mn=60000
   int tpsEtat = 0; 

MQTTNetwork ipstack;
CayenneMQTT::MQTTClient<MQTTNetwork, MQTTTimer> mqttClient(ipstack, username, password, clientID);
bool finished = false;
accesBDD abdd;
gestPipe gpc;
sEch sp, ss;
string dh, dmf, dhf;
int error = 0;
int tCaptTemp[10]; //Il ne peut y avoir que 10 catpeurs de temp maxi

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

/**
 * @param str chaine à spliter
 * @param delim delimieur
 * @return vecteur des éléments
 */
vector<string> split(const string& str, const string& delim) {
   vector<string> tokens;
   size_t prev = 0, pos = 0;
   do {
      pos = str.find(delim, prev);
      if (pos == string::npos) pos = str.length();
      string token = str.substr(prev, pos - prev);
      if (!token.empty()) tokens.push_back(token);
      prev = pos + delim.length();
   } while (pos < str.length() && prev < str.length());
   return tokens;
}

/**
 * Traitement approprie au message recu.
 * @param[in] message The message received from the Cayenne server.
 */
void traitementMessage(CayenneMQTT::MessageData& message) {
   if (message.topic == COMMAND_TOPIC) {
      //printf(" Domocayenne - channel= %d", message.channel);
      /*
      for (size_t i = 0; i < message.valueCount; ++i) {
              if (message.getValue(i)) {
                      printf(" value=%s ", message.getValue(i));
              }
              printf("\n");
      }
       */

      if (message.channel == 3) { // Commande Relai
         if (Debug)
            printf("domoCayenne : Pilotage Relai\n");
         if (atoi(message.getValue(0)) == 1) {
            // On met à 1
            sp.type = 11;
            sp.relai = 1;
         } else {
            // On met à 0
            sp.type = 11;
            sp.relai = 0;
         }
         oss.str("");
         oss << "Pilotage relai : " << sp.relai;
         mlog.wLine("\ntrtMessage", dateL(), oss.str());
         //On envoie sur le pipe
         gpc.writePipeEnt(&sp);
      }

      if (message.channel == 9) { // Marche/Arret appli
         if (Debug)
            printf("domoCayenne : Marche/Arret\n");
         if (atoi(message.getValue(0)) == 1) {
            sp.type = 12;
            sp.marche = 1;
         } else {
            sp.type = 12;
            sp.marche = 0;
         }
         oss.str("");
         oss << "Marche / Arret : " << sp.marche;
         mlog.wLine("\ntrtMessage", dateL(), oss.str());
         //On envoie sur le pipe
         gpc.writePipeEnt(&sp);
      }

      if (message.channel == 2) { // Aquittement alarme
         if (Debug)
            printf("domoCayenne : Acquit alarme\n");
         mqttClient.publishData(DATA_TOPIC, 7, NULL, NULL, 0);
         mqttClient.publishData(DATA_TOPIC, 8, NULL, NULL, 0);
         mqttClient.publishData(DATA_TOPIC, 6, NULL, NULL, 0);
         mqttClient.publishData(DATA_TOPIC, 2, NULL, NULL, 0);
         mlog.wLine("\ntrtMessage", dateL(), "Acquit alarme");
      }
   }
}

/**
 * Print the message info.
 * @param[in] message The message received from the Cayenne server.
 */
void outputMessage(CayenneMQTT::MessageData& message) {
   switch (message.topic) {
      case COMMAND_TOPIC:
         if (Debug)
            printf("topic=Command");
         break;
      case CONFIG_TOPIC:
         if (Debug)
            printf("topic=Config");
         break;
      default:
         if (Debug)
            printf("topic=%d", message.topic);
         break;
   }
   if (Debug)
      printf(" channel=%d", message.channel);
   /*
   if (message.clientID) {
           printf(" clientID=%s", message.clientID);
   }
   if (message.type) {
           printf(" type=%s", message.type);
   }
    */
   for (size_t i = 0; i < message.valueCount; ++i) {
      if (message.getValue(i)) {
         if (Debug)
            printf(" value=%s", message.getValue(i));
      }
      if (message.getUnit(i)) {
         if (Debug)
            printf(" unit=%s", message.getUnit(i));
      }
   }
   if (Debug)
      printf("\n");
}

/**
 * Handle messages received from the Cayenne server.
 * @param[in] message The message received from the Cayenne server.
 */
void messageArrived(CayenneMQTT::MessageData& message) {
   // Add code to process the message
   //outputMessage(message);
   traitementMessage(message);

   if (message.topic == COMMAND_TOPIC) {
      // If this is a command message we publish a response to show we recieved it. Here we are just sending a default 'OK' response.
      // An error response should be sent if there are issues processing the message.
      if ((error = mqttClient.publishResponse(message.id, NULL, message.clientID)) != CAYENNE_SUCCESS) {
         printf("Response failure, error: %d\n", error);
      }

      // Send the updated state for the channel so it is reflected in the Cayenne dashboard. If a command is successfully processed
      // the updated state will usually just be the value received in the command message.
      if ((error = mqttClient.publishData(DATA_TOPIC, message.channel, NULL, NULL, message.getValue())) != CAYENNE_SUCCESS) {
         printf("Publish state failure, error: %d\n", error);
      }
   }
}

/**
 * Connect to the Cayenne server.
 * @return Returns CAYENNE_SUCCESS if the connection succeeds, or an error code otherwise.
 */
int connectClient(void) {
   // Connect to the server.
   if (Debug)
      printf("Connecting to %s:%d\n", CAYENNE_DOMAIN, CAYENNE_PORT);
   while ((error = ipstack.connect(CAYENNE_DOMAIN, CAYENNE_PORT)) != 0) {
      if (finished)
         return error;
      if (Debug)
         printf("TCP connect failed, error: %d\n", error);
      sleep(3);
   }

   if ((error = mqttClient.connect()) != MQTT::SUCCESS) {
      printf("MQTT connect failed, error: %d\n", error);
      return error;
   }
   //printf("Connected\n");

   // Subscribe to required topics.
   if ((error = mqttClient.subscribe(COMMAND_TOPIC, CAYENNE_ALL_CHANNELS)) != CAYENNE_SUCCESS) {
      printf("Subscription to Command topic failed, error: %d\n", error);
   }
   if ((error = mqttClient.subscribe(CONFIG_TOPIC, CAYENNE_ALL_CHANNELS)) != CAYENNE_SUCCESS) {
      printf("Subscription to Config topic failed, error:%d\n", error);
   }

   // Send device info. Here we just send some example values for the system info. These should be changed to use actual system data, or removed if not needed.
   mqttClient.publishData(SYS_VERSION_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL, CAYENNE_VERSION);
   mqttClient.publishData(SYS_MODEL_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL, "Linux");
   mqttClient.publishData(SYS_CPU_MODEL_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL, "CPU Model");
   mqttClient.publishData(SYS_CPU_SPEED_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL, "1000000000");
   return CAYENNE_SUCCESS;
}

/**
 * Initialisation de la page cayenne
 */
void initVue() {
   // Check that we are still connected, if not, reconnect.
   if (!ipstack.connected() || !mqttClient.connected()) {
      ipstack.disconnect();
      mqttClient.disconnect();
      if (Debug)
         printf("Reconnecting\n");
      while (connectClient() != CAYENNE_SUCCESS) {
         if (finished)
            return;
         if (Debug)
            printf("Reconnect failed, retrying\n");
      }
   }
   //puts("--> Initialisation");
   //Initialisation des widgets d'alarme
   if ((error = mqttClient.publishData(DATA_TOPIC, 6, NULL, NULL, 0)) != CAYENNE_SUCCESS) {
      if (Debug)
         printf("Publish initialisation failed, error: %d\n", error);
   }
   mqttClient.publishData(DATA_TOPIC, 7, NULL, NULL, 0);
   mqttClient.publishData(DATA_TOPIC, 8, NULL, NULL, 0);
   mqttClient.publishData(DATA_TOPIC, 2, NULL, NULL, 0);
}

/**
 * Main loop where MQTT code is run.
 */
void loop(void) {
   static float teta = 0;
   static int marche = 0;
   time_t temps;
   struct tm datetime;
   char AAAA[5];
   char MM[3];
   char JJ[3];
   char HH[3];
   char MN[3];

   // Start the countdown timer for publishing data every X seconds
   //tpsTemp et tpsEtat sont en global récupérés dans XML (1mn = 60000)
   MQTTTimer timerTemp(tpsTemp); //Celui des températures
   MQTTTimer timerEtat(tpsEtat); //Celui de l'état

   while (!finished) {
      // Yield to allow MQTT message processing.
      mqttClient.yield(1000);

      // Check that we are still connected, if not, reconnect.
      if (!ipstack.connected() || !mqttClient.connected()) {
         ipstack.disconnect();
         mqttClient.disconnect();
         if (Debug)
            printf("Reconnecting\n");
         while (connectClient() != CAYENNE_SUCCESS) {
            if (finished)
               return;
            if (Debug)
               printf("Reconnect failed, retrying\n");
         }
      }

      // Publish some example data every few seconds. This should be changed to send your actual data to Cayenne.
      if (timerEtat.expired()) {
         marche = abdd.getstate();
         //On publie sur Cayenne
         if ((error = mqttClient.publishData(DATA_TOPIC, 0, NULL, NULL, marche)) != CAYENNE_SUCCESS) {
            if (Debug)
               printf("Publish marche failed, error: %d\n", error);
         } //else
         //printf("Etat= %d\n", marche);
         // Restart the countdown timerTemp for publishing data every X seconds.
         // Change the timeout parameter to publish at a different interval.
         timerEtat.countdown_ms(tpsEtat);
      }
      if (timerTemp.expired()) {
         marche = abdd.getstate();

         //recup date
         time(&temps);
         datetime = *localtime(&temps);
         strftime(AAAA, 5, "%Y", &datetime);
         strftime(MM, 3, "%m", &datetime);
         strftime(JJ, 3, "%d", &datetime);
         strftime(HH, 3, "%H", &datetime);
         strftime(MN, 3, "%M", &datetime);
         //printf("pub= %s - %s\n", strcat(AAAA, strcat(MM, JJ)), strcat(HH, MN));


         //On publie sur Cayenne
         if ((error = mqttClient.publishData(DATA_TOPIC, 0, NULL, NULL, marche)) != CAYENNE_SUCCESS) {
            if (Debug)
               printf("Publish failed, error: %d\n", error);
         } else {
            int ix = 0;
            while (tCaptTemp[ix] != 0 && ix < 10) {
               teta = abdd.gettemp(tCaptTemp[ix]);
               if (Debug)
                  printf("Temp N° %d = %5.2f\n", tCaptTemp[ix], teta);
               mqttClient.publishData(DATA_TOPIC, tCaptTemp[ix], TYPE_TEMPERATURE, UNIT_CELSIUS, teta);
               ix++;
            }
            mqttClient.publishData(DATA_TOPIC, 4, NULL, NULL, strcat(AAAA, strcat(MM, JJ)));
            mqttClient.publishData(DATA_TOPIC, 5, NULL, NULL, strcat(HH, MN));
            // Restart the countdown timerTemp for publishing data every X seconds
            timerTemp.countdown_ms(tpsTemp);
         }
      }
   }
}

/**
 * Interrupt handler for processing program shutdown.
 */
void intHandler(int signum) {
   puts("CTRL C = Arret");
   finished = true;
}

/**
 * Thread de surveillance d'arrivée d'un pipe de Domotik (sort)
 */
void* survP(void*) {
   int err = 0;
   char typeAl[7];
   time_t temps;
   struct tm datetime;
   char AAAA[5];
   char MM[3];
   char JJ[3];
   char HH[3];
   char MN[3];

   while (1) {
      //if (Debug)
      //	puts("domoCayenne (thread) : Attente pipe");
      gpc.readPipeSort(&ss);
      // Il y a une alerte !!!
      sprintf(typeAl, "%d", ss.type);
      if (Debug)
         printf("\t->Cayenne - Alerte type : %d\n", ss.type);
      //On publie l'alarme
      //recup date
      time(&temps);
      datetime = *localtime(&temps);
      strftime(AAAA, 5, "%Y", &datetime);
      strftime(MM, 3, "%m", &datetime);
      strftime(JJ, 3, "%d", &datetime);
      strftime(HH, 3, "%H", &datetime);
      strftime(MN, 3, "%M", &datetime);
      if ((err = mqttClient.publishData(DATA_TOPIC, 6, NULL, NULL, typeAl)) != CAYENNE_SUCCESS) {
         if (Debug)
            printf("Publish Alarme failed, error: %d\n", err);
      }
      mqttClient.publishData(DATA_TOPIC, 7, NULL, NULL, strcat(AAAA, strcat(MM, JJ)));
      mqttClient.publishData(DATA_TOPIC, 8, NULL, NULL, strcat(HH, MN));
   }
   pthread_exit(0);
}

/**
 * Main function.
 */
int main() {
   int pp;
   char numCh[40];
   attribText = (const char*) attribT;
   //Initialisation du tableau des capteur à scruter pour publication sur Cayenne
   for (int a = 0; a < 9; a++) {
      tCaptTemp[a] = 0;
   }
   //Fichier de log
   std::ofstream ficlog("/home/pi/DomoCayen.log", ios::app); //ouverture en ajout
   mlog.setOutput(ficlog);
   if (!ficlog.is_open())
      cout << "domoCayenne : Impossible d'ouvrir le fichier de log !" << endl;

   //Recup des identifiants Cayenne
   eResult = xmlDoc.LoadFile("/home/pi/Domotik/confAppli.xml");
   //eResult = xmlDoc.LoadFile("confAppli.xml");
   usleep(20000);
   if (eResult != XML_SUCCESS) {
      std::cerr << "\t->Cayenne : Erreur de chargement du fichier XML" << std::endl;
      perror("erreurxml ");
   }

   XMLNode* rootNode = xmlDoc.FirstChildElement("Domotik");
   if (rootNode == nullptr) {
      puts("Pas de noeud XML Domotik");
   } else {
      element = rootNode->FirstChildElement("Cayenne");
      if (element != nullptr) {
         attribText = element->Attribute("numCaptTemp");
         strcpy(numCh, attribText);
         attribText = element->Attribute("username");
         strcpy(username, attribText);
         attribText = element->Attribute("password");
         strcpy(password, attribText);
         attribText = element->Attribute("clientID");
         strcpy(clientID, attribText);
         attribText = element->Attribute("freqTemp");
         tpsTemp = atoi(attribText) * 1000;
         attribText = element->Attribute("freqEtat");
         tpsEtat = atoi(attribText) * 1000;
      }
   }
   if (Debug) {
      printf("\t->Cayenne :\n\tuser\t%s\n\tpass\t%s\n\tclId\t%s\n", username, password, clientID);
      printf("\tTempérature à publier : %s\n", numCh);
   }

   //Récupération de la liste des capteurs de température à scruter
   string conf(numCh);
   vector<string> vc = split(conf, "/");
   int num;
   for (int a = 0; a < (int) vc.size(); a++) {
      num = stoi(vc[a]);
      tCaptTemp[a] = num;
   }

   // Set up handler so we can exit cleanly when Ctrl-C is pressed.
   struct sigaction action;
   memset(&action, 0, sizeof (struct sigaction));
   //action.sa_handler = intHandler;
   //sigaction(SIGINT, &action, NULL);
   // Ignore the SIGPIPE signal since the program will attempt to reconnect if there are socket errors.
   action.sa_handler = SIG_IGN;
   sigaction(SIGPIPE, &action, NULL);

   pthread_attr_t thread_attr;
   pthread_t survPipe;

   // Set the default function that receives Cayenne messages.
   mqttClient.setDefaultMessageHandler(messageArrived);

   if (pthread_attr_init(&thread_attr) != 0) {
      fprintf(stderr, "pthread_attr_init error");
      exit(1);
   }
   if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) != 0) {
      fprintf(stderr, "pthread_attr_setdetachstate error");
      exit(1);
   }

   //creation du thread de surveillance du pipe
   pp = pthread_create(&survPipe, NULL, survP, NULL);
   if (pp != 0)
      perror("err lancement thread : ");

   // Connect to Cayenne.
   if (connectClient() == CAYENNE_SUCCESS) {
      initVue();
      // Run main loop.
      if (Debug)
         printf("\t->Cayenne loop ...\n");
      loop();
   } else {
      printf("\t->Connection Cayenne failed\n");
   }

   if (mqttClient.connected())
      mqttClient.disconnect();
   if (ipstack.connected())
      ipstack.disconnect();
   return 0;
}


