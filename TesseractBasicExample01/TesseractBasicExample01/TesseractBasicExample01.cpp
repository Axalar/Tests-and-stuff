#include "pch.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

int main()
{
	char *outText;

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init(NULL, "eng")) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}

	// Open input image with leptonica library
	Pix *image = pixRead("C:/Users/Usuario/vcpkg/buildtrees/tesseract/src/3.05.02-2e3743e8b7/testing/phototest.tif");
	api->SetImage(image);
	// Get OCR result
	outText = api->GetUTF8Text();
	printf("OCR output:\n%s", outText);

	// Destroy used object and release memory
	api->End();
	delete[] outText;
	pixDestroy(&image);

	return 0;
}