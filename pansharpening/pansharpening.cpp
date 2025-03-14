#include <iostream>
#include "gdal_priv.h"
#include "pansharpening.h"
#include "opencv2/opencv.hpp"
#include "gdal.h"
#include "GeotiffUtil.h"
#include "string"
#include "gdalwarper.h"
#include "cpl_conv.h"
#include "ogr_spatialref.h"
#include <string>


using namespace std;
using namespace cv;


void resampling(const char* src_path, const char* dst_path, const char* result_folder) {

	GDALDatasetH srcDataset;
	srcDataset = GDALOpen(src_path, GA_ReadOnly);
	CPLAssert(srcDataset != Null);

	const char* srcProjection;
	srcProjection = GDALGetProjectionRef(srcDataset);

	GDALDataType sourceDatatype;
	sourceDatatype = GDALGetRasterDataType(GDALGetRasterBand(srcDataset, 1));

	String outfname;
	int len = ((String)result_folder).length();
	String extension("_resampled_.tif");
	outfname = ((String)result_folder).substr(0, len - 4) + extension;

	cout << outfname << endl;

	const char* dst_filename = dst_path;
	GDALDatasetH dst_dataset;

	int dstrows, dstcols;
	double dstGeotransform[6];
	const char* dstProjection;

	dst_dataset = GDALOpen(dst_filename, GA_ReadOnly);
	dstcols = GDALGetRasterXSize(dst_dataset);
	dstrows = GDALGetRasterYSize(dst_dataset);
	GDALGetGeoTransform(dst_dataset, dstGeotransform);
	dstProjection = GDALGetProjectionRef(dst_dataset);

	void* handleTransformArg;
	handleTransformArg = GDALCreateGenImgProjTransformer(srcDataset, srcProjection, NULL, dstProjection, false, 0, 1);
	CPLAssert(handleTransformArg != NULL);

	double srcGeoTransform[6];
	int outnrows = 0;
	int outncols = 0;
	CPLErr eErr;

	eErr = GDALSuggestedWarpOutput(srcDataset, GDALGenImgProjTransform, handleTransformArg, srcGeoTransform, &outncols, &outnrows);
	GDALDestroyGenImgProjTransformer(handleTransformArg);

	GDALDriverH outHandleDriver;
	GDALDatasetH outDataset;

	outHandleDriver = GDALGetDriverByName("GTiff");
	outDataset = GDALCreate(outHandleDriver, outfname.c_str(), dstcols, dstrows, 1, sourceDatatype, NULL);
	GDALSetProjection(outDataset, dstProjection);
	GDALSetGeoTransform(outDataset, dstGeotransform);

	eErr = GDALReprojectImage(srcDataset, srcProjection, outDataset, dstProjection, GRA_Cubic, 0.0, 0.0, NULL, NULL, NULL);
	CPLAssert(eErr == CE_None);
	GDALClose(outDataset);

}




void pansharpening_brovey(Geotiff geotiffPan, float** NIR, float** red, float** green, float** blue, float** pan) {

	int nrows, ncols;
	int row, col;
	nrows = geotiffPan.ysize;
	ncols = geotiffPan.xsize;

	float** L = (float**)malloc(nrows * sizeof(float*));
	float** NIRsharp = (float**)malloc(nrows * sizeof(float*));
	float** redsharp = (float**)malloc(nrows * sizeof(float*));
	float** greensharp = (float**)malloc(nrows * sizeof(float*));
	float** bluesharp = (float**)malloc(nrows * sizeof(float*));
	float sum;

	for (row = 0; row < nrows; row++) {

		NIRsharp[row] = (float*)malloc(ncols * sizeof(float));
		redsharp[row] = (float*)malloc(ncols * sizeof(float));
		greensharp[row] = (float*)malloc(ncols * sizeof(float));
		bluesharp[row] = (float*)malloc(ncols * sizeof(float));

		for (col = 0; col < ncols; col++) {
			sum = (float)(red[row][col] + green[row][col] + blue[row][col] + NIR[row][col]);
			redsharp[row][col] = ((float)red[row][col] / sum) * ((float)pan[row][col]);
			greensharp[row][col] = ((float)green[row][col] / sum) * ((float)pan[row][col]);
			bluesharp[row][col] = ((float)blue[row][col] / sum) * ((float)pan[row][col]);
			NIRsharp[row][col] = ((float)NIR[row][col] / sum) * ((float)pan[row][col]);
		}
	}

	const char* outname = "brovey_panshrpened.tif";
	geotiffwrite(redsharp, greensharp, bluesharp, NIRsharp, geotiffPan, outname);

}


