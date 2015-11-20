#ifndef HELPFUNCTIONS_H
#define HELPFUNCTIONS_H

#include <string>
#include <list>

namespace numhop {

void splitExprRows(const std::string &expr, const char &comment, std::list<std::string> &rExpressions);
void stripLeadingTrailingWhitespaces(std::string &rString);
bool stripLeadingTrailingParanthesis(std::string &rString, bool &rDidStrip);
void fixMultiDivision(std::string &rString);

inline bool contains(const std::string &rString, char c)
{
    return (rString.find_first_of(c) != std::string::npos);
}

}

#endif // HELPFUNCTIONS_H
