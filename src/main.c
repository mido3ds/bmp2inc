/**
 *  Mahmoud Adas
 *  embeds 24bit-depth-color bitmaps into MASM inc file as 8bit-depth-color bmp
 **/
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>
#include "qdbmp/qdbmp.h"

#define getcwd _getcwd
#define open _open
#define close _close

// each pixel is 8 bit (256 colors)
unsigned char* getImagePixels(const unsigned char* path, int* width, int* height) {
	BMP* bmp = BMP_ReadFile(path);
    if (!bmp || BMP_GetDepth(bmp) != 24) return NULL;

	*width = BMP_GetWidth(bmp);
	*height = BMP_GetHeight(bmp);
    unsigned char* buffer = malloc((*width)*(*height));

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

bool isBmp(char* path, int len) {
    return path && len>0 && strcmp(path+len-4, ".bmp") == 0;
}

void writeInc(const char* path, int len, unsigned char* bytes, int w, int h) {
    if (!path || !bytes) return;

    char path2[len+4 +1];
    sprintf(path2, "%s.inc", path);

    printf(path2);
    FILE* f = fopen(path2, "w");

    fprintf(f, "%s_WIDTH equ %i\n%s_HEIGHT equ %i\n%s db ", path, w, path, h, path);
    for (long i = 0; i < w*h; i++) {
        fprintf(f, "%hhi,", bytes[i]);
    }
    fprintf(f, "\n");

    fclose(f);
}

int main(int argc, char const *argv[]) {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    DIR* dir;
    struct dirent* ent;
    dir = opendir(cwd);
    while ((ent = readdir(dir)) != NULL) {
        if (isBmp(ent->d_name, ent->d_namlen)) {
            int w,h;
            char* bytes = getImagePixels(ent->d_name, &w, &h);
            if (bytes) {
                writeInc(ent->d_name, ent->d_namlen, bytes, w, h);
                printf(ent->d_name);
                free(bytes);
            }
        }
    }

    closedir (dir);
    return 0;
}
