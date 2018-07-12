#include "fifo.h"
#include "timer.h"

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, int bufferSize = 10);
    private:
        Fifo *buffer;
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t pinL, uint8_t pinR, int bufferSize = 10) {
    buffer = new Fifo (bufferSize);
}
