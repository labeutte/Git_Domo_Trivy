#ifndef COMPRESS_H
#define COMPRESS_H
#include <vector>
#include <stdio.h>
//#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include "Log.h"

using namespace std;

class compress {
private:
    Log *pLog;
    string nomfichier_dest;
    cv::Mat image; //objet image
    vector<int> compression_param; //tableau pour les paramètres de compressions
    int taille_compression;
public:
    void compression(string, string, int);
    compress();
    void getAdLog(Log*);
};

#endif /* COMPRESS_H */

