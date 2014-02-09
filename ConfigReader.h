#ifndef TRADER_CONFIG_READER_H_
#define TRADER_CONFIG_READER_H_

#include <map>
#include <string>

namespace ConfigReader
{
    int GetFrontAddr(std::map<std::string, std::pair<std::string, std::string>>& output);
}

#endif;

