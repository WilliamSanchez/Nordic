#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

#include "string.h"

#define CRC_POLY (0xA001)

#define SLEEP_TIME_MS 5000
#define RECEIVE_BUFF_SIZE 256
#define TRANSMIT_BUFF_SIZE 256
#define RECEIVE_TIMEOUT 100
#define TRANSMIT_TIMEOUT 100

/*      COMANDOS DO MODULO LORA         */

#define RADIO_DATA_PARAMETER    0xD6
#define GET_PARAMETER           0XE2
#define GET_PARAMETER_REMOTE    0xD4
#define CONFIG_RADIO_PARAMETER  0xCA
#define GET_DATA_INFO           0xE7
#define GET_NOISE               0xD8
#define GET_RSSI                0xD5
#define GET_ROUTE               0xD2
#define GET_LEVEL_NOISE_CHANNEL 0xD8
#define SET_ROUTE               0xD3
#define READ                    0x00
#define WRITE                   0x01
/*****************************************/

#define master                  0x0000
#define sensor_1                0x0001
#define sensor_2                0xAABB

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart1));
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static uint8_t tx_buf[TRANSMIT_BUFF_SIZE];
static uint8_t rx_buf[RECEIVE_BUFF_SIZE];
static uint8_t cont_rx = 0;
/*
typedef struct dataSensor
{
        uint16_t ID;
        char time[10]; //hh:mm:ss
        float lat;
        float lon;
        float temp;
        float posc;
};

typedef struct adminDataSensor
{
        uint16_t ID;
        char time[10]; //hh:mm:ss
        uint8_t SNR;
        uint8_t RSSI;
        uint8_t noise;
        uint8_t route;
};
*/
/*
static uint8_t CRC_OK()
{
        uint8_t data_crc[2];
        uint8_t data_cal[2];
        CalculateCRC(rx_buf,9,data_cal);
        printk("CRC CALC= %x|%x",data_cal[0],data_cal[1]);
        return 1;
}
*/
static void CalculateCRC(uint8_t *data, uint32_t length, uint8_t *result_crc)
{
        uint32_t i;
        uint8_t aux, j;
        uint16_t crc_calc;
        
        crc_calc = 0xC181;
        for(i=0; i<length; i++)
        {
                crc_calc ^= ((uint16_t) data[i]) & 0x00FF;
                for(j=0; j< 8; j++)
                {
                        aux = crc_calc;
                        crc_calc >>= 1;
                        if(aux & 1)
                                crc_calc ^= CRC_POLY;
                }

        }
        result_crc[1]= (uint8_t)((crc_calc>>8) & 0x00FF); result_crc[0]=(uint8_t)(crc_calc & 0x00FF);
}
static void uart_cb(const struct device *dev, void *user_data)
{
        static uint8_t u8_data;
        
        if(!uart_irq_update(uart))
                return;

        if(!uart_irq_rx_ready(uart))
                return;
        
        while(uart_fifo_read(uart,&u8_data,1)==1)
                rx_buf[cont_rx]=u8_data; cont_rx++;
        
}


