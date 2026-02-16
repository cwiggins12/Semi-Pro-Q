
#pragma once

//==============================================================================
/** ANALYSIS TOOLS
*/
//abstract FIFO for the analyser
template <typename T>
struct Fifo {
    Fifo(int capacity) : fifo(capacity) {
        buffer.allocate(capacity, false);
    }

    ~Fifo() {
        buffer.free();
    }

    //pops to spec component buffer
    int pop(T* dest, int numToRead) {
        int start1, size1, start2, size2;
        fifo.prepareToRead(numToRead, start1, size1, start2, size2);
        int numRead = size1 + size2;
        if (size1 > 0) {
            std::copy(buffer + start1, buffer + start1 + size1, dest);
        }
        if (size2 > 0) {
            std::copy(buffer + start2, buffer + start2 + size2, dest + size1);
        }
        fifo.finishedRead(numRead);
        return numRead;
    }

    //clear on prepare
    void clear() {
        fifo.reset();
    }

    //called in process block to push all buffer samples based on channel amount
    void getBufferSamples(const T* left, const T* right, const int numSamples) {
        if (right) {
            for (int i = 0; i < numSamples; ++i) {
                pushSample((left[i] + right[i]) * 0.5f);
            }
        }
        else {
            pushBlock(left, numSamples);
        }
    }

private:
    void pushSample(T sample) {
        int start1, size1, start2, size2;
        fifo.prepareToWrite(1, start1, size1, start2, size2);
        if (size1 > 0) {
            buffer[(size_t)start1] = sample;
        }
        if (size2 > 0) {
            buffer[(size_t)start2] = sample;
        }
        fifo.finishedWrite(size1 + size2);
    }

    void pushBlock(const T* block, const int numSamples) {
        int start1, size1, start2, size2;
        fifo.prepareToWrite(numSamples, start1, size1, start2, size2);
        if (size1 > 0) {
            std::memcpy(buffer + start1, block, size1);
        }
        if (size2 > 0) {
            std::memcpy(buffer + start2, block, size2);
        }
        fifo.finishedWrite(size1 + size2);
    }

    juce::AbstractFifo fifo;
    juce::HeapBlock<T> buffer;
};

//peak structs to pass peaks of each block to peak UI component
struct PeakMeasurement {
    //destructive read
    float read() noexcept {
        return value.exchange(0.0f);
    }
    //only called in process block per channel. gets peak and calls update per block
    void getPeakFromBlock(const float* block, const int numSamples) {
        float peakValue = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            float absSample = std::abs(block[i]);
            if (absSample > peakValue) {
                peakValue = absSample;
            }
        }
        update(peakValue);
    }
private:
    std::atomic<float> value{ 0.0f };
    //write peak from block to value
    void update(float newValue) noexcept {
        auto oldValue = value.load(std::memory_order_relaxed);
        while (newValue > oldValue && !value.compare_exchange_weak(oldValue, newValue, std::memory_order_release, std::memory_order_relaxed));
    }
};
