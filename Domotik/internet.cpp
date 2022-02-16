#include "internet.h"
/*  Version du 25/11/2020 */

#define Debug 0

internet::internet() {
	attribText = (const char*) attribT;
	pid = -1;
	proto = NULL;
	cnt = 1;
	eResult = xmlDoc.LoadFile("/home/pi/Domotik/confAppli.xml");
	if (eResult != XML_SUCCESS) {
		if(Debug)
			puts("internet : Erreur de chargement du fichier XML");
		pLog->wLine("internet","","Erreur de chargement du fichier XML");
		std::cerr << "internet : Erreur de chargement du fichier XML" << std::endl;
	}

	XMLNode* rootNode = xmlDoc.FirstChildElement("Domotik");
	if (rootNode == nullptr) {
		pLog->wLine("internet","","Pas de noeud XML Domotik");
		puts("Pas de noeud XML Domotik");
	} else {
		element = rootNode->FirstChildElement("Ftp");
		if (element != nullptr) {
			attribText = element->Attribute("url");
			strcpy(urlSite, attribText);
			attribText = element->Attribute("logMdp");
			strcpy(logPwd, attribText);
		}
	}
	if (Debug)
		cout << "internet : " << urlSite << " - " << logPwd << endl;
}

/**
 * 
 * @param ad
 */
void internet::setAdBDD(accesBDD* ad) {
	adbdd = ad;
}

/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/

/*--------------------------------------------------------------------*/
unsigned short internet::checksum(void *b, int len) {
	unsigned short *buf = (unsigned short*) b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(unsigned char*) buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*    return 0 is ping not Ok, return 1 is ping OK.                ---*/

/*--------------------------------------------------------------------*/
int internet::ping(const char *adress) {
	const int val = 255;
	int i, sd, lenS, loop;

	pid = getpid();
	proto = getprotobyname("ICMP");
	hname = gethostbyname(adress);
	bzero(&addr_ping, sizeof (addr_ping));
	addr_ping.sin_family = hname->h_addrtype;
	addr_ping.sin_port = 0;
	addr_ping.sin_addr.s_addr = *(long*) hname->h_addr;

	addr = &addr_ping;
	sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if (sd < 0) {
		if (Debug)
			fprintf(stderr, "err socket");
		return 0;
	}
	if (setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof (val)) != 0) {
		if (Debug)
			fprintf(stderr, "err Set TTL option");
		return 0;
	}
	if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0) {
		if (Debug)
			fprintf(stderr, "err Request nonblocking I/O");
		return 0;
	}
	for (loop = 0; loop < 2; loop++) {
		lenS = sizeof (r_addr);

		if (recvfrom(sd, &pckt, sizeof (pckt), 0, (struct sockaddr*) &r_addr, (socklen_t*) & lenS) > 0) {
			return 1;
		}

		bzero(&pckt, sizeof (pckt));
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = pid;
		for (i = 0; i < (int) sizeof (pckt.msg) - 1; i++)
			pckt.msg[i] = i + '0';
		pckt.msg[i] = 0;
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof (pckt));
		if (sendto(sd, &pckt, sizeof (pckt), 0, (struct sockaddr*) addr, sizeof (*addr)) <= 0)
			if (Debug)
				fprintf(stderr, "err sendto");
		usleep(300000);
	}
	return 0;
}

void internet::putFTP(string img) {
	// PENSER à modifier le fichier /etc/ssl/openssl.cnf
	//Commenter ligne #CipherString = ...
	strcpy(site, "www.orange.fr");
	int r = ping(site);
	if (r) { // Il y a du réseau
		/*****FTP en ligne de Cde*****/
		string stftp;
		char command[250];
		if (Debug) {
			//stftp = "curl --user " + adbdd->getftplogin() + ':' + adbdd->getftpmdp() + " -T \"{" + img + "}\" --ftp-ssl-reqd -ssl " + adbdd->getftpadr() + "\n";
			stftp = "curl -v --user " + string(logPwd) + " -T \"{" + img + "}\" --ftp-ssl-reqd -ssl " + string(urlSite) + "\n";
			cout << "putFTP : " << stftp << endl;
		} else {
			stftp = "curl --user " + string(logPwd) + " -T \"{" + img + "}\" --ftp-ssl-reqd -ssl " + string(urlSite) + "\n";
		}
		strcpy(command, stftp.c_str());
		system(command);
		sleep(1);
		/***Fin ligne de commande***/

		/*****FTP AVEC CODAGE*****/
/*
		CURL *curl;
		CURLcode res;
		FILE *hd_src;
		struct stat file_info;
		curl_off_t fsize;
		struct curl_slist *headerlist = (struct curl_slist*) calloc(1, sizeof (struct curl_slist));
		int d = img.find("/", 13);
		strcpy(origin, img.c_str());
		string nomImg = img.substr(d + 1);
		strcpy(site, urlSite); //intermediaire pour ne pas modifier urlSite
		strcpy(urlFTP, strcat(site, nomImg.c_str()));
		if (Debug) {
			cout << "internet.putFTP image : " << nomImg <<endl;
			cout << "  ->url:" << urlFTP << "  ->log:" << logPwd << endl;
		}
		if (stat(origin, &file_info)) {
			if (Debug)
				fprintf(stderr, "FTP - Couldn't open '%s': %s\n", origin, strerror(errno));
		}
		// get the file size of the local file
		fsize = (curl_off_t) file_info.st_size;
		//printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

		hd_src = fopen(origin, "rb"); // get a FILE * of the same file

		// In windows, this will init the winsock stuff
		//curl_global_init(CURL_GLOBAL_ALL);

		curl = curl_easy_init(); // get a curl handle
		if (curl) {
			// enable verbose for easier tracing
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			// We activate SSL and we require it for both control and data
			curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
			//curl_easy_setopt(curl, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_SSL);
			// enable uploading
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
			// specify target
			curl_easy_setopt(curl, CURLOPT_URL, urlFTP);
			curl_easy_setopt(curl, CURLOPT_USERPWD, logPwd);
			curl_easy_setopt(curl, CURLOPT_PORT, 21);
			// pass in that last of FTP commands to run after the transfer
			curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);
			// now specify which file to upload
			curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
			// Set the size of the file to upload (optional).  If you give a *_LARGE
			// option you MUST make sure that the type of the passed-in argument is a
			// curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
			// make sure that to pass in a type 'long' argument.
			curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) fsize);
			// Now run off and do what you've been told!
			res = curl_easy_perform(curl);
			// Check for errors
			if (res != CURLE_OK)
				fprintf(stderr, "internet.putFTP : curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			// clean up the FTP commands list
			curl_slist_free_all(headerlist);
			// always cleanup
			curl_easy_cleanup(curl);
		}
		fclose(hd_src); // close the local file
		curl_global_cleanup();
*/
		/*****Fin FTP avec Codage*****/
		cout << endl << "Image envoyee sur Orange" << endl;
		pLog->wLine("\ninternet.putFTP","","Image envoyée sur orange");
	} else {
		if (Debug)
			fprintf(stderr, "internet.putFTP : Pas de réseau WAN");
	}
}

void internet::getAdLog(Log* pl){
	pLog=pl;
}
