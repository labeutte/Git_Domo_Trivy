/* 
 * File:   Alarme433.cpp
 */
#include "alarme433.h"

#define Debug 0

alarme433::alarme433() {
	autorizRecep = 1;
	basc = 0;
	//SUR PROTO
	//PinAlim = 23; //Pin qui alimente la carte d'emission
	//PinTX = 25; //Pin Data
	//SUR CARTE 3.1
	//PinAlim = 31; //Pin qui alimente la carte d'emission
	//PinTX = 10; //Pin Data
	//PinRelai = 3; // Pin pilotage relai
	eResult = xmlDoc.LoadFile("/home/pi/Domotik/confAppli.xml");
	if (eResult != XML_SUCCESS) {
		std::cerr << "alarme433 : Erreur de chargement du fichier XML" << std::endl;
	}

	XMLNode* rootNode = xmlDoc.FirstChildElement("Domotik");
	if (rootNode == nullptr) {
		puts("Pas de noeud XML Domotik");
		pLog->wLine("alarme433", "","Pas de noeud XML Domotik");
	} else {
		element = rootNode->FirstChildElement("Relai");
		if (element != nullptr) {
			BUF = element->Attribute("marche");
			fMarche = atoi(BUF);
			BUF = element->Attribute("arret");
			fArret = atoi(BUF);
			BUF = element->Attribute("pinalim");
			PinAlim = atoi(BUF);
			BUF = element->Attribute("pintx");
			PinTX = atoi(BUF);
			BUF = element->Attribute("pinrelai");
			PinRelai = atoi(BUF);
		}
	}
	if (wiringPiSetup() == -1) {
		printf("wiringPiSetup failed, dans alarme433");
	}
	pinMode(PinAlim, OUTPUT);
	pinMode(PinRelai, OUTPUT);
	digitalWrite(PinAlim, LOW);
	digitalWrite(PinRelai, LOW);
	if (Debug)
		printf("\nalarme433 : freqMARCHE=%d - freqARRET=%d\n", fMarche, fArret);
}

short alarme433::getautorize() {
	return (autorizRecep);
}

void alarme433::sonore() {
	for (int i = 0; i < 5; i++) {
		if (basc == 0) { //On active le klaxon
			digitalWrite(PinRelai, HIGH);
			autorizRecep = 0;
			emmiss(fMarche); //Marche
			autorizRecep = 1;
			sleep(3);
			digitalWrite(PinRelai, LOW);
			autorizRecep = 0;
			emmiss(fArret); //Arret
			autorizRecep = 1;
			sleep(3);
		}
	}
}

/**
 * @param sig = code à emettre en DECIMAL
 * @return 
 */
bool alarme433::emmiss(int sig) {
	int protocol = 0; // A value of 0 will use rc-switch's default value
	int pulseLength = 0; // en microseconds
	if (wiringPiSetup() == -1)
		return false;
	//printf("Envoie 433 code[%d]\n", sig);
	if (protocol != 0) mySwitch.setProtocol(protocol);
	if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);

	// On alimente carte emmiss433
	digitalWrite(PinAlim, 1);
	usleep(10000);
	mySwitch.enableTransmit(PinTX);
	mySwitch.send(sig, 24); //24=long du mot
	//On coupe alim
	digitalWrite(PinAlim, 0);
	usleep(10000);
	usleep(200000); //pour temporiser
	return true;
}

void alarme433::getAdLog(Log* pl){
	pLog=pl;
}
