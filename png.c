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
#include <errno.h>

#include <windows.h>
#include <wchar.h>

typedef enum { PRINT_HEX } order_t;

typedef struct pOrder{
	order_t type;
	unsigned int fg_color;
	unsigned int bg_color;
	unsigned char* data;
	char* auxdata;
	unsigned int count;
	size_t size;
}pOrder;

void readPNG(char*, bool);
unsigned int hex_to_decimal(unsigned char*, size_t);
int colorize();
void printer(pOrder* order);

void file_test( void ){
	FILE* fp = fopen("./t.png", "rb");
	pOrder order;

	order.count = 0;
	order.auxdata = "HI";

	int color = 31;

	if(fp == NULL)
		return;

	fseek(fp, 0, SEEK_END);
	printf("Size of file is: %lu bytes.\n", ftell(fp));
	int x = fseek(fp, 0, SEEK_SET);

	if(x != 0)
		printf("Error");

	x = fseek(fp, 8192, SEEK_SET);

	if(x != 0)
		printf("Error");
	/*
	while(feof(fp) == 0){
		printf("Current location: %lu ", ftell(fp));
		fread(data, 1, 32, fp);

		order.type = PRINT_HEX;
		order.data = data;
		order.size = 32;
		order.fg_color = color;
		order.bg_color = 0;
		printer(&order);

		color += 1;
		if(color > 40)
			color = 31;

	}
*/
}

int main(int argc, char* argv[]){
	unsigned int path = 0; 

	// Fix windows terminal colors
	if(colorize() != 0)
		return -1;

	//file_test();

	// Look for a image path in arguments
	for(uint8_t i = 0; i < argc; i++)
		if(strcmp(argv[i], "-p") == 0)
			path = i + 1;
	if(path == 0){
		printf("Cound not find path!\n");
		return 1;
	}

	// Start reading the image
	readPNG(argv[path], true);

	return 0;
}

