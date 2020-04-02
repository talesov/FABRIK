#include "image/stb_image.h"
#include <glad/glad.h>

class Texture
{
public:
	static unsigned int LoadTextureFromFile(const char* path);
};
