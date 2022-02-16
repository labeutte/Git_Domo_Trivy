/* 
 * File:   Log.h
 */
#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <cstring>
#include <sstream>

class Log {
    std::ostream *m_ostream; //!< Le flux de sortie
public:
    Log();
    void wLine(
            const std::string &tag,
            const std::string &message,
            const std::string &dat=""
            );
    void wPoint();
    void setOutput(std::ostream &flux);
};
#endif /* LOG_H */
