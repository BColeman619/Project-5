////////
// 
//	p4/firmware.c
//
//	Project 4 
//	
//	Hardware multiplexed 7-segment display 
//  on the breadboard using this C program on a 
//  PICOSOC RISC-V processor instantiated on the FPGA.
//
//  The second count is provided as a 16 bit binary number
//  in 4 hex digits.  Note that 4 decimal digits can be
//  sent to the display by sending 4 BCD digits instead of hex.
//
//  reg_gpio = ...      //GPIO output to display circuit
//  var = reg_gpio ...  // input from display circuit GPIO to program
//
// Team FLAC
// USD - COMP 300
// Dec. 2021
//
////////

#include <stdint.h>
#include <stdbool.h>


// a pointer to this is a null pointer, but the compiler does not
// know that because "sram" is a linker symbol from sections.lds.
extern uint32_t sram;

#define reg_spictrl (*(volatile uint32_t*)0x02000000)
#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)
#define reg_gpio (*(volatile uint32_t*)0x03000000)

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss,_heap_start;

uint32_t set_irq_mask(uint32_t mask); asm (
    ".global set_irq_mask\n"
    "set_irq_mask:\n"
    ".word 0x0605650b\n"
    "ret\n"
);

// Here is a function if you don't have "/" and "%"
uint32_t convert(uint32_t num)
{
    unsigned char min_ten = 0;
    unsigned char min_one = 0;
    unsigned char sec_ten = 0;
    unsigned char sec_one = 0;
    while(num >= 600)
    {
        min_ten = min_ten + 1;
        num = num - 600;
    }
    while(num >= 60)
    {
        min_one = min_one + 1;
        num = num - 60;
    }
    while(num >= 10)
    {
        sec_ten = sec_ten + 1;
        num = num - 10;
    }
    sec_one = num;
    return (min_ten << 12) | (min_one << 8) | (sec_ten << 4) | sec_one;
}

void main() {
    set_irq_mask(0xff);

    // zero out .bss section
    for (uint32_t *dest = &_sbss; dest < &_ebss;) {
        *dest++ = 0;
    }

    // switch to dual IO mode
    reg_spictrl = (reg_spictrl & ~0x007F0000) | 0x00400000;

    uint32_t led_timer = 0;
    uint32_t second_timer = 3599;
    uint32_t ms_timer = 0;
    uint32_t display_digit = 0;
    uint32_t comm = 0b1110;
    uint32_t segments = 0b1111111;
    uint32_t hex_to_display = 0x0;
    uint32_t second_toggle = 0;
    uint32_t new_second_toggle = 0;
    uint32_t display = 0;
    uint32_t minutes = 0;
    uint32_t min_tens = 0;
    uint32_t min_ones = 0;
    uint32_t sec_tens = 0;
    uint32_t sec_ones = 0;
    bool dec = 1;
    
    bool incrementing = false;
    while (1) {

      // read values from hardware
      //      assign read_data = ((second_toggle & 32'b1)); // from top.v
      new_second_toggle = reg_gpio & 0x1;


      while(new_second_toggle == second_toggle){
          new_second_toggle = reg_gpio & 0x1;
      }
      second_toggle = new_second_toggle;

      //Our code version 2
      uint32_t minute_save = 3599;
      if(reg_gpio & 0b100){second_timer=3599;} //reset pin TODO: set Buzzer low
     
      else if ((reg_gpio & 0b10000)){
        second_timer -= 60; //decrement min
        minute_save = second_timer - 59;
        
        if(second_timer <= 59){second_timer = 3599;}
        
        } 

      else if ((reg_gpio & 0b1000)){
        second_timer -= 1; // decrement sec
         
        if(second_timer <= minute_save){second_timer = minute_save + 59;}

      }

      else if (reg_gpio & 0b10){ //countdown pin start/stop
        second_timer--;

        if(second_timer <= 0){second_timer = 0;} //TODO set buzzer high
        
        }
        
      /*//Our code
      if(reg_gpio&0b100){ //if the pin is in then reset
          second_timer=3599;
      }
     

      else if ((reg_gpio & 0b10000))
          second_timer -=60; //decrement min
           if(second_timer <= 59){ //if you are patient and count goes to 0 reset to 1 hour
                    
                    second_timer = 3599;
                }
          } 

      else if ((reg_gpio & 0b1000)){
         second_timer -= 1; // decrement sec
         if(second_timer <= 0){ //if you are patient and count goes to 0 reset to 1 hour
                    
                    second_timer = 59;
                }


      else if (reg_gpio&0b10){ //if pin is in then pause else count down
            second_timer--;

                if(second_timer <= 0){ //if you are patient and count goes to 0 reset to 1 hour
                    
                    second_timer = 0;
                }
            }*/
      


    // Code to use a function if "/" and "%" not implemented
    uint32_t con = convert(second_timer);
    reg_gpio = con; // debug LEDs in 4 LSBs
       
  } // end of while(1)
} // end of main program
