#pragma once

#include <string>
#include <unordered_map>
#include <fstream>

constexpr const char* whitespace = " \t\r\n\v\f";
static std::string trim(const std::string& s)
{
    std::string result = s;
    size_t pos = result.find_first_not_of(whitespace);
    if (std::string::npos != pos) result = result.substr(pos);
    pos = result.find_last_not_of(whitespace);
    if (std::string::npos != pos) result = result.substr(0, pos + 1);
    return result;
}

class IniReader
{
    using Section = std::unordered_map<std::string, std::string>;
public:
    IniReader() = default;
    bool read(const std::string& filename)
    {
        std::string curr_section;
        std::ifstream fstrm(filename, std::ios::in);
        if (!fstrm.is_open())
            return error("Failed to open " + filename);

        size_t pos;
        Section section;
        for (std::string line; std::getline(fstrm, line);)
        {
            if (line.empty() || line[0] == ';' || line[0] == '#')
                continue;
            if (line[0] == '[')  // section
            {
                pos = line.find_first_of(']');
                if (pos == std::string::npos)
                    return error("Invalid syntax: " + line);
                
                if (!curr_section.empty())  // this is not the first section
                {
                    _sections[curr_section] = section;  // add the current section
                    curr_section.clear();
                    section.clear();
                }
                curr_section = line.substr(1, pos - 1);
            }
            else  // key-value pair
            {
                pos = line.find_first_of("=:");
                if (pos != std::string::npos)
                {
                    std::string name = trim(line.substr(0, pos));
                    std::string value = trim(line.substr(pos + 1));
                    section[name] = value;

                }
                else
                    return error("Invalid syntax: " + line);
            }
        }

        // add the last section
        if (!curr_section.empty())
            _sections[curr_section] = section;  // add the current section
        return true;
    }

    std::string get(const std::string& section, const std::string& key)
    {
        std::string result;
        const auto& keyvals = _sections.find(section);
        if (keyvals != _sections.end())
        {
            const auto& val = keyvals->second.find(key);
            if (val != keyvals->second.end())
                result = val->second;
        }
        return result;
    }

    std::string get_last_error() const { return _error; }

private:
    std::unordered_map<std::string, Section> _sections;
    std::string _error;
    bool error(const std::string& s) { _error = s; return false; }
};
