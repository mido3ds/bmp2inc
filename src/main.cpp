/**
 *  Mahmoud Adas
 *  embeds 24bit-depth-color bitmaps into MASM inc file as 8bit-depth-color bmp
 **/
#include <string>
#include <vector>
#include "dirent.h"
#include "qdbmp.hpp"

using namespace std;

// each pixel is 8 bit (256 colors)
unsigned char* getImagePixels(string path, int* width, int* height) {
	BMP* bmp = BMP_ReadFile(path.c_str());
    if (!bmp || BMP_GetDepth(bmp) != 24) return nullptr;

	*width = BMP_GetWidth(bmp);
	*height = BMP_GetHeight(bmp);
	unsigned char* buffer = new unsigned char[(*width)*(*height)];

    unsigned char r, g, b;
    for (int x = 0; x < *width; x++) {
		for (int y = 0; y < *height; y++) {
			BMP_GetPixelRGB(bmp, x, y, &r, &g, &b);
            buffer[x+y*(*height)] = (r/32) << 5 | (g/32) << 2 | (b/64);
		}
	}

    BMP_Free(bmp);

    return buffer;
}

void writeInc(string path, unsigned char* bytes, int w, int h) {
    if (!bytes) return;

    string path2(path);
    path2 += ".inc";

    FILE* f = fopen(path2.c_str(), "w");

    fprintf(f, "%s_WIDTH equ %i\n%s_HEIGHT equ %i\n%s db ", path.c_str(), w, path.c_str(), h, path.c_str());
    for (long i = 0; i < w*h; i++) {
        fprintf(f, "%hhi,", bytes[i]);
    }
    fprintf(f, "\n");

    fclose(f);
}

bool isBmp(string path) {
    return path.find(".bmp") != string::npos;
}

string getCurrentDir() {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}

vector<string> getPathToDirFiles(string directory) {
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
	for (auto filePath : getPathToDirFiles(getCurrentDir())) {
		if (isBmp(filePath)) {
			int w, h;
			unsigned char* pixels = getImagePixels(filePath, &w, &h);
			if (pixels) {
				writeInc(filePath, pixels, w, h);
				delete pixels;
			}
		}
	}
}
