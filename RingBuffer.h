#pragma once
#include <Arduino.h>
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define BUFFER_SIZE 64 /

struct RingBuffer {
    volatile uint32_t data[BUFFER_SIZE];
    volatile uint8_t head = 0;
    volatile uint8_t tail = 0;

    //new data avl
    bool available() { return head != tail; }

    // ISR calls to push raw IR values
    void push(uint32_t val) {
        uint8_t next = (head + 1) & (BUFFER_SIZE - 1);
        if (next != tail) { // Prevent overflow
            data[head] = val;
            head = next;
        }
    }


    uint32_t pop() {
        uint32_t val = data[tail];
        tail = (tail + 1) & (BUFFER_SIZE - 1);
        return val;
    }
};
#endif