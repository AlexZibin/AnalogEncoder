#include "fifo.h"
#include "timer.h"

enum class MOVEMENT_STATE {NONE, LEFT, RIGHT};

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 10, uint16_t samplingRate_ms = 10, int _triggerThreshold = 50);
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
        int triggerThreshold;

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
              refValue-pinL ? 0                                 refValue-pinL 
              refValue-pinR ? 0                                 refValue-pinR
              
        2) Controlling with light: (buffer3->average == 0) (negative == true)
        Phase |  pinL    pinR                           |    Phase |  pinL    pinR
          0   |    0       0                            |      0   |    0       0 
          5   |    1       0                            |      5   |    0       1
          6   |    1       1                            |      6   |    1       1
          7   |    0       1                            |      7   |    1       0
          8=0 |    0       0                            |      8=0 |    0       0
              refValue-pinL 
              refValue-pinR 
        */
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t?? pins_arduino_h? pinL, uint8_t pinR, 
                              uint8_t bufferSize, uint16_t samplingRate_ms, int _triggerThreshold) {
    pinMode (pinL, INPUT);
    pinMode (pinR, INPUT);

    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    buffer3 = new Fifo (bufferSize*2);
    bufferL->insert (500);
    bufferR->insert (500);
    
    pattern = nullptr;
    
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
    
    position = 0;
    triggerThreshold = _triggerThreshold;
    movementState = MOVEMENT_STATE::NONE;
    movementPhase = 0;
}

int32_t AnalogEncoder::read () { // Insert this function in loop(). Here runs the main integrating & comparison staff
    static int refValue;
    static int levelToReachBySlave;
    
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
        if (movementState == MOVEMENT_STATE::NONE) {
            buffer3->insert ((bufferL->average () + bufferR->average ()) / 2);
            refValue = buffer3->average ();
        }
        
        bufferL->insert (analogRead (pinL));
        bufferR->insert (analogRead (pinR));
        
        int aL = bufferL->average ();
        int aR = bufferR->average ();

        logln (++count);
        log (F("bufferL->average: ")); logln (aL);
        log (F("bufferR->average: ")); logln (aR);
        log (F("\t buffer3->average: ")); logln (refValue);
        
        if (buffer3->full ()) {
            switch (movementState) {
                case MOVEMENT_STATE::NONE:
                    if (abs (refValue - aL) >= triggerThreshold) { // moves from L to R; aL=master, aR=slave
                        movementState = MOVEMENT_STATE::RIGHT;
                        movementPhase = 1;
                        levelToReachBySlave = aL;
                        ++position;
                    } else if (abs (refValue - aR) >= triggerThreshold) { // moves from R to L
                        movementState = MOVEMENT_STATE::LEFT;
                        movementPhase = 1;
                        levelToReachBySlave = aR;
                        --position;
                    } 
                    break;
                case MOVEMENT_STATE::RIGHT:
                    switch (movementPhase) {
                        case 1:
                            if (abs (aR-levelToReachBySlave) * 5 <= triggerThreshold) {
                                ++movementPhase;
                                ++position;
                                 ////\\\\
                                ////  \\\\
                                
                            }
                            break;
                    }
                    break;
            } // end switch (movementState) 
        } // end if (buffer3->full ())
            
        log (F("position: ")); logln (position);
    }
    return position;
}
