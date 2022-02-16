/* 
 * File:   Log.cpp
 */
#include "Log.h"

Log::Log() {
}

void Log::wLine(const std::string &tag, const std::string &message, const std::string &dat) {
	//*m_log.m_ostream << m_log.m_logLabels[level] << tag << " : " << message << std::endl << std::flush;
	*this->m_ostream << tag << " : " << dat << " : " << message << std::endl << std::flush;
}

void Log::wPoint() {
	*this->m_ostream << "." << std::flush;
}

void Log::setOutput(std::ostream &flux) {
	m_ostream = &flux;
}
