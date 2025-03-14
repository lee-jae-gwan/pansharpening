#include <iostream>
#include "gdal_priv.h"
#include "pansharpening.h"
#include "gdal.h"
#include "GeotiffUtil.h"
#include "string"
#include "gdalwarper.h"
#include "cpl_conv.h"
#include "ogr_spatialref.h"
#include "pansharpening.h"


using namespace std;


int main() {

	GDALAllRegister();

	const char* path_arr[5];
	const char* result_path = "./";
	
	path_arr[0] = "B2.TIF";
	path_arr[1] = "B3.TIF";
	path_arr[2] = "B4.TIF";
	path_arr[3] = "B5.TIF";
	path_arr[4] = "B8.TIF";
	

	for(int i=0; i<4; i++){
		int len = string(path_arr[i]).length();
		string file_name = string(path_arr[i]).substr(49, len);
		string result_filename = string(result_path) + file_name;
		resampling(path_arr[i], path_arr[4], result_filename.c_str());
	}

	const char* resampled_b = "B2_resampled.tif";
	const char* resampled_g = "B3_resampled.tif";
	const char* resampled_r = "B4_resampled.tif";
	const char* resampled_nir = "B5_resampled.tif";

	Geotiff geotiffNIR, geotiffRed, geotiffGreen, geotiffBlue, geotiffPan;
	geotiffNIR = readGeotiff(resampled_nir);
	geotiffRed = readGeotiff(resampled_r);
	geotiffGreen = readGeotiff(resampled_g);
	geotiffBlue = readGeotiff(resampled_b);
	geotiffPan = readGeotiff(path_arr[4]);


	float** NIR = geotiffNIR.band;
	float** red = geotiffRed.band;
	float** green = geotiffGreen.band;
	float** blue = geotiffBlue.band;
	float** pan = geotiffPan.band;
	

	if (pan) {
		cout << "-------------------------start pansharpen------------------------------------------" << endl;
	}

	pansharpening_brovey(geotiffPan, NIR,red,green, blue, pan);
	pansharpening_FIHS(geotiffPan,NIR,red,green, blue, pan);


	return 0;
}
