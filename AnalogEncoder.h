#include "fifo.h"
#include "timer.h"

enum class MOVEMENT_STATE {NONE, LEFT, RIGHT};

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 40, uint16_t samplingRate_ms = 10);
        int32_t read (); // Insert this function in loop(). Here runs the main integrating & comparison staff
        inline void write (int32_t p) {position = p;};
        bool registerPattern (Fifo *_pattern, CallBackFunction func); // Pattern format: Direction (+1/-1) / delayMs / Direction / delayMs / ...

    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Fifo *pattern;
        Timer timer;
        int32_t position;
        MOVEMENT_STATE movementState;
        
        int8_t movementPhase; /*
               movementPhase:
        Phases of movement from Left to Right:              Phases of movement from Right to Left:
        1) Controlling with shadow: (refValue == 1)
        Phase |  pinL    pinR                               Phase |  pinL    pinR
          0   |    1       1 == refValue                      0   |    1       1 == refValue
          1   |    0       1                                  1   |    1       0
          2   |    0       0                                  2   |    0       0
          3   |    1       0                                  3   |    0       1
          4=0 |    1       1                                  4=0 |    1       1
        2) Controlling with light: (refValue == 0)
        Phase |  pinL    pinR                               Phase |  pinL    pinR
          0   |    0       0 == refValue                      0   |    0       0 == refValue
          1   |    1       0                                  1   |    0       1
          2   |    1       1                                  2   |    1       1
          3   |    0       1                                  3   |    1       0
          4=0 |    0       0                                  4=0 |    0       0
        */
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t?? pins_arduino_h? pinL, uint8_t pinR, uint8_t bufferSize, uint16_t samplingRate_ms) {
    pinMode (pinL, INPUT);
    pinMode (pinR, INPUT);

    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    pattern = nullptr;
    
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
    
    position = 0;
    movementState = MOVEMENT_STATE::NONE;
    movementPhase = 0;
}

int32_t AnalogEncoder::read () { // Here runs the main integrating & comparison staff
    //const int positionIncrement = 4;
    const float triggerRatio = 2.0;
    static uint16_t refValue;
    
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
        
        if (bufferL->full ()) {
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                    if (aR/aL >= triggerRatio) { // Left = dark, Right = bright; Movement L->R
                        movementState = MOVEMENT_STATE::RIGHT;
                        movementPhase = 1;
                        ++position;
                    } else if (aL/aR >= triggerRatio) { // Movement R->L
                        movementState = MOVEMENT_STATE::LEFT;
                        movementPhase = 1;
                        --position;
                    } else {
                        refValue = static_cast <int> ((aL + aR) / 2.0);
                    break;
                case MOVEMENT_STATE::RIGHT:
                    switch (movementPhase) {
                        case 1:
                            как узнать, что оба - одинаковые, но тёмные?
                            break;
                    }
                    break;
            } // end switch (movementState) 
        } // end if (bufferL->full ())
            
 /*       
        if (aR/aL >= triggerRatio) { // Left = dark, Right = bright; Movement L->R
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                case MOVEMENT_STATE::LEFT:
                    movementState = MOVEMENT_STATE::RIGHT;
                    position += positionIncrement;
                    logln (F("RIGHT start"));
                    break;
            }
        } else if (aL/aR >= triggerRatio) { // Movement R->L
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                case MOVEMENT_STATE::RIGHT:
                    movementState = MOVEMENT_STATE::LEFT;
                    position -= positionIncrement;
                    logln (F("LEFT start"));
                    break;
            }
        } else { // Left = Right; NO Movement
            switch (movementState) {
                case MOVEMENT_STATE::LEFT:
                case MOVEMENT_STATE::RIGHT:
                    logln (F("NONE start"));
                    break;
            }
            movementState = MOVEMENT_STATE::NONE;
        }
*/
        
        log (F("position: ")); logln (position);
    }
    return position;
}
