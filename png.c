//	==============================
//	Description:
//	Reads a png image into memory.
//	==============================
//	Author: Jai Fifield
//	Date: Sep 10, 2024
//	==============================

#include <stdio.h>
#include <string.h>
#include <stdint.h>

void readPNG(char*);

int main(int argc, char* argv[]){
	uint8_t path = 0; 

	for(int i = 0; i < argc; i++){
			if(strcmp(argv[i], "-p") == 0){
				path = i + 1;
			}
	}
	
	if(path == 0){
		printf("Cound not find path!\n");
		return 1;
	}

	readPNG(argv[path]);

	return 0;

}

void readPNG(char* path){
	FILE* fp_image = NULL;
	unsigned char data[32];
	unsigned char header[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

	memset(data, '\0', 32);

	fp_image = fopen(path, "rb");

	if(fp_image == NULL){
		printf("Cound not open image: %s\n", path);
		return;
	}
	
	printf("\nReading the first 8 bytes:\n");
	fread(data, 1, 8, fp_image);
	
	for(int i = 0; i < 8; i++){
		printf("%.2x ", data[i]);
	}

	if(memcmp(data, header, 8) != 0){
		printf("\nPNG header not found in first 8 bytes!\n");
		return;
	}

	printf(" -> PNG header found!\n");


	return;
}
