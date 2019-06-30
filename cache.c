#include "common.h"

#define LINE_WIDTH 14
#define SET_NUMBER 64
#define LINE_NUMBER 4


void try_increase(int n);

void hit_increase(int n);
void mem_read(uintptr_t block_num, uint8_t *buf);
void mem_write(uintptr_t block_num, const uint8_t *buf);

uint32_t cache_read(uintptr_t addr);

struct cache_line{
	uint8_t vaild ;	
	uint8_t dirty ;	
	uint8_t tag ;
	uint8_t data[SET_NUMBER];
} cache_line;
struct memory_addr{
	union{
		struct{
			uint16_t offset : 6 ;
			uint16_t set : 6 ;
			uint16_t tag : 3 ;
		};
		struct{
			uint16_t block_offset : 6 ;
			uint16_t block_num : 9 ;
		};
		uint16_t val;
	};
} memory_addr;

int Hit(uint32_t set , uint32_t tag);
uint32_t Update(struct memory_addr address);

static struct cache_line cache[SET_NUMBER][LINE_NUMBER];

int Hit(uint32_t SET , uint32_t TAG){
	try_increase(1);
	int Line = 0;
	for(;Line<LINE_NUMBER;Line++)
		if(cache[SET][Line].vaild == 1 && (TAG == cache[SET][Line].tag))
			return Line;
	return -1;
}
uint32_t Update(struct memory_addr address){
	uint32_t new_line = rand()%LINE_NUMBER;
	if(cache[address.set][new_line].vaild ==1 && cache[address.set][new_line].dirty == 1){
		//TODO: write_back()!!!;
		uint32_t origin_block = address.set + (SET_NUMBER * cache[address.set][new_line].tag);
		mem_write(origin_block,cache[address.set][new_line].data);
	}
	mem_read(address.block_num,cache[address.set][new_line].data);
	cache[address.set][new_line].tag = address.tag;
	cache[address.set][new_line].vaild = 1;
	cache[address.set][new_line].dirty = 0;
	return new_line;
}
void Write(struct memory_addr addr ,uint32_t line , uint32_t data , uint32_t wmask){
	uint32_t *p = (void *)cache[addr.set][line].data + addr.offset;
	*p = (*p & ~wmask) | (data & wmask);
}

// TODO: implement the following functions

uint32_t cache_read(uintptr_t addr) {
	struct memory_addr address;
	address.val = addr&(~0x3);
	int line = Hit(address.set,address.tag);
	if (line == -1){
		//MISS
		//printf ("READ MISS\n");
		line = Update(address);
	}
	else hit_increase(1);
	//HIT
	//printf ("READ HIT\n");
	uint32_t *p = (void *)cache[address.set][line].data + address.offset;
	// TODO: return 32bit data;
	return *p;
}

void cache_write(uintptr_t addr, uint32_t data, uint32_t wmask) {
	struct memory_addr address;
	address.val = addr & ~0x3;
	uint32_t line = Hit(address.set,address.tag);
	if (line == -1){
		//MISS
		line = Update(address);
	}
	else hit_increase(1);
	//HIT
	//write
	Write(address,line,data,wmask);
	cache[address.set][line].dirty = 1;
}

void init_cache(int total_size_width, int associativity_width) {
	for (int i = 0; i < SET_NUMBER; i++) {
		for (int j = 0; j < LINE_NUMBER; j++) {
			cache[i][j].vaild = 0;
			cache[i][j].tag = 0;
			cache[i][j].dirty = 0;
		}
	}
}
