#ifndef LOG_H
#define LOG_H

#include <sstream>

class Log
{
public:
    ~Log();

    std::ostringstream &get();

private:
    std::ostringstream _stream;
};

#endif // LOG_H
