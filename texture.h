#pragma once
#include <cstdint>

struct RGB {
	uint8_t r, g, b;
};

class Texture {
private:
	int m_width, m_height;
	RGB* m_data;
public:
	Texture();
	~Texture();
	Texture(const char* path);
	const RGB& operator()(int x, int y) const;
	void load_texture(const char* path);

	Texture(const Texture& toCopy);
	Texture& operator=(const Texture& toCopy);

	int get_height() { return m_height; }
	int get_width() { return m_width; }
};