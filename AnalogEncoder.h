#include "fifo.h"
#include "timer.h"

enum class MOVEMENT_STATE {NONE, LEFT, RIGHT};

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 40, uint16_t samplingRate_ms = 10);
        int32_t read (); // Insert this function in loop(). Here runs the main integrating & comparison staff
        inline void write (int32_t p) {position = p;};
        bool registerPattern (Fifo *pattern, CallBackFunction func); // Pattern format: Direction (+1/-1) / delayMs / Direction / delayMs / ...

    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Timer timer;
        int32_t position;
        MOVEMENT_STATE movementState;
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize, uint16_t samplingRate_ms) {
    pinMode (pinL, INPUT);
    pinMode (pinR, INPUT);

    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
    
    position = 0;
    movementState = MOVEMENT_STATE::NONE;
}

int32_t AnalogEncoder::read () { // Here runs the main integrating & comparison staff
    const int positionIncrement = 4;
    const float triggerRatio = 2.0;
    #define DEBUG
    #ifdef DEBUG
        static long count = 0;
        #define log(msg) Serial.print(msg)
        #define logln(msg) Serial.println(msg)
    #else
        #define log(msg)
        #define logln(msg)
    #endif
    
    if (timer.needToTrigger ()) {
        bufferL->insert (analogRead (pinL));
        bufferR->insert (analogRead (pinR));
        
        float aL = bufferL->average ();
        float aR = bufferR->average ();
        logln (++count);
        log (F("bufferL->average: ")); logln (aL);
        log (F("bufferR->average: ")); logln (aR);
        
        if (aR/aL >= triggerRatio) { // Left = dark, Right = bright; Movement L->R
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                case MOVEMENT_STATE::LEFT:
                    movementState = MOVEMENT_STATE::RIGHT;
                    position += positionIncrement;
                    break;
            }
        } else if (aL/aR >= triggerRatio) { // Movement R->L
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                case MOVEMENT_STATE::RIGHT:
                    movementState = MOVEMENT_STATE::LEFT;
                    position -= positionIncrement;
                    break;
            }
        } else { // Left = Right; NO Movement
            
        }
        log (F("position: ")); logln (position);
    }
    return position;
}
