#ifndef __STL_UTIL_H__
#define __STL_UTIL_H__

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

namespace StlUtil
{
    std::string StringTrim(std::string& str, char ch = ' ');

    int Split(const std::string& str, char delim, std::vector<std::string>& out);
    int Split(const std::string& str, const std::string& delim, std::vector<std::string>& out);
}

#endif

