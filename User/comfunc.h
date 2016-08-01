#ifndef _COMFUNC_H_
#define _COMFUNC_H_
//---------------------------------------------------------------------------------------
#include <stdint.h>
//---------------------------------------------------------------------------------------
typedef uint32_t time_t;
//---------------------------------------------------------------------------------------
#define usleep(us) delayus(us)
//---------------------------------------------------------------------------------------
#define SIZE_1K 1024L
#define SIZE_1M (SIZE_1K*SIZE_1K)
//---------------------------------------------------------------------------------------
#define min(a, b) ((a)<(b) ? (a):(b))
#define max(a, b) ((a)>(b) ? (a):(b))
#define bcd2bin(val) (((val) & 0x0f) + ((val) >> 4) * 10)
#define bin2bcd(val) ((((val) / 10) << 4) + (val) % 10)
//---------------------------------------------------------------------------------------
#define set_bit(x, bit) ((x) |= 1 << (bit))
#define clr_bit(x, bit) ((x) &= ~(1 << (bit)))
#define tst_bit(x, bit) ((x) & (1 << (bit)))
#define get_bits(val,x1,x2)   (((val)>>(x1))&((1<<((x2)-(x1)+1))-1))
#define hole(val, start, end) ((val) & (~(((1 << ((end) - (start) + 1)) - 1) << (start))))
#define fill(set, start, end) (((set) & ((1 << ((end) - (start) + 1)) - 1))<< (start))
#define set_bits(val, set, start, end) (val = hole(val, start, end) | fill(set, start, end))
//---------------------------------------------------------------------------------------
#define time_after(a,b)     ((long)(b) - (long)(a) < 0)
#define time_before(a,b)    time_after(b,a)
#define time_after_eq(a,b)  ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a,b)	time_after_eq(b,a)
//---------------------------------------------------------------------------------------
#define array_size(array) (sizeof(array)/sizeof(*array))
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type,member)))
//---------------------------------------------------------------------------------------
#define swap(a, b) do {a ^= b; b ^= a; a ^= b;} while (0)
#define _swab16(x) ((uint16_t)(				\
	(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |			\
	(((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#define _swab32(x) ((uint32_t)(				\
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |		\
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |		\
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |		\
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

#ifdef BIG_ENDIAN
#define cpu_to_le32(x) ((uint32_t)_swab32((x)))
#define le32_to_cpu(x) ((uint32_t)_swab32((uint32_t)(x)))
#define cpu_to_le16(x) ((uint16_t)_swab16((x)))
#define le16_to_cpu(x) ((uint16_t)_swab16((uint16_t)(x)))
#define cpu_to_be32(x) ((uint32_t)(x))
#define be32_to_cpu(x) ((uint32_t)(x))
#define cpu_to_be16(x) ((uint16_t)(x))
#define be16_to_cpu(x) ((uint16_t)(x))
#else
#define cpu_to_le32(x) ((uint32_t)(x))
#define le32_to_cpu(x) ((uint32_t)(x))
#define cpu_to_le16(x) ((uint16_t)(x))
#define le16_to_cpu(x) ((uint16_t)(x))
#define cpu_to_be32(x) ((uint32_t)_swab32((x)))
#define be32_to_cpu(x) ((uint32_t)_swab32((uint32_t)(x)))
#define cpu_to_be16(x) ((uint16_t)_swab16((x)))
#define be16_to_cpu(x) ((uint16_t)_swab16((uint16_t)(x)))
#endif

//---------------------------------------------------------------------------------------
#define BIT_REVERSE_U8(v8) \
do {\
    (v8) = (((v8) >> 1) & 0x55) | (((v8) << 1) & 0xaa);\
    (v8) = (((v8) >> 2) & 0x33) | (((v8) << 2) & 0xcc);\
    (v8) = (((v8) >> 4) & 0x0f) | (((v8) << 4) & 0xf0);\
} while (0)
#define BIT_REVERSE_U16(v16) \
do {\
    (v16) = (((v16) >> 1) & 0x5555) | (((v16) << 1) & 0xaaaa);\
    (v16) = (((v16) >> 2) & 0x3333) | (((v16) << 2) & 0xcccc);\
    (v16) = (((v16) >> 4) & 0x0f0f) | (((v16) << 4) & 0xf0f0);\
    (v16) = (((v16) >> 8) & 0x00ff) | (((v16) << 8) & 0xff00);\
} while (0)
#define BIT_REVERSE_U32(v32) \
do {\
    (v32) = (((v32) >> 1) & 0x55555555ul) | (((v32) << 1) & 0xaaaaaaaaul);\
    (v32) = (((v32) >> 2) & 0x33333333ul) | (((v32) << 2) & 0xccccccccul);\
    (v32) = (((v32) >> 4) & 0x0f0f0f0ful) | (((v32) << 4) & 0xf0f0f0f0ul);\
    (v32) = (((v32) >> 8) & 0x00ff00fful) | (((v32) << 8) & 0xff00ff00ul);\
    (v32) = (((v32) >> 16)& 0x0000fffful) | (((v32) << 16)& 0xffff0000ul);\
} while (0)
#define BIT_REVERSE_U64(v64) \
do {\
    (v64) = (((v64) >> 1) & 0x5555555555555555ull) | (((v64) << 1) & 0xaaaaaaaaaaaaaaaaull);\
    (v64) = (((v64) >> 2) & 0x3333333333333333ull) | (((v64) << 2) & 0xccccccccccccccccull);\
    (v64) = (((v64) >> 4) & 0x0f0f0f0f0f0f0f0full) | (((v64) << 4) & 0xf0f0f0f0f0f0f0f0ull);\
    (v64) = (((v64) >> 8) & 0x00ff00ff00ff00ffull) | (((v64) << 8) & 0xff00ff00ff00ff00ull);\
    (v64) = (((v64) >> 16)& 0x0000ffff0000ffffull) | (((v64) << 16)& 0xffff0000ffff0000ull);\
    (v64) = (((v64) >> 1) & 0x00000000ffffffffull) | (((v64) << 1) & 0xffffffff00000000ull);\
} while (0)

//---------------------------------------------------------------------------------------
void delayus(unsigned long us);
void delayms(unsigned long ms);

void mdelay(int ms);
//---------------------------------------------------------------------------------------
uint8_t checksum(const void *data, int len);
//---------------------------------------------------------------------------------------
int is_all_xx(const void *s1, uint8_t val, int n);
int is_all_bcd(const void *arr, int n);
//---------------------------------------------------------------------------------------
void memaddnum(uint8_t mem[], int num, int cnt);
//---------------------------------------------------------------------------------------
int get_last_bit_seqno(uint32_t x);
//---------------------------------------------------------------------------------------
void reverse(void *buff, int len);
//---------------------------------------------------------------------------------------
uint32_t get_le_val(const uint8_t * p, int bytes);
uint32_t get_be_val(const uint8_t * p, int bytes);
void put_le_val(uint32_t val, uint8_t * p, int bytes);
void put_be_val(uint32_t val, uint8_t * p, int bytes);
//---------------------------------------------------------------------------------------
void float2bin(float f, int ibits, int fbits, uint8_t bin[]);
void float2bcd(float f, int ibits, int fbits, uint8_t bcd[]);
float bin2float(uint8_t bin[], int ibits, int fbits);
float bcd2float(uint8_t bcd[], int ibits, int fbits);

uint32_t bcd2int(uint8_t bcd[], int n);
void int2bcd(uint32_t val, uint8_t bcd[], int n);
//---------------------------------------------------------------------------------------
uint16_t get_crc16(const void *data, uint32_t size);
//---------------------------------------------------------------------------------------
#endif
