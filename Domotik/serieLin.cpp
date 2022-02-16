/****************************************************************************
 *  serieLin.cpp
 *  gestion liaison serie
 *  Version du 25/11/2020
 ****************************************************************************/
#include "serieLin.h"
#include <string>

#define Debug 0
#define couleur(param) fprintf(stderr,"\033[%sm",param)

serieLin::serieLin() {
	uartStream = -1;
}

serieLin::~serieLin() {
}
/*****************************************************************************/

/**
 * @param fd
 * @param should_block
 */
void serieLin::set_blocking(int fd, int should_block) {
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0) {
		fprintf(stderr, "error %d from tggetattr", errno);
		return;
	}
	tty.c_cc[VMIN] = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout
	if (tcsetattr(fd, TCSANOW, &tty) != 0)
		fprintf(stderr, "error %d setting term attributes", errno);
}
/*****************************************************************************/

/**
 * @param fd
 * @param speed
 * @param parity
 * @return 
 */
int serieLin::set_interface_attribs(int fd, int speed, int parity) {
	struct termios tty;
	if (tcgetattr(fd, &tty) < 0) {
		fprintf(stderr, "Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}
	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	cfsetospeed(&tty, (speed_t) speed);
	cfsetispeed(&tty, (speed_t) speed);
	tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls (rts/cts ...)
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8; // 8-bit characters 
	tty.c_cflag &= ~PARENB; // no parity bit 
	tty.c_cflag &= ~CSTOPB; // only need 1 stop bit 
	tty.c_cflag &= ~CRTSCTS; // no hardware flowcontrol
	//tty.c_cflag |= CRTSCTS; // valide hardware flowcontrol
	// setup for non-canonical mode c � dire mode Raw
	// lflag = input
	// oflag = output
	// On enleve tout
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | IGNPAR | IUCLC | ISTRIP | INLCR | IGNCR | ICRNL | IXON | NLDLY | CRDLY | OLCUC);
	//On remet
	//tty.c_iflag |= (NLDLY | CRDLY | PARMRK| IGNBRK  | IUCLC);
	tty.c_iflag |= (NLDLY | CRDLY | IGNBRK); // On supprime delai sur CR et NL et on ignore BRK en Entree
	////tty.c_iflag |= (IXON | IXOFF | IXANY); //valide Xon-Xoff
	//tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN | ECHOE );
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input
	tty.c_oflag &= ~OPOST; //mode raw output : When the OPOST option is 
	//disabled, all other option bits in c_oflag are ignored.
	/* VMIN sp�cifie le nombre minimum de caract�res � lire. Si elle est r�gl�e sur 0, la valeur 
	 * VTIME sp�cifie le temps d'attente pour chaque lecture de caract�res. Notez que cela ne 
	 * signifie pas qu'un appel de lecture pour N octets attendra l'entr�e de N caract�res. Au contraire, 
	 * le d�lai d'attente s'appliquera au premier caract�re et l'appel de lecture renverra le nombre de 
	 * caract�res imm�diatement disponibles (jusqu'� ce que vous demande).
	 * Si VMIN est non nul, VTIME sp�cifie le temps d'attente pour le premier caract�re lu. Si 
	 * un personnage est lu dans le d�lai imparti, toute lecture va bloquer (attendre) jusqu'� ce que 
	 * tous les caract�res VMIN soient lus. C'est-�-dire, une fois que le premier caract�re est lu, le 
	 * pilote de l'interface s�rie s'attend � recevoir un paquet complet de caract�res (octets VMIN 
	 * total). Si aucun caract�re n'est lu dans le d�lai imparti, alors l'appel � lire renvoie 0. 
	 * Cette m�thode vous permet de dire au pilote s�rie que vous avez besoin exactement de N octets 
	 * et tout appel lue renverra 0 ou N octets. Cependant, le d�lai d'attente s'applique uniquement 
	 * au premier caract�re lu, alors, si, pour une raison quelconque, le pilote manque un personnage 
	 * � l'int�rieur du paquet N byte, alors l'appel lue pourrait bloquer pour toujours des caract�res 
	 * d'entr�e suppl�mentaires.
	 * VTIME sp�cifie le temps n�cessaire pour attendre les caract�res entrants en dixi�mes de secondes. 
	 * Si VTIME est r�gl� sur 0 (par d�faut), les lectures bloqueront (attendez) ind�finiment, sauf 
	 * si l'option NDELAY est d�finie sur le port avec open ou fcntl.
	 */
	//A la place de setmincount()
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 10;
	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		fprintf(stderr, "Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}
/*****************************************************************************/

/**
 * @param fd
 * @param vmin
 * @param vtime
 */
void serieLin::set_mincount(int fd, int vmin, int vtime) {
	struct termios tty;
	if (tcgetattr(fd, &tty) < 0) {
		fprintf(stderr, "Error tcgetattr: %s\n", strerror(errno));
		return;
	}
	tty.c_cc[VMIN] = vmin ? 1 : 0;
	tty.c_cc[VTIME] = vtime; /* half second timer */
	if (tcsetattr(fd, TCSANOW, &tty) < 0)
		fprintf(stderr, "Error tcsetattr: %s\n", strerror(errno));
}
/*****************************************************************************/

/**
 * @param port
 * @return 
 */
int serieLin::openTTY(const char* port) {
	// At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) 
	// respectively
	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests 
	//  on the file can return immediately with a failure status
	//		if there is no input immediately available (instead of blocking). Likewise, write 
	//          requests can also return
	//		immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause 
	//  the terminal device to become the controlling terminal for the process.
	uartStream = open(port, O_RDWR | O_NOCTTY | O_NDELAY); //Open in non blocking read/write mode
	//uartStream = open (port, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);
	if (uartStream < 0) {
		fprintf(stderr, "error %d opening %s: %s", errno, port, strerror(errno));
		return (-1);
	}
	set_interface_attribs(uartStream, B115200, 0); // set speed to bps, 8n1 (no parity)
	//fcntl(uartStream, F_SETFL, fcntl(uartStream, F_GETFL)&~O_NONBLOCK); //bloquant
	//fcntl(uartStream, F_SETFL, fcntl(uartStream, F_GETFL)|O_NONBLOCK); // NON bloquant
	fcntl(uartStream, F_SETFL, 0);
	//set_mincount(uartStream, 1, 5); // set vmin= nb car attendu mini  et 
	// vtime temps maxi entre 2 car en 1/10 de seconde
	return (uartStream);
}
/*****************************************************************************/

/**
 * 
 */
void serieLin::closeTTY(void) {
	close(uartStream);
}
/*****************************************************************************/

/**
 * @param tx_buffer
 * @param lg_buffer
 * @return 
 */
int serieLin::writeTTY(char *tx_buffer, int lg_buffer) {
	int count = write(uartStream, tx_buffer, lg_buffer); //Filestream, bytes to write, number of bytes to write
	usleep(130 * lg_buffer);
	tcdrain(uartStream); //attend que toutes les écritures aient été transmises
	if (count != lg_buffer) {
		fprintf(stderr, "Error write No:%d ->Recus:%d, Envoyes:%d\n", errno, lg_buffer, count);
	}
	//    tcflush(uartStream, TCOFLUSH); ::Efface les données écrites mais non transmises
	return (count);
}

/*****************************************************************************/
int serieLin::writeImTTY(char *tx_buffer, int lg_buffer) {
	for (int w = 0; w < lg_buffer; w++) {
		write(uartStream, tx_buffer + w, 1);
		usleep(110);
	}
	tcdrain(uartStream); /* delay for output */
	//tcflush(uartStream, TCOFLUSH);
	usleep(110);
	return (1);
}
/*****************************************************************************/

/**
 * @param rx_buffer
 * @param nBytesToRead
 * @return 
 */
int serieLin::readTTY(char* rx_buffer, int nBytesToRead) {
	int nbytes = 0; /* Number of bytes read */
	int aLire = 0;
	int lg = 254;
	memset(buffer, '\0', lg);
	ioctl(uartStream, FIONREAD, &aLire); //donne le nb de car à lire
        usleep(400); //0.0005s
	if(Debug)
		printf("A lire : %d\n",aLire);
	nbytes = read(uartStream, buffer, lg);
	//for(int j=0; j<aLire;j++){
	//	read(uartStream, buffer+j, 1);
	//}
	strcpy(rx_buffer, buffer);
	return (nbytes);
}
/*****************************************************************************/

/**
 * sendAT : Envoi d'une commande AT et attente r�ponse
 * @param ATcommand
 * @param repAttendu
 * @param timeout
 * @return 
 */
string serieLin::sendAT(const char* ATcommand, const char* repAttendu, unsigned int timeout) {
	char bufW[256];
	char bufR[256];
	int nBytesRead = 0;
	int rep = 0;
	memset(bufW, '\0', 254);
	tcflush(this->uartStream, TCIOFLUSH); // vide les flux d'E/S pas encore lu/ecrit
	strcpy(bufW, ATcommand);
	couleur("36"); //Cyan
	if (!writeTTY(bufW, strlen((char*) bufW)))
		fprintf(stderr, "Erreur d'envoi sendAT\n");
	else {
		if (Debug)
			fprintf(stdout, "=>\t%s\n", (char*) bufW);
	}
	couleur("0");
	tcdrain(uartStream); //On attend la fin des flux
	/*******Le timeout est très important (laisser temps réponse*******/
	//usleep(strlen(bufW)*2000); // sleep enough to transmit the X plus
	usleep(timeout); // receive Y:  approx 100 uS per char transmit

	// On lit ensuite
	memset(bufR, '\0', 254);
	nBytesRead = readTTY(bufR, sizeof (bufR));
	couleur("33"); //Jaune
	if (nBytesRead > 0) {
		bufR[nBytesRead] = '\0';
		if (Debug)
			fprintf(stdout, "\t%s\n(%d)-------\n", bufR, nBytesRead);
	} else
		fprintf(stderr, "Erreur AT reception (0 car lu)\n");
	couleur("0");
	// Comparaison des donnees
	reponseAT = string(bufR);
	if (reponseAT.find(string(repAttendu))) {
		rep = 1;
		//cout<<"reponseAT="<<reponseAT<<endl;
	} else
		reponseAT = "";
	tcdrain(uartStream); //On attend la fin des flux
	tcdrain(1);
	tcflush(uartStream, TCIOFLUSH);
	//cout<<"reponseAT="<<reponseAT<<"|-"<<endl;
	return reponseAT;
}
