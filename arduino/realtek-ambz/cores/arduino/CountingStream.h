#include <Print.h>

class CountingStream : public Print
{
  virtual size_t write(uint8_t) { return 1; };
  virtual size_t write(const uint8_t *buffer, size_t size) { return size; };
};

