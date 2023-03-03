#include "External/tgaimage.h"
#include "texture.h"
#include "sampler.h"
#include "examples.h"

namespace TextureTests {
	void BasicTextureTest(int width, int height, Texture& myTexture)
	{
		TGAImage textureImage(width, height, TGAImage::RGB);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				const RGB& color = myTexture(x, y);
				textureImage.set(x, y, TGAColor(color.r, color.g, color.b, 1));
			}
		}
		textureImage.flip_vertically(); // so that origin (0,0) is bottom left, not top left
		textureImage.write_tga_file("texture_test.tga");
	}

	void SamplerClampWrappingTest(int height, int width, Sampler& mySampler, TGAImage& textureImageGrid)
	{
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
	}

	void SamplerMirrorWrappingTest(Sampler& mySampler, int height, int width, TGAImage& textureImageGrid)
	{
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
	}

	void SamplerRepeatWrappingTest(Sampler& mySampler, int height, int width, TGAImage& textureImageGrid)
	{
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
	}

	void SamplerFillWrappingTest(Sampler& mySampler, int height, int width, TGAImage& textureImageGrid)
	{
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
	}

	void SamplerNearestUpscalingTest(Sampler& mySampler, int height, int width, TGAImage& textureImageUpscaled)
	{
		mySampler.setSamplingMode(NEAREST);
		mySampler.setWrappingMode(FILL);
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
		textureImageUpscaled.write_tga_file("sampler_upscale_test_nearest.tga");
	}

	void SamplerBilinearUpscalingTest(Sampler& mySampler, int height, int width, TGAImage& textureImageUpscaled)
	{
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
	}

	int runTests()
	{
		std::string str = "Resources\\apples.jpg";
		Texture myTexture(str.c_str());
		int width = myTexture.get_width();
		int height = myTexture.get_height();

		// ----- Basic Texture functionality test -----
		BasicTextureTest(width, height, myTexture);

		// ----- Basic Sampler functionality -----
		Sampler mySampler(myTexture, NEAREST, CLAMPTOEDGE);
		TGAImage textureImageGrid(width * 3, height * 3, TGAImage::RGB);
		
		SamplerClampWrappingTest(height, width, mySampler, textureImageGrid);
		SamplerRepeatWrappingTest(mySampler, height, width, textureImageGrid);
		SamplerMirrorWrappingTest(mySampler, height, width, textureImageGrid);
		SamplerFillWrappingTest(mySampler, height, width, textureImageGrid);

		// ----- Sampler bilinear and nearest upscaling tests -----
		TGAImage textureImageUpscaled(width * 2, height * 2, TGAImage::RGB);

		SamplerNearestUpscalingTest(mySampler, height, width, textureImageUpscaled);
		SamplerBilinearUpscalingTest(mySampler, height, width, textureImageUpscaled);

		return 0;
	}
}