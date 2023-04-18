#include "mbed.h"
#include <cstdint>
#include <cstdio>

Thread main_thread;
Thread device_thread;

//master

SPI spi(D11, D12, D13); // mosi, miso, sclk
DigitalOut cs(D9);

SPISlave device(PD_4, PD_3, PD_1, PD_0); //mosi, miso, sclk, cs; PMOD pins

DigitalOut led(LED3);

int store[256] = {0};

int slave()
{
   device.format(16, 3);
   device.frequency(1000000);
   //device.reply(0x00); // Prime SPI with first reply
   while (1)
   {
      
      if (device.receive())
      {
            int v = device.read(); // Read byte from master
            if ((v | 0X00FF) == 0X01FF)
            {                    
               int address = v & 0X00FF;
               v = device.read(); // Read another byte from master
               store[address] = v;
               //printf("write %d in %d\n", v, address); 
               led = !led;      // led turn blue/orange if device receive
            }
            else if ((v | 0X00FF) == 0X00FF) 
            {
                int address = v & 0X00FF;
                //printf("Read %d\n", store[address]); 
                device.reply(store[address]);
                device.read();
            }
            else
            {
               printf("Default reply to master: 0x00\n");
               device.reply(0x00); //Reply default value
            };
      }
   }
}

void master()
{
   // Setup the spi for 8 bit data, high steady state clock,
   // second edge capture, with a 1MHz clock rate
   spi.format(16, 3);
   spi.frequency(1000000);

   for (int i = 0; i < 10; i++) {
       cs = 1;
       cs = 0;
       int command = 1;
       uint8_t address = i;
       uint16_t data = i * 10;
       printf("Send write %d in address = %d.\n", data, address); 
       spi.write(0x0000 | command << 8 | address);
       ThisThread::sleep_for(100ms);
       spi.write(data);
       ThisThread::sleep_for(100ms);
       cs = 1;

       cs = 0;
       command = 0;
       address = i;
       printf("Send read address = %d\n", address);
       
       spi.write(0X0000 | command << 8 | address); //Send number to slave
       ThisThread::sleep_for(100ms); //Wait for debug print
       int response = spi.write(0X0000); //Read slave reply
       ThisThread::sleep_for(100ms); //Wait for debug print
       printf("Read %d in address = %d.\n", response, address);
       ThisThread::sleep_for(100ms); //Wait for debug print
       cs = 1;
   }
}

int main()
{
   main_thread.start(master);
   device_thread.start(slave);
}