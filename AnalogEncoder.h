#include "fifo.h"
#include "timer.h"

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 10, uint16_t samplingRate_ms = 10);
        int32_t read ();
        inline void write (int32_t p) {position = p;};

    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Timer timer;
        int32_t position;
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize, uint16_t samplingRate_ms) {
    position = 0;
    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
}
