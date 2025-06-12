#include "tgaimage.h"
#include <iostream>
#include <string>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

int main(int argc, char **argv)
{
	std::string str1 = argv[0];
	std::string outFile = str1 + ".output.tga";

	TGAImage image(100, 100, TGAImage::RGB);
	image.set(52, 41, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file(outFile.c_str());
	return 0;
}
