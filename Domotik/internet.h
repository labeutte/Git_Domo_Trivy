#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include "accesBDD.h"
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tinyxml2.h>
#include "Log.h"

using namespace tinyxml2;
using namespace std;

#define PACKETSIZE  64

struct packet {
    struct icmphdr hdr;
    char msg[PACKETSIZE - sizeof (struct icmphdr)];
};

class internet {
    Log *pLog;
    int pid;
    struct protoent *proto;
    struct packet pckt;
    struct sockaddr_in r_addr;
    struct hostent *hname;
    struct sockaddr_in addr_ping, *addr;
    int cnt;
    accesBDD* adbdd;
    XMLDocument xmlDoc;
    XMLElement *element, *item;
    XMLError eResult;
    const char* attribText;
    char attribT[30];
    char origin[40];
    char urlFTP[100];
    char urlSite[70];
    char logPwd[70];
    char site[60];

public:
    internet();
    void getAdLog(Log*);
    unsigned short checksum(void*, int);
    int ping(const char*);
    void putFTP(string);
    void setAdBDD(accesBDD*);
};
