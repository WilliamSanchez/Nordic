#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 5000
#define RECEIVE_BUFF_SIZE 32
#define TRANSMIT_BUFF_SIZE 100
#define RECEIVE_TIMEOUT 100
#define TRANSMIT_TIMEOUT 100

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart1));
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static uint8_t tx_buf[TRANSMIT_BUFF_SIZE] = {"LoRa enviando comandos \n\r"};
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
        switch (evt->type)
        {
        case UART_RX_RDY:
                        if((evt->data.rx.len) > 0 ){
                                printk("Chegou dado do PC: %s\r",rx_buf);
                                memset(rx_buf,NULL,RECEIVE_BUFF_SIZE);
                                gpio_pin_toggle_dt(&led);
                        }
                break;
        case UART_RX_DISABLED:
                        uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
                break;
        
        default:
                break;
        }
}


int main(void)
{
        int ret;
        uint8_t cont=0;
        bool led_state = true;

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

        ret = uart_callback_set(uart, uart_cb, NULL);
        if (ret < 0){
                return 1;
        }

        ret = uart_rx_enable(uart, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
        if(ret){
                return 1;
        }

        ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US); //SYS_FOREVER_MS
        if(ret){
                return 1;
        }


        while(1){
                ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}
                led_state = !led_state;
                sprintf(tx_buf,"Thingy91x [%d] LED state: %s\n\r", cont++, led_state ? "ON" : "OFF");
                uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_MS);
                k_msleep(SLEEP_TIME_MS);
        }

        return 0;
}
