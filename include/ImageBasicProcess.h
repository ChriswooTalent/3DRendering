#ifndef _IMAGEBASICPROCESS_H_
#define _IMAGEBASICPRECESS_H_
 
#include "DataTypes.h"

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h> 
#include <conio.h>
#include <memory.h>

using namespace std;


extern "C" void SaveDicomDataToBMP(const short *DicomIn_put, const char *filename, int width, int height);
extern "C" void LoadImageFromBMP(const char *filename, UINT8 *data, int &channels);
extern "C" void LoadImageFromDat(const char *filename, UINT8 *data, int width, int height, int channels);

#endif
