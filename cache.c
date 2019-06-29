#include "common.h"

#define LINE_WIDTH 14
#define SET_NUMBER 64
#define LINE_NUMBER 4

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
	int Line = 0;
	for(;Line<LINE_NUMBER;Line++)
		if(cache[SET][Line].vaild && (TAG == cache[SET][Line].tag))
			return Line;
	return -1;
}
uint32_t Update(struct memory_addr address){
	uint32_t new_line = rand()%LINE_NUMBER;
	if(cache[address.set][new_line].vaild ==1 && cache[address.set][new_line].dirty == 1){
		//TODO: write_back();
		mem_write(address.block_num,cache[address.set][new_line].data);
	}
	mem_read(address.block_num,cache[address.set][new_line].data);
	cache[address.set][new_line].tag = address.tag;
	cache[address.set][new_line].vaild = 1;
	return new_line;
}
struct DATA{
	union{
		struct {
			uint8_t byte0;
			uint8_t byte1;
			uint8_t byte2;
			uint8_t byte3;
		};
		uint32_t val;
	};
}DATA;
void Write(struct memory_addr addr ,uint32_t line , uint32_t data , uint32_t wmask){
	struct DATA Data;
	Data.val = data;
    switch(wmask){
        case 0xff:
			cache[addr.set][line].data[addr.offset]= Data.byte0;
            break;
        case 0xffff:
			cache[addr.set][line].data[addr.offset]= Data.byte0;
			cache[addr.set][line].data[addr.offset+1]= Data.byte1;
            break;
        case 0xffffff:
			cache[addr.set][line].data[addr.offset]= Data.byte0;
			cache[addr.set][line].data[addr.offset+1]= Data.byte1;
			cache[addr.set][line].data[addr.offset+2]= Data.byte2;
            break;
        case 0xffffffff:
			cache[addr.set][line].data[addr.offset]= Data.byte0;
			cache[addr.set][line].data[addr.offset+1]= Data.byte1;
			cache[addr.set][line].data[addr.offset+2]= Data.byte2;
			cache[addr.set][line].data[addr.offset+3]= Data.byte3;
            break;
    } 
	//printf ("Data : %08x \t  cache_data %08x\n",data,cache_read(addr.val));
}

// TODO: implement the following functions

uint32_t cache_read(uintptr_t addr) {
	struct memory_addr address;
	address.val = addr;
	struct DATA Data;
	int line = Hit(address.set,address.tag);
	if (line == -1){
		//MISS
	// printf ("READ MISS\n");
		uint32_t new_line = Update(address);
		Data.byte0 = cache[address.set][new_line].data[address.offset];
		Data.byte1 = cache[address.set][new_line].data[address.offset+1];
		Data.byte2 = cache[address.set][new_line].data[address.offset+2];
		Data.byte3 = cache[address.set][new_line].data[address.offset+3];
	}
	else{
		//HIT
	// printf ("READ HIT\n");
		Data.byte0 = cache[address.set][line].data[address.offset];
		Data.byte1 = cache[address.set][line].data[address.offset+1];
		Data.byte2 = cache[address.set][line].data[address.offset+2];
		Data.byte3 = cache[address.set][line].data[address.offset+3];
	}
	// TODO: return 32bit data;
	return Data.val;
}
void cache_write(uintptr_t addr, uint32_t data, uint32_t wmask) {
	struct memory_addr address;
	address.val = addr;
	uint32_t line = Hit(address.set,address.tag);
	if (line == -1){
		//MISS
		uint32_t new_line = Update(address);
		//write
		Write(address,new_line,data,wmask);
		cache[address.set][new_line].dirty = 1;
	}
	else{
		//HIT
		//write
		Write(address,line,data,wmask);
		cache[address.set][line].dirty = 1;
	}
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
