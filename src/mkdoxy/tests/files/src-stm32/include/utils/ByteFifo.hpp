#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <stdint.h>

/// A DMA-ready byte ring buffer with contiguous I/O API
/// Allows direct read/write access using:
/// readableSpan() -> read-out -> notifyRead()
/// writeableSpan() -> write-in -> notifyWritten()
template <int Size> class ByteFifo {
    std::array<uint8_t, Size> m_fifo;
    int m_head, m_tail;

    int static adjust(int index, int delta) {
        index += delta;
        if (index >= Size) {
            index -= Size;
        }
        if (index < 0) {
            index += Size;
        }
        return index;
    }

public:
    ByteFifo()
        : m_head(0)
        , m_tail(0) {}

    /// Pointer to the beginning
    uint8_t* data() const { return (uint8_t*)m_fifo.data(); }

    /// Total capacity
    constexpr size_t size() const { return m_fifo.size(); }

    /// Readable bytes
    size_t available() const {
        if (m_head >= m_tail) {
            return m_head - m_tail;
        } else {
            return Size - m_tail + m_head;
        }
    }

    /// True if some data is ready for reading.
    bool hasData() const { return m_head != m_tail; }

    /// Override the write index aka. head
    void setHead(int newHead) { m_head = newHead; }

    /// Clears the buffer by setting m_tail = m_head;
    void clear() { m_tail = m_head; }

    /// Write len bytes into buffer starting at data.
    void writeSpan(uint8_t* data, size_t len) {
        if (len > Size) {
            data += len - Size;
            len = Size;
        }

        const size_t chunk = std::min(size_t(Size - m_head), len);
        std::copy_n(data, chunk, this->data() + m_head);
        if (len > chunk) {
            std::copy_n(data + chunk, len - chunk, this->data());
        }
        notifyWritten(len);
    }

    /// Gets a contiguous range of bytes ready for writing.
    /// Doesn't represent all writeable bytes if wrapped over the end.
    /// Zero size_t means FIFO is full.
    std::pair<uint8_t*, size_t> writeableSpan() const {
        int preTail = adjust(m_tail, -1);
        return m_head >= preTail
            ? std::make_pair(data() + m_head, std::max(0, int(size()) - m_head))
            : std::make_pair(data() + m_head, std::max(0, preTail - m_head));
    }

    /// Move the write index (head), indicating len bytes have been written.
    void notifyWritten(size_t len) { m_head = adjust(m_head, len); }

    /// Reads one byte, must be available.
    uint8_t pop() {
        auto value = m_fifo[m_tail];
        m_tail = adjust(m_tail, 1);
        return value;
    }

    /// Writes one byte
    void push(uint8_t b) {
        m_fifo[m_head] = b;
        notifyWritten(1);
    }

    /// Read len bytes into buffer starting at data.
    void peekSpan(uint8_t* data, size_t len) {
        const size_t chunk = std::min(size_t(Size - m_tail), len);
        memcpy(data, this->data() + m_tail, chunk);
        if (chunk < len) {
            memcpy(data + chunk, this->data(), len - chunk);
        }
    }

    /// Gets a contiguous range of bytes ready for reading.
    /// Doesn't represent all readable bytes if wrapped over the end.
    /// Zero size_t means FIFO is empty.
    std::pair<uint8_t*, size_t> readableSpan() const {
        return m_head >= m_tail
            ? std::make_pair(data() + m_tail, m_head - m_tail)
            : std::make_pair(data() + m_tail, int(size()) - m_tail);
    }

    /// Move the read index (tail), indicating len bytes have been read out.
    void notifyRead(size_t len) { m_tail = adjust(m_tail, len); }
};
