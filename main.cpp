#include "tgaimage.h"
#include <iostream>
#include <string>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

// Bresenham’s Line Drawing Algorithm
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
	for (float step = 0; step < 1; step += 0.01)
	{
		int x = x0 + step * (x1 - x0);
		int y = y0 + step * (y1 - y0);
	}
}

int main(int argc, char **argv)
{
	std::string str1 = argv[0];
	std::string outFile = str1 + ".output.tga";

	TGAImage image(100, 100, TGAImage::RGB);
	// image.set(30, 30, red);
	// image.set(30, 30, red);
	// 45度显示不出来
	// line(30, 30, 70, 70, image, white);
	line(30, 30, 70, 70, image, white);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file(outFile.c_str());
	return 0;
}
