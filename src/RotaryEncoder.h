/*
 * File:   RotaryEncoder.c
 * Author: mrchunckuee_electronics
 * Source File:     RotaryEncoder.c
 * Description:     Implementation of rotary encoder and debounced pushbutton 
 *                  handling routines.
 * 
 * Created on 30 de septiembre de 2026, 12:58 AM
 */

#ifndef ROTARYENCODER_H
#define	ROTARYENCODER_H

#ifdef	__cplusplus
extern "C" {
#endif

 
    #include <xc.h>
    #include <stdint.h>
    #include <stdbool.h>

/*********** P O R T   D E F I N E S ******************************************/
    #define ENCODER_A_PIN       PORTBbits.RB5
    #define ENCODER_B_PIN       PORTBbits.RB4
    #define ENCODER_SW_PIN      PORTBbits.RB3

    // This for PIC18F25K83
    #define ENCODER_HARDWARE_INIT() do { \
        ANSELBbits.ANSELB4 = 0; /* RB4 digital input */ \
        ANSELBbits.ANSELB5 = 0; /* RB5 digital input */ \
        ANSELBbits.ANSELB3 = 0; /* RB3 digital input */ \
        TRISBbits.TRISB4 = 1;   /* Input */ \
        TRISBbits.TRISB5 = 1;   /* Input */ \
        TRISBbits.TRISB3 = 1;   /* Input */ \
        WPUBbits.WPUB4 = 0;     /* Disable internal pull-up (External used) */ \
        WPUBbits.WPUB5 = 0;     /* Disable internal pull-up (External used) */ \
        WPUBbits.WPUB3 = 0;     /* Disable internal pull-up (External used) */ \
        IOCBNbits.IOCBN4 = 1;   /* Interrupt-on-change RB4 falling */ \
        IOCBPbits.IOCBP4 = 1;   /* Interrupt-on-change RB4 rising */ \
        IOCBNbits.IOCBN5 = 1;   /* Interrupt-on-change RB5 falling */ \
        IOCBPbits.IOCBP5 = 1;   /* Interrupt-on-change RB5 rising */ \
        IOCBFbits.IOCBF4 = 0;   /* Clear flag */ \
        IOCBFbits.IOCBF5 = 0;   /* Clear flag */ \
        PIE0bits.IOCIE = 1;     /* Enable IOC interrupts */ \
    } while(0)

/*********** D A T A   T Y P E S   D E F I N I T I O N ************************/
    typedef enum{
        BTN_RELEASED = 0,
        BTN_PRESSED,
        BTN_CLICKED,
        BTN_LONG_PRESSED
    } ButtonEvent_t;

    typedef struct {
        // Rotary Encoder
        volatile int16_t position;      // Current encoder position
        volatile int8_t  direction;     // Last direction: -1 (CCW), 0 (None), 1 (CW)
        volatile uint8_t state;         // Current AB pin state (2 bits)

        int16_t minPosition;            // Minimum position boundary
        int16_t maxPosition;            // Maximum position boundary
        bool enableLimits;              // Boundary clamping switch

        // Pushbutton Controls
        volatile uint8_t  buttonState;      // Current debounced state (0: Released, 1: Pressed)
        volatile ButtonEvent_t buttonEvent; // Pending event flag
        uint16_t buttonTimerMs;             // Milliseconds counter for long press
    } Encoder_t;

    extern volatile Encoder_t rotary;

/*********** P R O T O T Y P E S **********************************************/
    void ENCODER_Initialize(int16_t minVal, int16_t maxVal, bool useLimits);
    void ENCODER_ISR_Handler(void);
    void ENCODER_ButtonUpdate_5ms(void);
    ButtonEvent_t ENCODER_GetButtonEvent(void);
    int16_t ENCODER_GetPosition(void);
    void ENCODER_SetPosition(int16_t newPos);


    
    
    
    
    
    


#ifdef	__cplusplus
}
#endif

#endif	/* ROTARYENCODER_H */

