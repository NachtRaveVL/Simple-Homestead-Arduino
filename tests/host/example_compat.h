#ifndef TerraduinoExampleCompat_H
#define TerraduinoExampleCompat_H

#include <stdint.h>
#include <iostream>
#include <string>
#include <cctype>

#ifndef F
#define F(value) value
#endif

class String : public std::string {
public:
    using std::string::string;
    String() : std::string() { }
    String(const std::string &value) : std::string(value) { }
    void toLowerCase() {
        for (size_t i = 0; i < size(); ++i) {
            (*this)[i] = (char)std::tolower((unsigned char)(*this)[i]);
        }
    }
};

inline uint32_t millis() { return 1U; }
inline void delay(unsigned long) { }

class TerraExampleSerial {
public:
    void begin(unsigned long) { }
    explicit operator bool() const { return true; }

    template<typename T>
    void print(const T &value) { std::cout << value; }

    void print(char value) { std::cout << value; }
    void print(float value, int) { std::cout << value; }
    void print(double value, int) { std::cout << value; }

    template<typename T>
    void println(const T &value) { std::cout << value << '\n'; }

    void println(float value, int) { std::cout << value << '\n'; }
    void println(double value, int) { std::cout << value << '\n'; }

    void println() { std::cout << '\n'; }
};

static TerraExampleSerial Serial;

#endif