int main(void)
{
        int ret;
        uint8_t checksum[2];

        ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        if(ret < 0){
                return 1;
        }

        ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
        if(ret < 0){
                return 1;
        }

        if(!device_is_ready(uart)){
                printk("UART não esta disponivel");
                gpio_pin_configure_dt(&led, GPIO_ACTIVE_LOW);
                return 1;
        }

        if(!device_is_ready(led.port)){
                printk("LED não esta disponivel");
                return 1;
        }

        ret =  uart_irq_callback_user_data_set(uart,uart_cb,NULL);

        uart_irq_rx_enable(uart);

        printk("Starting... \n");

        uint8_t data[100];
        uint8_t data_cal[2];

        while(1){
                ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}
                k_msleep(SLEEP_TIME_MS);

                data[0] = (uint8_t)((master>>8) & 0x00FF);//0x00;//
                data[1] = (uint8_t)(master & 0x00FF);//0x00;//
                data[2] = RADIO_DATA_PARAMETER;//0xD6;//
                data[3] = READ;//0x00;//
                data[4] = 0x01;
                data[5] = 0x00;
                CalculateCRC(data,6,checksum);
                memcpy(tx_buf,data,6);
                memcpy((tx_buf+6),(char*)checksum,2);
                printk("\n\r%x|%x|%x|%x|%x|%x|%x|%x\n\r",tx_buf[0],tx_buf[1],tx_buf[2],tx_buf[3],tx_buf[4],tx_buf[5],tx_buf[6],tx_buf[7]);
                if (device_is_ready(uart)){
                        ret = uart_fifo_fill(uart, tx_buf, 8);
                        printk("Num dados %d\n\r",ret);
                        if( ret < 0)
                                printk("error sending data \n\r");
                }
                k_msleep(SLEEP_TIME_MS);

                for(int i=0; i < cont_rx; i++)
                        printk("|%x",rx_buf[i]);
        
                CalculateCRC(rx_buf,cont_rx-2,data_cal);
                printk("\n\rNum Caracters %d",cont_rx);
                printk("\n\rCRC DATA = %x|%x ",rx_buf[cont_rx-2],rx_buf[cont_rx-1]);
                printk("\n\rCRC CALC = %x|%x ",data_cal[0],data_cal[1]);;
                        
                cont_rx=0;

                data[0] = (uint8_t)((master>>8) & 0x00FF);//0x00;//
                data[1] = (uint8_t)(master & 0x00FF);//0x00;//
                data[2] = 0xE2;//
                data[3] = READ;//0x00;//
                data[4] = 0x00;
                data[5] = 0x00;
                CalculateCRC(data,6,checksum);
                memcpy(tx_buf,data,6);
                memcpy((tx_buf+6),(char*)checksum,2);
                printk("\n\r%x|%x|%x|%x|%x|%x|%x|%x\n\r",tx_buf[0],tx_buf[1],tx_buf[2],tx_buf[3],tx_buf[4],tx_buf[5],tx_buf[6],tx_buf[7]);
                if (device_is_ready(uart)){
                        ret = uart_fifo_fill(uart, tx_buf, 8);
                        printk("Num dados %d\n\r",ret);
                        if( ret < 0)
                                printk("error sending data \n\r");
                }
                k_msleep(SLEEP_TIME_MS);

                for(int i=0; i < cont_rx; i++)
                        printk("|%x",rx_buf[i]);
        
                CalculateCRC(rx_buf,cont_rx-2,data_cal);
                printk("\n\rNum Caracters %d",cont_rx);
                printk("\n\rCRC DATA = %x|%x ",rx_buf[cont_rx-2],rx_buf[cont_rx-1]);
                printk("\n\rCRC CALC = %x|%x ",data_cal[0],data_cal[1]);
                
                cont_rx=0;

                data[0] = (uint8_t)((master>>8) & 0x00FF);//0x00;//
                data[1] = (uint8_t)(master & 0x00FF);//0x00;//
                data[2] = 0xE7;//
                data[3] = READ;//0x00;//
                data[4] = 0x00;
                data[5] = 0x00;
                CalculateCRC(data,6,checksum);
                memcpy(tx_buf,data,6);
                memcpy((tx_buf+6),(char*)checksum,2);
                printk("\n\r%x|%x|%x|%x|%x|%x|%x|%x\n\r",tx_buf[0],tx_buf[1],tx_buf[2],tx_buf[3],tx_buf[4],tx_buf[5],tx_buf[6],tx_buf[7]);
                if (device_is_ready(uart)){
                        ret = uart_fifo_fill(uart, tx_buf, 8);
                        printk("Num dados %d\n\r",ret);
                        if( ret < 0)
                                printk("error sending data \n\r");
                }
                k_msleep(SLEEP_TIME_MS);

                for(int i=0; i < cont_rx; i++)
                        printk("|%x",rx_buf[i]);
        
                CalculateCRC(rx_buf,cont_rx-2,data_cal);
                printk("\n\rNum Caracters %d",cont_rx);
                printk("\n\rCRC DATA = %x|%x ",rx_buf[cont_rx-2],rx_buf[cont_rx-1]);
                printk("\n\rCRC CALC = %x|%x ",data_cal[0],data_cal[1]);
                
                cont_rx=0;

                k_msleep(SLEEP_TIME_MS);
        }

        return 0;
}