void pansharpening_FIHS(Geotiff geotiffPan, float** NIR, float** red, float** green, float** blue, float** pan) {

	//FIHS ±â¹ý Ææ»þÇÁ´×

	int nrows, ncols;
	int row, col;
	nrows = geotiffPan.ysize;
	ncols = geotiffPan.xsize;

	float** L = (float**)malloc(nrows * sizeof(float*));
	float** NIRsharp = (float**)malloc(nrows * sizeof(float*));
	float** redsharp = (float**)malloc(nrows * sizeof(float*));
	float** greensharp = (float**)malloc(nrows * sizeof(float*));
	float** bluesharp = (float**)malloc(nrows * sizeof(float*));


	for (row = 0; row < nrows; row++) {

		L[row] = (float*)malloc(ncols * sizeof(float));
		NIRsharp[row] = (float*)malloc(ncols * sizeof(float));
		redsharp[row] = (float*)malloc(ncols * sizeof(float));
		greensharp[row] = (float*)malloc(ncols * sizeof(float));
		bluesharp[row] = (float*)malloc(ncols * sizeof(float));

		for (col = 0; col < ncols; col++) {
			L[row][col] = ((float)(red[row][col] + green[row][col] + blue[row][col] + NIR[row][col])) / 4.0;
			redsharp[row][col] = red[row][col] + (pan[row][col] - L[row][col]);
			greensharp[row][col] = green[row][col] + (pan[row][col] - L[row][col]);
			bluesharp[row][col] = blue[row][col] + (pan[row][col] - L[row][col]);
			NIRsharp[row][col] = NIR[row][col] + (pan[row][col] - L[row][col]);

		}
	}
	const char* outname = "FIHS_panshrpened.TIF";
	geotiffwrite(redsharp, greensharp, bluesharp, NIRsharp, geotiffPan, outname);
}

void geotiffwrite(float** red, float** green, float** blue, float** NIR, Geotiff pan, const char* outname) {

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
	outDataset = GDALCreate(outHandleDriver, outname, ncols, nrows, 4, GDT_Float32, NULL);
	GDALSetGeoTransform(outDataset, geotransform);
	GDALSetProjection(outDataset, projection);

	float* scanLineRed = (float*)CPLMalloc(sizeof(float) * ncols);
	float* scanLineGreen = (float*)CPLMalloc(sizeof(float) * ncols);
	float* scanLineBlue = (float*)CPLMalloc(sizeof(float) * ncols);
	float* scanLineNIR = (float*)CPLMalloc(sizeof(float) * ncols);

	GDALRasterBandH handleRedBand;
	GDALRasterBandH handleBlueBand;
	GDALRasterBandH handleGreenBand;
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
		GDALRasterIO(handleRedBand, GF_Write, 0, row, ncols, 1, scanLineRed, ncols, 1, GDT_Float32, 0, 0);
		GDALRasterIO(handleGreenBand, GF_Write, 0, row, ncols, 1, scanLineGreen, ncols, 1, GDT_Float32, 0, 0);
		GDALRasterIO(handleBlueBand, GF_Write, 0, row, ncols, 1, scanLineBlue, ncols, 1, GDT_Float32, 0, 0);
		GDALRasterIO(handleNIRBand, GF_Write, 0, row, ncols, 1, scanLineNIR, ncols, 1, GDT_Float32, 0, 0);
	}
	GDALClose(outDataset);
}