#pragma once
#ifndef GEOTIFFUTIL_H
#define GEOTIFFUTIL_H
#define DIMGT 6


typedef struct {
	const char* filename;
	int xsize;
	int ysize;
	int nbands;
	double geotransform[6];
	double NoDataValue;
	char* projection;
	float** band;
}Geotiff;


Geotiff readGeotiff(const char*);
void readProjectionGeotiff(GDALDatasetH, Geotiff*);
void readDimensionsGeotiff(const char*, GDALDatasetH, Geotiff*);
void readBandGeotiff(const char*, GDALDatasetH, Geotiff*, int);
int writeGeotiff(Geotiff, float**, float**, float**, float**, const char*);

#endif