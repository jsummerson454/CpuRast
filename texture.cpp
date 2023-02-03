#include "texture.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture()
{
	m_width = 0;
	m_height = 0;
	m_data = nullptr;
}

Texture::Texture(const char* path)
{
	m_width = 0;
	m_height = 0;
	m_data = nullptr;
	load_texture(path);
}

Texture::~Texture()
{
	delete[] m_data;
}

// No bounds checking on retrival, so take same care as one would with a regular array access
const RGB& Texture::operator()(int x, int y) const
{
	return m_data[y * m_width + x];
}

void Texture::load_texture(const char* path)
{
	// if texture already loaded, delete old texture
	if (m_data != nullptr) {
		delete[] m_data;
		m_width = 0;
		m_height = 0;
	}

	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);
	uint8_t* data = stbi_load(path, &width, &height, &channels, 3);
	if (data == nullptr) {
		std::cout << "Error loading texture:" << path << std::endl;
		return;
	}

	m_width = width;
	m_height = height;
	m_data = new RGB[width * height];
	for (int i = 0; i < width * height; i++) {
		m_data[i] = RGB{ data[3 * i], data[3 * i + 1], data[3 * i + 2] };
	}

	stbi_image_free(data);
}

// Copy constructor
Texture::Texture(const Texture& toCopy)
{
	m_width = toCopy.m_width;
	m_height = toCopy.m_height;
	m_data = new RGB[m_width * m_height];
	memcpy(m_data, toCopy.m_data, sizeof(RGB) * m_width * m_height);
}

// Assignment operator
Texture& Texture::operator=(const Texture& toCopy)
{
	// if texture already loaded, delete old texture
	if (m_data != nullptr) {
		delete[] m_data;
		m_width = 0;
		m_height = 0;
	}

	m_width = toCopy.m_width;
	m_height = toCopy.m_height;
	m_data = new RGB[m_width * m_height];
	memcpy(m_data, toCopy.m_data, sizeof(RGB) * m_width * m_height);
	return *this;
}