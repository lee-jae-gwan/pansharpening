#pragma once
#ifndef PANSHARPENING_H
#define PANSHARPENING_H

#include "geotiffutil.h"


void geotiffwrite(float** red, float** green, float** blue, float** NIR, Geotiff pan, const char* outname);
void pansharpening_FIHS(Geotiff geotiffPan, float** NIR, float** red, float** green, float** blue, float** pan);
void pansharpening_brovey(Geotiff geotiffPan, float** NIR, float** red, float** green, float** blue, float** pan);
void resampling(const char* src_path, const char* dst_path, const char* result_folder);

#endif