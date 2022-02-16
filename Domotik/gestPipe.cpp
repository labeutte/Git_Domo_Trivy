/* 
 * File:   gestPipe.cpp
 * Author: Gilles
 *  Version du 25/11/2020
*/
#include "gestPipe.h"
#define Debug 1

gestPipe::gestPipe() {
	//On efface les anciens pipes
//	system("sudo rm /home/pi/Domotik/ent");
//	system("sudo rm /home/pi/Domotik/sort");
//	usleep(200000);

	//Chemin absolu à cause des deux executables
	strcpy(pipE, "/home/pi/Domotik/ent");
	strcpy(pipS, "/home/pi/Domotik/sort");

	mknod(pipE, S_IFIFO | 0666, 0);
	mknod(pipS, S_IFIFO | 0666, 0);
	if ((fe = open(pipE, O_RDWR, 0)) == -1) {
		pLog->wLine("gestPipe", "", "err ouv pipe ent");
		if (Debug)
			perror("err ouv pipe ent : ");
	}
	if ((fs = open(pipS, O_RDWR, 0)) == -1) {
		pLog->wLine("gestPipe", "", "err ouv pipe sort");
		if (Debug) {
			perror("err ouv pipe sort : ");
			//printf("pipes open : %d ; %d\n", fe, fs);
		}
	}
	/*
	usleep(200000);
	system("sudo chown pi.pi /home/pi/Domotik/ent");
	system("sudo chown pi.pi /home/pi/Domotik/sort");
	system("sudo chmod gu+w /home/pi/Domotik/ent");
	system("sudo chmod gu+w /home/pi/Domotik/sort");
	*/
}

void gestPipe::writePipeSort(sEch* ss) {
	int cr;
	cr = write(fs, (char*) ss, sizeof (sEch));
	if (cr == 0) {
		pLog->wLine("gestPipe.writeS", "", "err write pipe sort");
		if (Debug)
			perror("erreur write pipe sort : ");
	} else {
		if (Debug)
			printf("write pipe Sort (type): %d\n", ss->type);
	}
}

void gestPipe::readPipeEnt(sEch* se) {
	read(fe, (char*) &linfo, sizeof (sEch));
	se->capt = linfo.capt;
	se->relai = linfo.relai;
	se->type = linfo.type;
	se->marche = linfo.marche;
}

void gestPipe::writePipeEnt(sEch* se) {
	int cr;
	cr = write(fe, (char*) se, sizeof (sEch));
	if (cr == 0) {
		pLog->wLine("gestPipe.writeE", "", "err write pipe ent");
		if (Debug)
			perror("erreur write pipe ent : ");
	} else {
		if (Debug)
			printf("write pipe Ent (type): %d\n", se->type);
	}

}

void gestPipe::readPipeSort(sEch* ss) {
	read(fs, (char*) &linfo, sizeof (sEch));
	ss->capt = linfo.capt;
	ss->relai = linfo.relai;
	ss->type = linfo.type;
	ss->marche = linfo.marche;
}

void gestPipe::getAdLog(Log* pl) {
	pLog = pl;
}
