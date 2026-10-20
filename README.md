# RotaryEncoder_Library
A C library designed to handle incremental rotary encoders (such as the PEC11R series) using Interrupt-on-Change (IOC) peripherals, quadrature state machine decoding, and a non-blocking debouncing routine for the integrated pushbutton.

## Features
* **Quadrature Decoding (Gray Code):** Processes channels A and B using a software state transition table for maximum accuracy regarding rotation direction (CW Clockwise and CCW Counter-Clockwise).
* **Interrupt-on-Change (IOC):** Efficient use of the PIC's IOC peripheral on both rising and falling edges, avoiding constant polling in the main loop.
* **Integrated Pushbutton Management:** Includes a non-blocking state machine with debouncing capable of detecting short clicks and long presses.
* **Optional Position Limits:** Allows configuring minimum and maximum count boundaries with optional automatic clamping.

## Supported / Tested Devices
This library is designed to work with Interrupt-on-Change (IOC) peripherals. The following chips have been explicitly tested and verified:

| Microcontroller | Pins | Status | Notes |
| :--- | :--- | :--- | :--- |
| **PIC18F25K83** | Pin A -> RB4, Pin B -> RB5 and Pin SW button -> RB3 | ✔ Fully Tested | The high- and low-priority interrupt vector is used. |
| *Generic PIC16 / PIC18* | PORTB pins | ⚠ Compatible |  |

## Hardware Connection
Typical connection for optimal encoder performance, external pull-up resistors and filtering capacitors are recommended on channels A, B, and the switch (SW) line to mitigate noise and bouncing.

| Rotary Encoder Pin | Function | PIC Connection (Example) |
| :--- | :--- | :--- |
| **A** | Channel A (Quadrature Output) | IOC Pin / Digital Input (e.g., RB4 with external pull-up) |
| **B** | Channel B (Quadrature Output) | IOC Pin / Digital Input (e.g., RB5 with external pull-up) |
| **C** | Common Ground | GND |
| **SW (Switch)** | Pushbutton Integrated Switch | Digital Input / IOC Pin (e.g., RB3 with external pull-up) |

## Usage Example
This example shows how to initialize the rotary encoder, get position and get sw button status.

```c
#include <xc.h>
#include "INTERRUPTManager.h"
#include "RotaryEncoder.h"

void main(void){
    // System and oscillator initialization
    SYSTEM_Initialize();
    
    // Initialize encoder with limits enabled from 0 to 100
    ENCODER_Initialize(0, 100, true);
    
    INTERRUPT_GlobalInterruptEnable();

    int16_t lastPos = -1;

    while(1){
        // Read the current position
        int16_t currentPos = ENCODER_GetPosition();
        if(currentPos != lastPos){
            lastPos = currentPos;
            // Update your display or application logic with currentPos and rotary.direction
        }

        // Periodic button update (every ~5 ms)
        ENCODER_ButtonUpdate_5ms();

        // Process integrated button events
        ButtonEvent_t event = ENCODER_GetButtonEvent();
        if(event == BTN_CLICKED){
            ENCODER_SetPosition(0); // Reset position counter to 0 on click
        }

        __delay_ms(5);
    }
}
```

Important: handle IOCs within the high-priority interrupt (it can be configured to use low priority or the IVT).
```c
void __interrupt(high_priority) INTERRUPT_InterruptManagerHigh (void)
{
    // Check if Interrupt-On-Change triggered the interrupt
    if (PIE0bits.IOCIE && PIR0bits.IOCIF){
        ENCODER_ISR_Handler();
    }
}
```
The algorithm on `ENCODER_ISR_Handler()` evaluates the previous state of pins A and B combined with their current state to compute a 4-bit index expression inside the interrupt service routine: 

***index = (rotary.state << 2) | currentState***

This index looks up a jump table (`encoderTable`) that determines if the movement corresponds to a clockwise turn (1), counter-clockwise turn (-1), or invalid/noise state (0).

## Project Structure
* `src/`: Core library files (`RotaryEncoder.c` and `RotaryEncoder.h`).
* `LICENSE`: MIT License.
* `CHANGELOG`: History of updates and bug fixes.

## Documentation & Tutorial
For a detailed implementation explanation and step-by-step guide, you can review the following examples:
* [https://mrchunckuee.blogspot.com/](https://mrchunckuee.blogspot.com/)
