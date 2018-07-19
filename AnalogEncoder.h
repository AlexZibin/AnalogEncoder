#include "fifo.h"
#include "timer.h"

// Idea and parts of code from https://github.com/mathertel/RotaryEncoder/blob/master/RotaryEncoder.cpp

struct PatternCtlInfo {
    bool passedCurrentStepTest;
    Fifo *pattern; // Pattern format: {Direction (+1/-1), delayMs[, Direction, delayMs, ...]}
    void (*callBackFunction) (long);
};

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
