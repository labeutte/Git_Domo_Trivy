/* 
 * File:   structEch.h
 * Author: Gilles
 */

#ifndef STRUCTECH_H
#define STRUCTECH_H
struct ECH {
    short relai;
    short capt;
    short type; //type d'alerte 
              // 0 : rien
              // 1 : 433
              // 2 : camera
    short marche;
};
typedef struct ECH sEch;
#endif /* STRUCTECH_H */

