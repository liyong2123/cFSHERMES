# include "NSP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* use type defs */
void send_char(char c){
    //this is yet to be defined. Sends single char
}
char recv_char(){
    //this is yet to be defined. Recieves single char
    //from stream
}

/*****************************
 * params: poll, b, a, command
 * returns 1 byte(unsigned char) representing Message
 * control field data to put into NSP packet header
 * corresponding to the poll(1 bit), b flag(1 bit), a flag(1 bit), command number(5 bits)
 * *****************************/
uchar8 MCFMasking(int poll, int b, int a, int command){
    uchar8 retVal = 0b00000000;
    if(poll == 1 || poll == 0){
        retVal = retVal | (poll<<7);
    } else {
        //throw error
    }
    if(b == 1 || b == 0){
        retVal = retVal | (b<<6);
    } else {
        //throw error
    }
    if(a == 1 || a == 0){
        retVal = retVal | (a<<5);
    } else {
        //throw error
    }
    if(command >= 0 && command <= 8){
        retVal = retVal | (command);
    } else{
        printf("Error, Command out of range");
        return -1;
    }
    return retVal;
}

Buffer_Builder_t* bufferBuilder(NSP_Packet* pack){
    char *ptr = (char*) pack;
    int len = sizeof(NSP_Packet);

    //allocate additional 2 spaces for the END tags for SLIP FRAMING
    uchar8* arr = malloc( (len * sizeof(uchar8)) + 2);
    printf("%d\n",END);
    for(int i = 0; i < len + 1; i++){
        if(i < 5){
            printf("%d\n",(uchar8)*ptr);
        }
        if((uchar8)*ptr == ESC){
            arr[i + 1] = (uchar8)ESC_ESC;
        } else if((uchar8)*ptr == END){
            arr[i + 1] = (uchar8)ESC_END;
        } else{
        arr[i + 1] = *ptr;
        }
        ptr++;
    }

    arr[0] = END;
    arr[261] = END;
    Buffer_Builder_t *ret = malloc(sizeof(Buffer_Builder_t));
    ret->arr = arr;
    ret-> len = len + 2;
    return ret;
}

Buffer_Builder_t* bufferBuilder2(NSP_Packet* pack){
    int len = sizeof(NSP_Packet);
    unsigned char *raw = malloc(len + 2);

    //This way, we use MEMCPY to copy over bytes from NSP_Packet to char/byte array
    // and padded on both ends by END Flags. Does not escape certain values if they occur.
    memcpy(&raw[1], pack, len);
    raw[0] = END;
    raw[len+1] = END;

    Buffer_Builder_t *ret = malloc(sizeof(Buffer_Builder_t));
    ret -> arr = raw;
    ret-> len = len + 2;

    return ret;
}

void fillDataField(NSP_Packet* pack, char *arr, int len){
    for(int i = 0; i < len && i < DATA_CAP + 1; i++){
        pack->Data[i] = *arr;
        arr++;
    }
}

NSP_Packet* fill_Packet(uchar8* buff, int len){
    //we can determine the length ourselves too.
    NSP_Packet *retVal = malloc(sizeof(NSP_Packet));
    int currlen = 0;
    while(*buff != END){
        buff++;
    }
    buff++;

    if((uchar8)*buff == ESC_END){
        retVal->Header.Destination_Address = END;
    } else if((uchar8)*buff == ESC_ESC){
        retVal->Header.Destination_Address = ESC;
    } else {
        retVal->Header.Destination_Address = (*buff);
    }
    buff++;

    if((uchar8)*buff == ESC_END){
        retVal->Header.Source_Address = END;
    } else if((uchar8)*buff == ESC_ESC){
        retVal->Header.Source_Address = ESC;
    } else {
        retVal->Header.Source_Address = (*buff);
    }
    buff++;

    if((uchar8)*buff == ESC_END){
        retVal->Header.MCF = (uchar8)END;
    } else if((uchar8)*buff == ESC_ESC){
        retVal->Header.MCF = (uchar8)ESC;
    } else {
        retVal->Header.MCF = (uchar8)(*buff);
    }
    buff++;

    int temp = 0;
    while(currlen < len-5 && *buff != END){
        if(*buff == ESC_END){
            retVal->Data[temp++] = (uchar8)END;
        } else if(*buff == ESC_ESC){
            retVal->Data[temp++] = (uchar8)ESC;
        } else{
            retVal->Data[temp++] = (*buff);
        }
        buff++;
    }

    if(*buff == END){
        printf("ERROR");
    } else{
        short* tempptr = (short*) buff;
        retVal->CRC = (*tempptr);
        buff+=2;
    }
    if(*buff != END){
        printf("ERROR IN PARSING INCOMING PACKET");
    }
    return retVal;
}

int main(){
    //initiates the test packet, and fills in MCF, and the addresses with arbitrary values
    NSP_Packet* testPacket;
    testPacket = malloc( sizeof(NSP_Packet));
    testPacket -> Header.MCF = MCFMasking(1,0,0,0);
    testPacket -> Header.Destination_Address = 100;
    testPacket -> Header.Source_Address = 0300;

    //this is the data field which we want to put into the packet. limit of 255 characters
    char arr[DATA_CAP] = "This is the data, with random stuff mixed in and we can see the results,\
     however we do need something longer so that we can fill this with 255 characters, this is only for testing\
     purposes however and should not be used for the real packet....";
    if(strlen(arr) > DATA_CAP){
        printf("Error, data overflowing");
    }
    //fills in data field with the string, and also the CRC within the NRP packet
    fillDataField(testPacket, arr, strlen(arr));
    testPacket -> CRC = 1011;

    //use buffer Builder 1 or 2 to return a struct with the length and the array of bytes itself. 1 has character escapement
    //keep track of pointers to free later on
    Buffer_Builder_t* ptr = bufferBuilder(testPacket);
    Buffer_Builder_t* frPtr = ptr;

    uchar8* ptr2 = ptr->arr;
    uchar8* frPtr2 = ptr2;

    //loop through to print out all the values within the byte array as hex
    for(int i = 0; i < 5/*ptr->len*/; i++){
        if(*ptr2 == END){
            printf("END");
        } else if(*ptr2 == ESC){
            printf("ESC");
        } else if(*ptr2 == ESC_END){
            printf("ESC_END");
        } else if(*ptr2 == ESC_ESC){
            printf("ESC_ESC");
        } else {
            printf("0x%02x", *ptr2);
        }
        printf("\n");
        ptr2++;
    }
    //length of byte array
    printf("\n\n%d\n", frPtr->len);

    free(testPacket);
    free(frPtr);
    free(frPtr2);
}

/********
 * we want to built a post slip buffer to send to jerry's code
 * *******/
void send_packet(p,len) 
unsigned char *p; 
int len;    {
    send_char(END);

    while(len--){
        switch(*p){
            case END:
                send_char(ESC);
                send_char(ESC_END);
            break;

            case ESC:
                send_char(ESC);
                send_char(ESC_ESC);
            break;

            default:
                send_char(*p);
        }
        p++;
    }
    send_char(END);
}

int recv_packet(p,len)
unsigned char *p;
int len;    {
    unsigned char c;
    int recieved = 0;
    while(1) {
        c = recv_char();
        switch(c){
            case END:
                if(recieved){
                    return recieved;
                } else{
                    break;
                }
            case ESC:
                c = recv_char();
                if(c == ESC_END){
                    c = END;
                    break;
                } else if(c == ESC_ESC){
                    c = ESC;
                    break;
                }
            default:
                if(recieved < len)
                    p[recieved++] = c;
        }
    }
}
