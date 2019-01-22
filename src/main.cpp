/**
 *  Mahmoud Adas
 *  embeds 24bit-depth-color bitmaps into MASM inc file as 8bit-depth-color bmp
 **/

#include <string>
#include <vector>

#ifdef _WIN32
	#include "win_dirent.h"
#elif __APPLE__ or __linux__ or __unix__ or defined(_POSIX_VERSION)
	#include <dirent.h>
#else
	#error "Unknown compiler"
#endif

#include "qdbmp.hpp"

using namespace std;

struct Image {
	vector<unsigned char> buffer;
	int w, h;

	Image(int w, int h) :w(w), h(h), buffer(w*h) {}
};

// each pixel is 8 bit (256 colors)
static Image* convertTo256ColorImage(string path) {
	BMP* bmp = BMP_ReadFile(path.c_str());
    if (!bmp || BMP_GetDepth(bmp) != 24) return nullptr;

	Image* image = new Image(BMP_GetWidth(bmp), BMP_GetHeight(bmp));

    unsigned char r, g, b;
    for (int x = 0; x < image->w; x++) {
		for (int y = 0; y < image->h; y++) {
			BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
            image->buffer[x+y*(image->h)] = (r/32) << 5 | (g/32) << 2 | (b/64);
		}
	}

    BMP_Free(bmp);

    return image;
}

static bool writeInc(string file, Image* image) {
    if (!image) return false;

    FILE* f = fopen(string(file + ".inc").c_str(), "w");
	if (!f) return false;

    fprintf(f, "%s_WIDTH equ %i\n%s_HEIGHT equ %i\n%s db ", file.c_str(), image->w, file.c_str(), image->h, file.c_str());
    for (long i = 0; i < image->w*image->h; i++) {
        fprintf(f, "%hhi,", image->buffer[i]);
    }
    fprintf(f, "\n");

    return fclose(f) == 0;
}

static bool isBmp(string path) {
	return path.size() >= 4 && path.substr(path.size() - 4) == ".bmp";
}

static string getCurrentDir() {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}

static vector<string> getDirFiles(string directory) {
	DIR* dir;
	struct dirent* ent;
	dir = opendir(directory.c_str());
	vector<string> files;
	while ((ent = readdir(dir)) != NULL) {
		files.push_back(string(ent->d_name));
	}
	closedir(dir);
	return files;
}

int main() {
	string dir = getCurrentDir();
	for (auto file : getDirFiles(dir)) {
		if (isBmp(file)) {
			Image* image = convertTo256ColorImage(string(dir + '\\' + file));
			if (image) {
				if (!writeInc(file, image)) {
					fprintf(stderr, "ERROR, couldn't write include file");
				}
				delete image;
			} else {
				fprintf(stderr, "ERROR, couldn't open file[%s] or read it\n", file.c_str());
			}
		}
	}
}
