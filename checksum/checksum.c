#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CRC_POLY (0xA001)



void main(int argc, char **argv)
{
 
   char *data = argv[1];
   uint8_t bitbang;
   uint16_t crc_calc=0xc181;

   if(argc < 2){
   	printf("Input a file to send\n");
	exit(-1);
   }
   printf("data= %s\n",argv[1]); 
   
   
   for(int i=0; i< strlen(argv[1]); i++)
   {
   	printf("char %c\n\r",data[i]);
        crc_calc ^= ((uint16_t)data[i]) & 0x00FF;
        
        for(int j=0; j < 8; j++)
        {
            printf("%d = %c, %d\n\r", j, data[i], bitbang);  
            bitbang = crc_calc;
            crc_calc >>= 1;
            if(bitbang & 1)
            {
            	crc_calc ^= CRC_POLY;
            }
        }
   }
   printf("CRC %x\n\r",crc_calc);
}



