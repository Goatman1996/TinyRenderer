#include "tgaimage.h"
#include <iostream>
#include <string>
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

Model *model = NULL;
const int width = 800;
const int height = 800;

// Bresenham’s Line Drawing Algorithm
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
	bool swaped = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		// dy > dx
		// 则交换 x 和 y，交换后，后面继续使用x进行遍历
		std::swap(x0, y0);
		std::swap(x1, y1);
		swaped = true;
	}
	if (x0 > x1)
	{
		// 如果起点比终点大，会进不去遍历
		// 所以交换位置，以便后面继续从小到大的遍历规则
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	// 我们要遍历x，所以每个要填色的像素的x时确定的
	// 根据斜率，可以计算出一个dt，用于计算每一个x步长所对应的y的变化
	int dx = x1 - x0;
	int dy = y1 - y0;
	// （1）下面几步展示了计算过程的简化，省略了所有的浮点运算，和（2）一起看
	// float dt = std::abs(dy / static_cast<float>(dx));
	// float dt = std::abs(dy / static_cast<float>(dx) * 2);
	// float dt = std::abs(dy / static_cast<float>(dx) * 2 * dx);
	int dt = std::abs(dy * 2);

	int y = y0;
	int crease = 0;
	for (int x = x0; x <= x1; x++)
	{
		// 第0个点是正确的在像素上的，可以直接填色
		// 交换过x 和 y的，交换回来
		if (swaped)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}

		crease += dt;
		// y的增长超过半个像素，即认为y也更新到了下个像素
		// （2）下面几步展示了计算过程的简化，和（1）一起看
		// if (crease > 0.5f)
		// if (crease > 1)
		if (crease > dx)
		{
			// (3)
			// crease -= 1;
			crease -= 2 * dx;
			if (y1 > y0)
				y += 1;
			else
				y -= 1;
		}
	}
}

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
{
	line(p0.x, p0.y, p1.x, p1.y, image, color);
}

// 计算点P在三角形pts[0], pts[1], pts[2]中的重心坐标
// 叉乘可以，等写完这章要验证一下
Vec3f barycentric(Vec2i *pts, Vec2i P)
{
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1)
		return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(Vec2i *pts, TGAImage &image, TGAColor color)
{
	// 找到AABB
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}

	// 遍历包围盒，使用重心坐标，判断是否在三角形内
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
				continue;
			image.set(P.x, P.y, color);
		}
	}
}

void triangle(Vec2i p0, Vec2i p1, Vec2i p2, TGAImage &image, TGAColor color)
{
	Vec2i pts[3] = {p0, p1, p2};
	triangle(pts, image, color);
}

int main(int argc, char **argv)
{
	std::string outFile = argv[1];

	TGAImage image(width, height, TGAImage::RGB);

	// 加载模型
	model = new Model(argv[2]);

	auto light = Vec3f(0, 0, -1); // 光源方向
	Vec2i screen_Cord[3];
	Vec3f world_Cord[3];
	// 遍历所有面
	for (int i = 0; i < model->nfaces(); i++)
	{
		// 遍历三角形三条边
		std::vector<int> face = model->face(i);

		for (int j = 0; j < 3; j++)
		{
			// 按顺序读，两两顶点作为一条边
			float scale = 1;
			Vec3f v0 = model->vert(face[j]) * scale;
			// Vec3f v1 = model->vert(face[(j + 1) % 3]) * scale;
			// 移动到图片中心
			// 假设了模型中顶点的范围是[-1, 1]
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			// int x1 = (v1.x + 1.) * width / 2.;
			// int y1 = (v1.y + 1.) * height / 2.;
			// line(x0, y0, x1, y1, image, white);
			// triangle(pts, image, white);
			screen_Cord[j] = Vec2i(x0, y0);
			world_Cord[j] = v0;
		}

		// 法线
		auto normal = (world_Cord[2] - world_Cord[0]) ^ (world_Cord[1] - world_Cord[0]);
		normal = normal.normalize();
		auto intensity = normal * light; // 光照强度

		if (intensity > 0)
		{
			triangle(screen_Cord, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, intensity * 255));
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file(outFile.c_str());
	return 0;
}
