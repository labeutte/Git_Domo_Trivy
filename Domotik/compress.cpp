/* 
 * File:   compress.cpp
 */
#include "compress.h"
#define Debug 0

using namespace cv;

compress::compress() {
}

void compress::compression(string imageSrc, string imageDst, int taille) {
    //string nomNoExt;
    //nomNoExt = imageSrc;
    //nomNoExt.erase(nomNoExt.end() - 4, nomNoExt.end());
    //nomfichier_dest = nomNoExt + "_conv.jpg";

    image = cv::imread(imageSrc, CV_LOAD_IMAGE_COLOR);
    taille_compression = taille;
    if (!image.data) {
        if (Debug)
            fprintf(stderr, " No image data \n ");
    }
    compression_param.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_param.push_back(taille_compression);

    try {
        cv::imwrite(imageDst, image, compression_param);
    } catch (runtime_error& ex) {
        fprintf(stderr, "Veillez convertir l'image au format JPG : %s\n", ex.what());
    }
}

void compress::getAdLog(Log* pl){
	pLog=pl;
}
