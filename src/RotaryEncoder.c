/*
 * File:   RotaryEncoder.c
 * Author: mrchunckuee_electronics
 * Source File:     RotaryEncoder.c
 * Description:     Implementation of rotary encoder and debounced pushbutton 
 *                  handling routines.
 * 
 * Created on 30 de septiembre de 2026, 12:58 AM
 */

#include "main.h"

volatile Encoder_t rotary;

/*******************************************************************************
 * Function:        static inline uint8_t ENCODER_ReadState(void)
 * Description:     Reads the physical states of Encoder Channels A and B. Combines 
 *                  them into a 2-bit state representation.
 * Precondition:    Encoder channel pins (A and B) configured as digital inputs.
 * Parameters:      None
 * Return Values:   uint8_t - Combined 2-bit state (Bit 1: Channel B, Bit 0: Channel A)
 * Remarks:         Marked as 'static inline' for minimal execution overhead within the ISR.
 ******************************************************************************/
static inline uint8_t ENCODER_ReadState(void){
    uint8_t pinA = ENCODER_A_PIN;
    uint8_t pinB = ENCODER_B_PIN;
    return (uint8_t)((pinB << 1) | pinA);
}

/*******************************************************************************
 * Function:        void ENCODER_Initialize(int16_t minVal, int16_t maxVal, bool useLimits)
 * Description:     Initializes the rotary encoder structure variables and configures 
 *                  the hardware pins for external pull-ups and change interrupts.
 * Precondition:    Global interrupts should be configured in the main system.
 * Parameters:      minVal    - Minimum value for position boundaries.
 *                  maxVal    - Maximum value for position boundaries.
 *                  useLimits - Enable (true) or disable (false) position clamping.
 * Return Values:   None
 * Remarks:         Call this function once before entering the main loop.
 * ****************************************************************************/
void ENCODER_Initialize(int16_t minVal, int16_t maxVal, bool useLimits){
    rotary.position = 0;
    rotary.direction = 0;
    rotary.minPosition = minVal;
    rotary.maxPosition = maxVal;
    rotary.enableLimits = useLimits;

    rotary.buttonState = 0;
    rotary.buttonEvent = BTN_RELEASED;
    rotary.buttonTimerMs = 0;
    
    rotary.state = ENCODER_ReadState();

    ENCODER_HARDWARE_INIT();
}

/*******************************************************************************
 * Function:        void ENCODER_ISR_Handler(void)
 * Description:     Decodes quadrature pulses using a Gray code state transition 
 *                  table and updates encoder position and direction.
 * Precondition:    Must be called from within the IOC Interrupt Service Routine.
 * Parameters:      None
 * Return Values:   None
 * Remarks:         Clears specific IOC flags for encoder signals.
 * ****************************************************************************/
void ENCODER_ISR_Handler(void){
    static const int8_t encoderTable[16] = {
         0,  1, -1,  0,
        -1,  0,  0,  1,
         1,  0,  0, -1,
         0, -1,  1,  0
    };

    uint8_t currentState = ENCODER_ReadState();

    if(currentState != rotary.state){
        uint8_t index = (uint8_t)((rotary.state << 2) | currentState);
        int8_t step = encoderTable[index];

        if(step != 0){
            int16_t nextPos = rotary.position + step;

            if(rotary.enableLimits){
                if(nextPos > rotary.maxPosition) {
                    nextPos = rotary.maxPosition;
                } 
                else if(nextPos < rotary.minPosition){
                    nextPos = rotary.minPosition;
                }
            }

            rotary.position = nextPos;
            rotary.direction = step;
        }

        rotary.state = currentState;
    }

    IOCBFbits.IOCBF4 = 0;
    IOCBFbits.IOCBF5 = 0;
}

/*******************************************************************************
 * Function:        void ENCODER_ButtonUpdate_5ms(void)
 * Description:     Non-blocking button state machine with debouncing and long press 
 *                  detection.
 * Precondition:    Must be called periodically every 5ms (e.g., from a SysTick/Timer ISR
 *                  or main loop interval).
 * Parameters:      None
 * Return Values:   None
 * Remarks:         Active-low logic suitable for external pull-up resistors.
 * ****************************************************************************/
void ENCODER_ButtonUpdate_5ms(void){
    // Active low reading (0 when pressed)
    uint8_t currentPinState = (ENCODER_SW_PIN == 0) ? 1 : 0; 
    static uint8_t debounceCounter = 0;

    // Debounce processing (requires 4 consecutive identical samples = 20ms)
    if(currentPinState != rotary.buttonState){
        debounceCounter++;
        if (debounceCounter >= 4){
            rotary.buttonState = currentPinState;
            debounceCounter = 0;

            if (rotary.buttonState == 1){
                // Button just pressed
                rotary.buttonTimerMs = 0;
            }else{
                // Button released
                if (rotary.buttonTimerMs < 1000){
                    rotary.buttonEvent = BTN_CLICKED;
                }
            }
        }
    }else{
        debounceCounter = 0;
    }

    // Long press detection
    if (rotary.buttonState == 1){
        rotary.buttonTimerMs += 5;
        if (rotary.buttonTimerMs == 1000){ // 1 second threshold
            rotary.buttonEvent = BTN_LONG_PRESSED;
        }
    }
}

/*******************************************************************************
 * Function:        ButtonEvent_t ENCODER_GetButtonEvent(void)
 * Description:     Reads and consumes the current pending button event.
 * Precondition:    ENCODER_Initialize must have been called.
 * Parameters:      None
 * Return Values:   ButtonEvent_t - Pending event (BTN_RELEASED, BTN_CLICKED, BTN_LONG_PRESSED).
 * Remarks:         Clears the pending event after reading.
 * ****************************************************************************/
ButtonEvent_t ENCODER_GetButtonEvent(void){
    ButtonEvent_t event = rotary.buttonEvent;
    rotary.buttonEvent = BTN_RELEASED; // Clear event after reading
    return event;
}

/*******************************************************************************
 * Function:        int16_t ENCODER_GetPosition(void)
 * Description:     Reads the current encoder position in a thread-safe manner.
 * Precondition:    ENCODER_Initialize must have been called.
 * Parameters:      None
 * Return Values:   int16_t - Current position counter value.
 * Remarks:         Briefly disables the interrupt to ensure atomic read.
 * ****************************************************************************/
int16_t ENCODER_GetPosition(void){
    int16_t pos;
    PIE0bits.IOCIE = 0;
    pos = rotary.position;
    PIE0bits.IOCIE = 1;
    return pos;
}

/*******************************************************************************
 * Function:        void ENCODER_SetPosition(int16_t newPos)
 * Description:     Sets the encoder position counter to a specific value.
 * Precondition:    ENCODER_Initialize must have been called.
 * Parameters:      newPos - New value to write into the position counter.
 * Return Values:   None
 * Remarks:         Briefly disables the interrupt to ensure atomic write.
 * ****************************************************************************/
void ENCODER_SetPosition(int16_t newPos){
    PIE0bits.IOCIE = 0;
    rotary.position = newPos;
    PIE0bits.IOCIE = 1;
}