#include "common/cstdio/printf.h"

#include <cstdarg>

#include "common/cstdio/putc.h"
#include "common/cstdio/puts.h"
#include "platforms/platform.h"

namespace {

void ulitoa(uint64_t value, uint32_t base, bool upper_cause, char* str) {
  uint64_t v = 1;
  while (value / v >= base) {
    v *= base;
  }

  int n = 0;
  while (v != 0) {
    int digit = value / v;
    value %= v;
    v /= base;
    if (n || digit > 0 || v == 0) {
      *str++ = digit + (digit < 10 ? '0' : (upper_cause ? 'A' : 'a') - 10);
      n++;
    }
  }
  *str = 0;
}

void litoa(long value, char* str) {
  if (value < 0) {
    value = -value;
    *str++ = '-';
  }
  ulitoa(value, 10, 0, str);
}

void uitoa(uint32_t value, uint8_t base, bool upper_cause, char* str) {
  uint32_t v = 1;
  while (value / v >= base) {
    v *= base;
  }

  int n = 0;
  while (v != 0) {
    int digit = value / v;
    value %= v;
    v /= base;
    if (n || digit > 0 || v == 0) {
      *str++ = digit + (digit < 10 ? '0' : (upper_cause ? 'A' : 'a') - 10);
      n++;
    }
  }
  *str = 0;
}

void itoa(int value, uint8_t base, char* str) {
  if (value < 0) {
    value = -value;
    *str++ = '-';
  }
  uitoa(value, base, false, str);
}

int atoi(const char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  } else if ('A' <= c && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

void parse_field_width(char& c, char** src, int& res) {
  constexpr int kBase = 10;
  char* p = *src;
  int digit;

  res = 0;
  while ((digit = atoi(c)) != -1) {
    if (digit > kBase) {
      break;
    }
    res = res * kBase + digit;
    c = *p++;
  }
  *src = p;
}

void puts(int field_width, bool flag_zero, char* buf) {
  // Fill empty fields with zero or space
  {
    char* p = buf;
    while (*p++ && field_width > 0) {
      field_width--;
    }
    const char c = flag_zero ? '0' : ' ';
    while (field_width-- > 0) {
      evisor::putc(c);
    }
  }

  while (auto c = *buf++) {
    evisor::putc(c);
  }
}

void vfprintf(char* format, va_list va) {
  char buf[16];

  while (auto c = *(format++)) {
    // %
    if (c != '%') {
      evisor::putc(c);
      continue;
    }
    c = *(format++);

    // [flags]
    bool flag_zero = false;
    if (c == '0') {
      flag_zero = true;
      c = *(format++);
    }

    // [minimum field width]
    int field_width = 0;
    if ('0' <= c && c <= '9') {
      parse_field_width(c, &format, field_width);
    }

    // [precision]
    bool precision_long = false;
    if (c == 'l') {
      precision_long = true;
      c = *(format++);
    }

    // argument type
    switch (c) {
      case 'd':
      case 'D':
        if (precision_long) {
          litoa(va_arg(va, uint64_t), buf);
        } else {
          itoa(va_arg(va, int32_t), 10, buf);
        }
        puts(field_width, flag_zero, buf);
        break;
      case 'u':
      case 'U':
        if (precision_long) {
          ulitoa(va_arg(va, uint64_t), 10, 0, buf);
        } else {
          uitoa(va_arg(va, uint32_t), 10, false, buf);
        }
        puts(field_width, flag_zero, buf);
        break;
      case 'x':
      case 'X':
        if (precision_long) {
          ulitoa(va_arg(va, uint64_t), 16, (c == 'X'), buf);
        } else {
          uitoa(va_arg(va, uint32_t), 16, (c == 'X'), buf);
        }
        puts(field_width, flag_zero, buf);
        break;
      case 'c':
      case 'C':
        evisor::putc(static_cast<uint8_t>(va_arg(va, int32_t)));
        break;
      case 's':
      case 'S':
        puts(field_width, 0, va_arg(va, char*));
        break;
      case '%':
        evisor::putc(c);
      default:
        break;
    }
  }
}

}  // namespace

namespace evisor {

void printf(const char* format, ...) {
  va_list va;
  va_start(va, format);
  vfprintf(const_cast<char*>(format), va);
  va_end(va);
}

}  // namespace evisor
