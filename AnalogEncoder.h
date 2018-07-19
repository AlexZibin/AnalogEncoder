#include "fifo.h"
#include "timer.h"

// Idea and parts of code from https://github.com/mathertel/RotaryEncoder/blob/master/RotaryEncoder.cpp

struct PatternCtlInfo {
    bool passedCurrentStepTest;
    Fifo *pattern; // Pattern format: {Direction (+1/-1), delayMs[, Direction, delayMs, ...]}
    void (*callBackFunction) (long);
};

PatternCtlInfo patternCtlInfo [] = {{0, nullptr, nullptr}, {0, nullptr, nullptr}, {0, nullptr, nullptr}};

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 10, uint16_t samplingRate_ms = 10, int _triggerThreshold = 50);
        int32_t read (void); // Insert this function in loop(). Here runs the main integrating & comparison staff
        inline void write (int32_t p) {position = p;};
        //bool registerPattern (Fifo *_pattern, CallBackFunction func); // Pattern format: {Reserved=0, Direction (+1/-1), delayMs[, Direction, delayMs, ...]}

    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Fifo *buffer3;
        Fifo *pattern;
        Timer timer;
        int32_t position;
        int triggerThreshold;

        volatile int8_t _oldState;
        int8_t state;
    
        int refValue, aL, aR;
        float coeffL, coeffR;
        bool triggerByShadow;
        
        int8_t getState (void);
        int8_t normalize (int8_t val);
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
    _oldState = 0;
    
    //movementState = MOVEMENT_STATE::NONE;
    //movementPhase = 0;
}

const int8_t knobdir[] = {
  0, -1,  1,  0,
  1,  0,  0, -1,
 -1,  0,  0,  1,
  0,  1, -1,  0
};

int32_t AnalogEncoder::read (void) { // Insert this function in loop(). Here runs the main integrating & comparison staff
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
        if (_oldState == 0) {
            buffer3->insert ((bufferL->average () + bufferR->average ()) / 2);
        }
        
        bufferL->insert (analogRead (pinL));
        bufferR->insert (analogRead (pinR));
        
        aL = bufferL->average ();
        aR = bufferR->average ();

        logln (++count);
        log (F("bufferL->average: ")); logln (aL);
        log (F("bufferR->average: ")); logln (aR);
        log (F("\t buffer3->average: ")); logln (refValue);
        
        if (buffer3->full ()) {
            if (_oldState == 0) {
                refValue = buffer3->average ();
                coeffL = refValue / aL;
                coeffR = refValue / aR;
            } // end if (_oldState == 0)
            
            state = getState ();
            if (_oldState != state) {
                int32_t incPos = knobdir[state | (_oldState << 2)];
                if (incPos) {
                    _oldState = state;
                    position += incPos;
                }

                log (F("\nstate: ")); logln (state);
                log (F("_oldState: ")); logln (_oldState);
                log (F("position: ")); logln (position);
            }
        } // end if (buffer3->full ())
    }
    return position;
}
 
int8_t AnalogEncoder::getState (void) {
    /*
    int aaL = aL*coeffL;
    int aaR = aR*coeffR;
    log (F("*= coeffL bufferL->average aaL: ")); logln (aaL);
    log (F("*= coeffR bufferR->average aaR: ")); logln (aaR);
    */
    
    return (normalize (aL) << 1) | normalize (aR);
}

int8_t AnalogEncoder::normalize (int8_t val) {
    if (abs (val - refValue) <= triggerThreshold) 
        return 0;

    if (_oldState == 0) { // need to set triggerByShadow
        if (val - refValue >= triggerThreshold) {
            triggerByShadow = false;
        } else if (refValue - val >= triggerThreshold) {
            triggerByShadow = true;
        }
        return 1;
    } else if (triggerByShadow == false) {
        if (val - refValue >= triggerThreshold) return 1;
    } else {
        if (refValue - val >= triggerThreshold) return 1;
    }
    return 0;
}

/////////////////
    Example program:    

    AnalogEncoder analogEncoder (A1, A2);

    int32_t analogEncoderPosition = analogEncoder.read ();
    static int counter = 0;
    if (analogEncoderPosition) {
        logln (++counter);
        log ("analogEncoderPosition: "); logln (analogEncoderPosition); 
        //analogEncoderPosition = 0;
    }

