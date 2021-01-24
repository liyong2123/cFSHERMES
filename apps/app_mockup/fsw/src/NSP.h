#ifndef NSP
#define NSP
#include <assert.h>

//SLIP PROTOCOL
#define END         0300 // decimal 192
#define ESC         0333 // decimal 219
#define ESC_END     0334
#define ESC_ESC     0335

#define DATA_CAP    254

typedef unsigned char uchar8;

static_assert(sizeof(uchar8) == 1, "Data structure doesn't match page size");


/***************************
 * Designination Address: 1 Byte
 * Source Address: 1 Byte
 * Message Control Field: 1 Byte -> 
 *        7: Poll Bit-> always 1
 *        6: "B" bit
 *        5: "A"(ACK) Bit
 *        4-0(5 total): command code 
 * ******************************/
struct NSP_Header{
    //Larry was right, syntactically, : should work, but in practice
    //it made it so compiler was allocating more than what it should be
    uchar8 Destination_Address;
    uchar8 Source_Address;
    //use bit masking/MCF function
    uchar8 MCF;
};


/************************
* Data 0-256 Bytes
* Message CRC(Check Sum): 2 Bytes
* Total Bytes should be 260 or less per documentation
***************************/
struct NSP_Packet{
    //Total of 3 Bytes from header
    struct NSP_Header Header;
    //255 Bytes
    uchar8 Data[DATA_CAP];
    //2 Bytes
    unsigned short CRC;
};

struct Buffer_Builder_t{
    uchar8* arr;
    short len;
};

typedef struct Buffer_Builder_t Buffer_Builder_t;
typedef struct NSP_Packet NSP_Packet;

/* array, where begins and ends with "END" */

Buffer_Builder_t* bufferBuilder(NSP_Packet* pack);
Buffer_Builder_t* bufferBuilder2(NSP_Packet* pack);
void fillDataField(NSP_Packet* pack, char *arr, int len);
uchar8 MCFMasking(int poll, int b, int a, int commmand);
void send_packet(unsigned char *p, int len);
int recv_packet(unsigned char *p, int len);
#endif