void readPNG(char* path, bool verbose){
	FILE* fp_image = NULL;				// An image file pointer
	unsigned int buffer_size = 0xFFFF;	// The size of the data buffer for reading from
	size_t chunk_length = 0;			// When reading chunks we store the length here
	unsigned char data[buffer_size];	// The data buffer for reading from
	
	bool running = true; 				// the loop control

	pOrder order;
	order.count = 0;
	order.auxdata = (char*)malloc(256);
	memset(order.auxdata, '\0', 256);

	unsigned int chunk_types = 13;

	// The PNG chunk types
	unsigned char PNG_signature[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
	unsigned char IHDR_chunk[5] = {0x49, 0x48, 0x44, 0x52, '\0'};
	unsigned char PLTE_chunk[5] = {0x50, 0x4c, 0x54, 0x45, '\0'};
	unsigned char IDAT_chunk[5] = {0x49, 0x44, 0x41, 0x54, '\0'};
	unsigned char IEND_chunk[5] = {0x49, 0x45, 0x4e, 0x44, '\0'};

	// Other chunks (Ignored)
	unsigned char tRNS_chunk[5] = {0x74, 0x52, 0x4e, 0x53, '\0'};
	unsigned char cHRM_chunk[5] = {0x63, 0x48, 0x52, 0x4d, '\0'};
	unsigned char gAMA_chunk[5] = {0x67, 0x41, 0x4d, 0x41, '\0'};
	unsigned char iCPP_chunk[5] = {0x69, 0x43, 0x43, 0x50, '\0'};
	unsigned char sBIT_chunk[5] = {0x73, 0x42, 0x49, 0x54, '\0'};
	unsigned char sRGB_chunk[5] = {0x73, 0x52, 0x47, 0x42, '\0'};
	unsigned char cICP_chunk[5] = {0x63, 0x49, 0x43, 0x50, '\0'};
	unsigned char mDCv_chunk[5] = {0x6d, 0x44, 0x43, 0x76 ,'\0'};
	unsigned char cLLi_chunk[5] = {0x63, 0x4c, 0x4c, 0x69, '\0'};

	unsigned char* chunks[13] = {IHDR_chunk, PLTE_chunk, IDAT_chunk, IEND_chunk, tRNS_chunk, cHRM_chunk, gAMA_chunk, iCPP_chunk, sBIT_chunk, sRGB_chunk, cICP_chunk, mDCv_chunk, cLLi_chunk};

	size_t check = 0;

	memset(data, '\0', buffer_size);

	// Open the image file
	fp_image = fopen(path, "rb");

	if(fp_image == NULL){
		printf("Cound not open image: %s\n", path);
		return;
	}
	
	fseek(fp_image, 0, SEEK_END);
	printf("Size %ld bytes\n", ftell(fp_image));
	fseek(fp_image, 0, SEEK_SET);

	// Read the file, the first 8 bytes should be the PNG signature
	check = fread(data, 1, 8, fp_image);
	
	if(check != 8)
		printf("fread size does not match!\n");

	if(verbose){
		order.type = PRINT_HEX;
		order.data = data;
		order.size = 8;
		order.fg_color = 32;
		order.bg_color = 0;
	
		printer(&order);
	}

	if(memcmp(data, PNG_signature, 8) != 0){
		printf("\nPNG header not found in first 8 bytes!\n");
		return;
	}

	if(verbose)
		printf(" -> \x1b[32mPNG\x1b[0m signature found!");

	// Clear the data buffer
	memset(data, '\0', buffer_size);

	// ===================================
	// This reads chunks
	// ===================================

	while(running){
		// First we need to read the 4 bytes to get the length
		check = fread(data, 1, 4, fp_image);
	
		if(check != 4)
			printf("fread size does not match!\n");

		chunk_length = hex_to_decimal(data, 4);
		if(chunk_length > buffer_size){
			printf("Overflow\n");
			return;
		}

		if(verbose){
			order.type = PRINT_HEX;
			order.data = data;
			order.size = 4;
			order.fg_color = 31;
			order.bg_color = 0;
			printer(&order);
		}
	
		memset(data, '\0', buffer_size);

		// Next we need to discover the chunk type (4 bytes)
		check = fread(data, 1, 4, fp_image);

		if(check != 4)
			printf("fread size does not match!\n");

		for(unsigned int i = 0; i < chunk_types; i++){
			if(memcmp(data, chunks[i], 4) == 0){
				sprintf(order.auxdata, " -> Found \x1b[36m%s\x1b[0m chunk. (\x1b[31mLength\x1b[0m: %llu)", chunks[i], chunk_length);
				if(i == 3) // IEND
					running = false;
			}
		}
		
		if(verbose){
			order.type = PRINT_HEX;
			order.data = data;
			order.size = 4;
			order.fg_color = 36;
			order.bg_color = 0;
			printer(&order);
		}

		memset(data, '\0', buffer_size);
		
		if(running && (chunk_length == 0)){
			printf("Bad chunk length\n");
			return;
		}

		// The the remaining chunk data
		check = fread(data, 1, chunk_length, fp_image);
		if(check != chunk_length)
			printf("fread %llu, length %llu\n", check, chunk_length);
		
		if(chunk_length < 128){
			if(verbose){
				order.type = PRINT_HEX;
				order.data = data;
				order.size = chunk_length;
				order.fg_color = 0;
				order.bg_color = 0;
				printer(&order);
			}
		}else{
			int x = 8 - order.count;
			
			if(order.count < 8 && order.count > 0){

				order.type = PRINT_HEX;
				order.data = data;
				order.size = x;
				order.fg_color = 0;
				order.bg_color = 0;
				printer(&order);
			}
				
			printf("\n ... [%llu bytes] ... ", chunk_length - x);
			int y = chunk_length % 8;

			if(y != 0){
				order.type = PRINT_HEX;
				order.data = (data+(chunk_length-y));
				order.size = y;
				order.fg_color = 0;
				order.bg_color = 0;
				printer(&order);
			}

			//if(fseek(fp_image, chunk_length, SEEK_CUR) != 0){
			//	printf("Seek error: (%ld)\n", (long int)chunk_length);
			//	perror("readPNG");
			//	return;
			//}
		}

		memset(data, '\0', buffer_size);

		// Read the CRC
		fread(data, 1, 4, fp_image);

		if(verbose){
			order.type = PRINT_HEX;
			order.data = data;
			order.size = 4;
			order.fg_color = 35;
			order.bg_color = 0;
			printer(&order);
		}

		memset(data, '\0', buffer_size);
		chunk_length = 0;

/*		if(!running){
			while(order.count != 1){
				order.type = PRINT_HEX;
				order.data = data;
				order.size = 1;
				order.fg_color = 30;
				order.bg_color = 0;
				printer(&order);
			}
		}*/

	}
}


/*
	// Read the chunk type (IHDR)
	fread(data, 1, 4, fp_image);

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[36m%.2x \x1b[0m", data[i]);

	if(memcmp(data, IHDR_chunk, 4) != 0){
		printf("-> \x1b[36mIHDR\x1b[0m chunk not found!\n");
		return;
	}
	if(verbose)
		printf(" -> \x1b[36mIHDR\x1b[0m chunk found! (\x1b[46m\x1b[31mlength\x1b[0m: %d)\n", chunk_length);

	memset(data, '\0', buffer_size);

	// Read the IHDR info
	
	// Width (4 bytes)
	fread(data, 1, 4, fp_image);
	
	width = hex_to_decimal((void*)data, 4);

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[46m\x1b[34m%.2x \x1b[0m", data[i]);

	memset(data, '\0', buffer_size);

	// Height (4 bytes)
	fread(data, 1, 4, fp_image);

	height = hex_to_decimal((void*)data, 4);

	if(verbose)
		for(int i = 0; i < 4; i++)
			printf("\x1b[46m\x1b[33m%.2x \x1b[0m", data[i]);
	
	if(verbose)
		printf(" -> \x1b[34mWidth\x1b[0m: %dpx, \x1b[33mHeight\x1b[0m: %dpx\n", width, height);

	memset(data, '\0', buffer_size);

	// Bit Depth (1 byte)
	fread(data, 1, 1, fp_image);

	if(verbose)
		printf("\x1b[46m\x1b[32m%.2x \x1b[0m", data[0]);
	
	memset(data, '\0', buffer_size);
	
	// Color Type (1 byte)
	fread(data, 1, 1, fp_image);

	if(verbose)
		printf("\x1b[46m\x1b[31m%.2x \x1b[0m", data[0]);
	
	memset(data, '\0', buffer_size);
	
	// Compression Method (1 byte)
	fread(data, 1, 1, fp_image);

	if(verbose)
		printf("\x1b[46m\x1b[33m%.2x \x1b[0m", data[0]);
	
	memset(data, '\0', buffer_size);
	
	// Filter Method (1 byte)
	fread(data, 1, 1, fp_image);

	if(verbose)
		printf("\x1b[46m\x1b[35m%.2x \x1b[0m", data[0]);
	
	memset(data, '\0', buffer_size);
	
	// Interlace Method (1 byte)
	fread(data, 1, 1, fp_image);

	if(verbose)
		printf("\x1b[46m\x1b[34m%.2x \x1b[0m", data[0]);
	
	memset(data, '\0', buffer_size);

	// Read CRC
	fread(data, 1, 4, fp_image);

	if(verbose)
		for(int i = 0; i < 3; i++)
			printf("\x1b[41m\x1b[37m%.2x \x1b[0m", data[i]);

	if(verbose)
		printf(" -> \x1b[32mBD\x1b[0m: %d, \x1b[31mCol\x1b[0m: %d, \x1b[33mCom\x1b[0m: %d, \x1b[35mFil\x1b[0m: %d, \x1b[34mInt\x1b[0m: %d\n", 0, 0, 0, 0, 0);

	if(verbose)
		printf("\x1b[41m\x1b[37m%.2x \x1b[0m", data[3]);

	CRC = hex_to_decimal((void*)data, 4);

	memset(data, '\0', buffer_size);

	// We dont know what the next chunk will be.
	read_chunk(fp_image, data, buffer_size, verbose);

	return;
}*/


void printer(pOrder* order){
	if(order->type == PRINT_HEX)
		for(int i = 0; i < (int)order->size; i++){
			
			if(order->count >= 8){
				if(order->auxdata[0] != '\0'){
					printf("%s\n", order->auxdata);
					memset(order->auxdata, '\0', 32);
				}else
					printf("\n");
				order->count = 0;
			}
			
			if(order->bg_color == 0)
				printf("\x1b[%dm%.2x \x1b[0m", order->fg_color, order->data[i]);
			else
				printf("\x1b[%dm\x1b[%dm%.2x \x1b[0m", order->fg_color, order->bg_color, order->data[i]);
			
			order->count += 1;
		}
	else
		printf("%s", order->data);
}

unsigned int hex_to_decimal(unsigned char* data, size_t size){
	int sum = 0;

	for(int i = size-1; i >= 0; i--){
		//printf("\n%d + %d * %d\n", sum, (uint32_t)data[i], (int)(pow(16, 2*(size - (i+1)))));
		sum = sum + ((uint32_t)data[i] * (int)(pow(16, 2*(size-(i+1)))));
	}

	return sum;
}

int colorize( void ){
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hOut == INVALID_HANDLE_VALUE)
		return GetLastError();

	DWORD dwMode = 0;
	if(!GetConsoleMode(hOut, &dwMode))
		return GetLastError();

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if(!SetConsoleMode(hOut, dwMode))
		return GetLastError();

	return 0;
}

