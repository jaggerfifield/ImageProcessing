//	==============================
//	Description:
//	Reads a png image into memory.
//	==============================
//	Author: Jai Fifield
//	Date: Sep 10, 2024
//	==============================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

void readPNG(char*);
unsigned int hex_to_decimal(unsigned char*, size_t);

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
	FILE* fp_image = NULL;				// An image file pointer
	uint8_t buffer_size = 32;			// The size of the data buffer for reading from
	unsigned int chunk_length = 0;		// When reading chunks we store the length here
	unsigned char data[buffer_size];	// The data buffer for reading from

	bool verbose = true;

	//PNG properties
	unsigned int height = 0, width = 0;

	// The PNG chunk types
	unsigned char PNG_signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	unsigned char IHDR_header[4] = {0x49, 0x48, 0x44, 0x52};

	memset(data, '\0', buffer_size);

	// Open the image file
	fp_image = fopen(path, "rb");

	if(fp_image == NULL){
		printf("Cound not open image: %s\n", path);
		return;
	}
	
	// Read the file, the first 8 bytes should be the PNG signature
	fread(data, 1, 8, fp_image);
	
	if(verbose)
		for(int i = 0; i < 8; i++)
			printf("\x1b[32m%.2x \x1b[0m", data[i]);

	if(memcmp(data, PNG_signature, 8) != 0){
		printf("\nPNG header not found in first 8 bytes!\n");
		return;
	}

	if(verbose)
		printf(" -> \x1b[32mPNG\x1b[0m signature found!\n");

	// Clear the data buffer
	memset(data, '\0', buffer_size);

	// Look for the IHDR chunk size (the next 4 bytes)
	fread(data, 1, 4, fp_image);

	chunk_length = hex_to_decimal((void*)data, 4);

	if(chunk_length <= 0){
		printf("Chunk length of 0!\n");
		return;
	}

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[31m%.2x \x1b[0m", data[i]);

	memset(data, '\0', buffer_size);

	// Read the chunk type (IHDR)
	fread(data, 1, 4, fp_image);

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[36m%.2x \x1b[0m", data[i]);

	if(memcmp(data, IHDR_header, 4) != 0){
		printf("-> \x1b[36mIHDR\x1b[0m chunk not found!\n");
		return;
	}
	if(verbose)
		printf(" -> \x1b[36mIHDR\x1b[0m chunk found! (\x1b[31mlength\x1b[0m: %d)\n", chunk_length);

	memset(data, '\0', buffer_size);

	// Read the IHDR info
	fread(data, 1, 4, fp_image);
	
	width = hex_to_decimal((void*)data, 4);

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[34m%.2x \x1b[0m", data[i]);

	memset(data, '\0', buffer_size);

	fread(data, 1, 4, fp_image);

	height = hex_to_decimal((void*)data, 4);

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[33m%.2x \x1b[0m", data[i]);
	
	if(verbose)
		printf(" -> \x1b[34mWidth\x1b[0m: %dpx, \x1b[33mHeight\x1b[0m: %dpx\n", width, height);

	return;
}

unsigned int hex_to_decimal(unsigned char* data, size_t size){
	int sum = 0;

	for(int i = size-1; i >= 0; i--){
		//printf("\n%d + %d * %d\n", sum, (uint32_t)data[i], (int)(pow(16, 2*(size - (i+1)))));
		sum = sum + ((uint32_t)data[i] * (int)(pow(16, 2*(size-(i+1)))));
	}

	return sum;
}
