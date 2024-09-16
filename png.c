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

int main(int argc, char* argv[]){
	unsigned int path = 0; 

	// Fix windows terminal colors
	if(colorize() != 0)
		return -1;

	// Look for a image path in arguments
	for(int i = 0; i < argc; i++)
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
	char auxdata[buffer_size]; 			// Extra data buffer

	bool running = true; 				// the loop control

	pOrder order;
	order.count = 0;

	memset(auxdata, '\0', buffer_size);

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
	
	// Determine the image size
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
		order.auxdata = auxdata;
		order.size = 8;
		order.fg_color = 32;
		order.bg_color = 0;
	
		printer(&order);
	}

	// Check if valid PNG
	if(memcmp(data, PNG_signature, 8) != 0){
		fprintf(stderr, "\nPNG header not found in first 8 bytes!\n"); return; }

	if(verbose)
		printf(" -> \x1b[32mPNG\x1b[0m signature found!");

	// Clear the data buffer
	memset(data, '\0', buffer_size);

	// ========= This reads chunks ==============
	while(running){
		// First we need to read the 4 bytes to get the length
		if(fread(data, 1, 4, fp_image) != 4){
			fprintf(stderr, "fread size does not match!\n"); perror("readPNG"); return; }

		chunk_length = hex_to_decimal(data, 4);
		
		// Check our buffer size
		if(chunk_length > buffer_size){
			fprintf(stderr, "Buffer overflow\n"); return; }

		if(verbose){
			order.type = PRINT_HEX;
			order.data = data;
			order.auxdata = auxdata;
			order.size = 4;
			order.fg_color = 31;
			order.bg_color = 0;
			printer(&order);
		}
	
		memset(data, '\0', buffer_size);

		// Next we need to discover the chunk type (4 bytes)
		if(fread(data, 1, 4, fp_image) != 4){
			fprintf(stderr, "fread size does not match!\n"); perror("readPNG"); return; }

		// Look through the chunk types to see what we have
		for(unsigned int i = 0; i < chunk_types; i++){
			if(memcmp(data, chunks[i], 4) == 0){
				sprintf(auxdata, " -> Found \x1b[36m%s\x1b[0m chunk. (\x1b[31mLength\x1b[0m: %llu)", chunks[i], chunk_length);
				order.auxdata = auxdata;
				if(i == 3) // IEND chunk, we are done
					running = false;
			}
		}
		
		if(verbose){
			order.type = PRINT_HEX;
			order.data = data;
			order.auxdata = auxdata;
			order.size = 4;
			order.fg_color = 36;
			order.bg_color = 0;
			printer(&order);
		}

		memset(data, '\0', buffer_size);
		
		// TODO is this needed?
		if(running && (chunk_length == 0)){
			printf("Bad chunk length\n"); return; }

		// The the remaining chunk data
		if(fread(data, 1, chunk_length, fp_image) != chunk_length){
			printf("fread size did not match length %llu\n", chunk_length); perror("readPNG"); return; }
		
		// If the length is big we dont want to print it
		if(chunk_length < 128){
			if(verbose){
				order.type = PRINT_HEX;
				order.data = data;
				order.auxdata = auxdata;
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
				order.auxdata = auxdata;
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
				order.auxdata = auxdata;
				order.size = y;
				order.fg_color = 0;
				order.bg_color = 0;
				printer(&order);
			}
		}

		memset(data, '\0', buffer_size);

		// Read the CRC
		fread(data, 1, 4, fp_image);

		if(verbose){
			order.type = PRINT_HEX;
			order.data = data;
			order.auxdata = auxdata;
			order.size = 4;
			order.fg_color = 35;
			order.bg_color = 0;
			printer(&order);
		}

		memset(data, '\0', buffer_size);
		chunk_length = 0;
	}
}

void printer(pOrder* order){
	if(order->type == PRINT_HEX)
		for(int i = 0; i < (int)order->size; i++){
			
			if(order->count >= 8){
				if(order->auxdata[0] != '\0'){
					printf("%s\n", order->auxdata);
					order->auxdata[0] = '\0';
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
	unsigned int sum = 0;

	for(int i = size-1; i >= 0; i--)
		sum = sum + ((unsigned int)data[i] * (int)(pow(16, 2*((int)size-(i+1)))));

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

