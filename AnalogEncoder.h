#include "fifo.h"
#include "timer.h"

enum class MOVEMENT_STATE {NONE, LEFT, RIGHT};

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 20, uint16_t samplingRate_ms = 10);
        int32_t read (); // Insert this function in loop(). Here runs the main integrating & comparison staff
        inline void write (int32_t p) {position = p;};
        bool registerPattern (Fifo *_pattern, CallBackFunction func); // Pattern format: {Reserved=0, Direction (+1/-1), delayMs[, Direction, delayMs, ...]}

    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Fifo *buffer3;
        Fifo *pattern;
        Timer timer;
        int32_t position;
        MOVEMENT_STATE movementState;
        
        int8_t movementPhase; /*
               movementPhase:
        4 phases of movement from Left to Right:        |    4 phases of movement from Right to Left:
        ("0" represents low analog value, "1" represents high)
        1) Controlling with shadow: (buffer3->average == 1)   
        Phase |  pinL    pinR                           |    Phase |  pinL    pinR
          0   |    1       1                            |      0   |    1       1 
          1   |    0       1                            |      1   |    1       0
          2   |    0       0                            |      2   |    0       0
          3   |    1       0                            |      3   |    0       1
          4=0 |    1       1                            |      4=0 |    1       1
              refValue-pinL > 0                                 refValue-pinL < 0
              refValue-pinR < 0                                 refValue-pinR > 0
              
        2) Controlling with light: (buffer3->average == 0) (negative == true)
        Phase |  pinL    pinR                           |    Phase |  pinL    pinR
          0   |    0       0                            |      0   |    0       0 == refValue
          5   |    1       0                            |      5   |    0       1
          6   |    1       1                            |      6   |    1       1
          7   |    0       1                            |      7   |    1       0
          8=0 |    0       0                            |      8=0 |    0       0
              refValue-pinL < 0
              refValue-pinR > 0
        */
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t?? pins_arduino_h? pinL, uint8_t pinR, uint8_t bufferSize, uint16_t samplingRate_ms) {
    pinMode (pinL, INPUT);
    pinMode (pinR, INPUT);

    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    buffer3 = new Fifo (bufferSize*2);
    bufferL->insert (0);
    bufferR->insert (0);
    
    pattern = nullptr;
    
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
    
    position = 0;
    movementState = MOVEMENT_STATE::NONE;
    movementPhase = 0;
}

int32_t AnalogEncoder::read () { // Insert this function in loop(). Here runs the main integrating & comparison staff
    //const int positionIncrement = 4;
    const int triggerRatio = 2;
    //static bool negative;
    
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
        //refValue = static_cast <int16_t> ((aL + aR) / 2.0);

        buffer3->insert ((bufferL->out () + bufferR->out ()) / 2);
        
        bufferL->insert (analogRead (pinL));
        bufferR->insert (analogRead (pinR));
        
        int aL = bufferL->average ();
        int aR = bufferR->average ();
        int b3avg = buffer3->average ();
        logln (++count);
        log (F("bufferL->average: ")); logln (aL);
        log (F("bufferR->average: ")); logln (aR);
        log (F("buffer3->average: ")); logln (b3avg);
        
        if (buffer3->full ()) {
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                    if (aR/aL >= triggerRatio) { // Left is darker than Right
                        if (b3avg/aL >= triggerRatio) { // Left is going down; Right is stable, i.e. SHADOW moves from L to R
                            movementState = MOVEMENT_STATE::RIGHT;
                            movementPhase = 1;
                            ++position;
                        } else if (aR/b3avg >= triggerRatio) { // Right is going up; Left is stable, i.e. LIGHT moves from R to L
                            movementState = MOVEMENT_STATE::LEFT;
                            movementPhase = 5;
                            --position;
                        }
                    } else if (aL/aR >= triggerRatio) { // // Right is darker than Left
                    } 
                    break;
                case MOVEMENT_STATE::RIGHT:
                    switch (movementPhase) {
                        case 1:
                            как узнать, что оба - одинаковые, но тёмные? buffer3->average ()!
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
