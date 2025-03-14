#include <stdio.h>
#include <stdlib.h>
#include "gdal.h"
#include "cpl_conv.h"
#include "geotiffutil.h"
#include <string>


Geotiff readGeotiff(const char* fname) {

    GDALAllRegister();
    GDALDatasetH geotiffReader;
    GDALDriverH geotiffDriver;
    Geotiff GeotiffObj;

    GeotiffObj.filename = fname;
    geotiffReader = GDALOpen(fname, GA_ReadOnly);
    geotiffDriver = GDALGetDatasetDriver(geotiffReader);


    readDimensionsGeotiff(fname, geotiffReader, &GeotiffObj);
    readProjectionGeotiff(geotiffReader, &GeotiffObj);
    readBandGeotiff(fname, geotiffReader, &GeotiffObj, 1);
    return GeotiffObj;

}

void readBandGeotiff(
    const char* filename,
    GDALDatasetH reader,
    Geotiff* tiff,
    int bandNumber
) {

    int ncols, nrows;
    int row;
    int col;

    GDALRasterBandH handleBand;
    float** outBand;
    handleBand = GDALGetRasterBand(reader, bandNumber);

   
    ncols = tiff->xsize;
    nrows = tiff->ysize;

    outBand = (float**)malloc(nrows * sizeof(float*));
    GDALDataType datatype;
    datatype = GDALGetRasterDataType(GDALGetRasterBand(reader, bandNumber));
    unsigned short* scanLine = (unsigned short*)CPLMalloc(sizeof(unsigned short) * ncols);

    for (row = 0; row < nrows; row++) {
        outBand[row] = (float*)malloc(ncols * sizeof(float));
        GDALRasterIO(
            handleBand, GF_Read, 0, row, ncols, 1,
            scanLine, ncols, 1,
            datatype, 0, 0
        );

        for (col = 0; col < ncols; col++) {
            outBand[row][col] = (float)scanLine[col];
        }
    }

    CPLFree(scanLine);
    tiff->band = outBand;

}


void readProjectionGeotiff(
    GDALDatasetH reader,
    Geotiff* tiff
) {

    double gt[DIMGT];
    double* pGT; 
    char* projection;
    GDALGetGeoTransform(reader, gt);
    pGT = &gt[0]; 
    tiff->projection = (char*)GDALGetProjectionRef(reader);

    int counter; 
    counter = 0;
    while (counter < DIMGT) {
        tiff->geotransform[counter] = *(pGT + counter);
        counter++;
    }
}



void readDimensionsGeotiff(
    const char* filename,
    GDALDatasetH reader,
    Geotiff* tiff
) {

    int xdim, ydim;
    int nbands;
    xdim = GDALGetRasterXSize(reader);
    ydim = GDALGetRasterYSize(reader);
    nbands = GDALGetRasterCount(reader);

    tiff->xsize = xdim;
    tiff->ysize = ydim;
    tiff->nbands = nbands;

}


int writeGeotiff(
    Geotiff pan,
    float** red,
    float** green,
    float** blue,
    float** NIR,
    const char* outname
) {

    char* projection = pan.projection;
    double geotransform[DIMGT];
    GDALDriverH outHandleDriver;
    GDALDatasetH outDataset;
    int nrows, ncols;
    int row, col;
    ncols = pan.xsize;
    nrows = pan.ysize;

   
    int count;
    count = 0;
    while (count < DIMGT) {
        geotransform[count] = pan.geotransform[count];
        count++;
    }

    outHandleDriver = GDALGetDriverByName("GTiff");
    outDataset = GDALCreate(outHandleDriver,
        outname,
        ncols, nrows, 4,
        GDT_Float32, NULL);
    GDALSetGeoTransform(outDataset, geotransform);
    GDALSetProjection(outDataset, projection);


    float* scanLineRed = (float*)CPLMalloc(sizeof(float) * ncols);
    float* scanLineGreen = (float*)CPLMalloc(sizeof(float) * ncols);
    float* scanLineBlue = (float*)CPLMalloc(sizeof(float) * ncols);
    float* scanLineNIR = (float*)CPLMalloc(sizeof(float) * ncols);


    GDALRasterBandH handleRedBand;
    GDALRasterBandH handleGreenBand;
    GDALRasterBandH handleBlueBand;
    GDALRasterBandH handleNIRBand;

    handleRedBand = GDALGetRasterBand(outDataset, 1);
    handleGreenBand = GDALGetRasterBand(outDataset, 2);
    handleBlueBand = GDALGetRasterBand(outDataset, 3);
    handleNIRBand = GDALGetRasterBand(outDataset, 4);

    for (row = 0; row < nrows; row++) {

        for (col = 0; col < ncols; col++) {
            scanLineRed[col] = red[row][col];
            scanLineGreen[col] = green[row][col];
            scanLineBlue[col] = blue[row][col];
            scanLineNIR[col] = NIR[row][col];
        }

        GDALRasterIO( 
            handleRedBand, GF_Write, 0, row, ncols, 1,
            scanLineRed, ncols, 1,
            GDT_Float32, 0, 0
        );

        GDALRasterIO( 
            handleGreenBand, GF_Write, 0, row, ncols, 1,
            scanLineGreen, ncols, 1,
            GDT_Float32, 0, 0
        );

        GDALRasterIO( 
            handleBlueBand, GF_Write, 0, row, ncols, 1,
            scanLineBlue, ncols, 1,
            GDT_Float32, 0, 0
        );

        GDALRasterIO( 
            handleNIRBand, GF_Write, 0, row, ncols, 1,
            scanLineNIR, ncols, 1,
            GDT_Float32, 0, 0
        );


    }

    GDALClose(outDataset);
    return 0;
}

