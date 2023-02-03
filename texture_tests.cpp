#include "External/tgaimage.h"
#include "texture.h"
#include "sampler.h"

namespace TextureTests {
	int runTests()
	{
		// ----- Basic Texture functionality -----
		std::string str = "Resources\\apples.jpg";
		Texture myTexture(str.c_str());
		int width = myTexture.get_width();
		int height = myTexture.get_height();
		TGAImage textureImage(width, height, TGAImage::RGB);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				const RGB& color = myTexture(x, y);
				textureImage.set(x, y, TGAColor(color.r, color.g, color.b, 1));
			}
		}
		textureImage.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImage.write_tga_file("texture_test.tga");

		// ----- Basic Sampler functionality -----
		TGAImage textureImageGrid(width * 3, height * 3, TGAImage::RGB);
		Sampler mySampler(myTexture, NEAREST, CLAMPTOEDGE);
		for (int y = 0; y < height * 3; y++) {
			for (int x = 0; x < width * 3; x++) {
				float x_sample = float(x - width) / width;
				float y_sample = float(y - height) / height;
				glm::vec3 col = mySampler(x_sample, y_sample);
				col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
				textureImageGrid.set(x, y, TGAColor(col.x, col.y, col.z, 1));
			}
		}
		textureImageGrid.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImageGrid.write_tga_file("sampler_test_clamp.tga");

		mySampler.setWrappingMode(REPEAT);
		for (int y = 0; y < height * 3; y++) {
			for (int x = 0; x < width * 3; x++) {
				float x_sample = float(x - width) / width;
				float y_sample = float(y - height) / height;
				glm::vec3 col = mySampler(x_sample, y_sample);
				col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
				textureImageGrid.set(x, y, TGAColor(col.x, col.y, col.z, 1));
			}
		}
		textureImageGrid.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImageGrid.write_tga_file("sampler_test_repeat.tga");

		mySampler.setWrappingMode(MIRROR);
		for (int y = 0; y < height * 3; y++) {
			for (int x = 0; x < width * 3; x++) {
				float x_sample = float(x - width) / width;
				float y_sample = float(y - height) / height;
				glm::vec3 col = mySampler(x_sample, y_sample);
				col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
				textureImageGrid.set(x, y, TGAColor(col.x, col.y, col.z, 1));
			}
		}
		textureImageGrid.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImageGrid.write_tga_file("sampler_test_mirror.tga");

		mySampler.setWrappingMode(FILL);
		for (int y = 0; y < height * 3; y++) {
			for (int x = 0; x < width * 3; x++) {
				float x_sample = float(x - width) / width;
				float y_sample = float(y - height) / height;
				glm::vec3 col = mySampler(x_sample, y_sample);
				col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
				textureImageGrid.set(x, y, TGAColor(col.x, col.y, col.z, 1));
			}
		}
		textureImageGrid.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImageGrid.write_tga_file("sampler_test_fill.tga");

		// ----- Sampler bilinear and nearest upscaling tests -----
		TGAImage textureImageUpscaled(width * 2, height * 2, TGAImage::RGB);
		mySampler.setSamplingMode(NEAREST);
		mySampler.setWrappingMode(FILL);
		for (int y = 0; y < height * 2; y++) {
			for (int x = 0; x < width * 2; x++) {
				float x_sample = (float) x / (2*width);
				float y_sample = (float) y / (2*height);
				glm::vec3 col = mySampler(x_sample, y_sample);
				col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
				textureImageUpscaled.set(x, y, TGAColor(col.x, col.y, col.z, 1));
			}
		}
		textureImageUpscaled.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImageUpscaled.write_tga_file("sampler_upscale_test_nearest.tga");


		mySampler.setSamplingMode(BILINEAR);
		for (int y = 0; y < height * 2; y++) {
			for (int x = 0; x < width * 2; x++) {
				float x_sample = (float)x / (2 * width);
				float y_sample = (float)y / (2 * height);
				glm::vec3 col = mySampler(x_sample, y_sample);
				col = col * glm::vec3(255) + glm::vec3(0.5); // convert from [0.f,1.f] colourspace to [0, 255] for TGAColor
				textureImageUpscaled.set(x, y, TGAColor(col.x, col.y, col.z, 1));
			}
		}
		textureImageUpscaled.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImageUpscaled.write_tga_file("sampler_upscale_test_bilinear.tga");

		return 0;
	}
}