#ifndef HELPFUNCTIONS_H
#define HELPFUNCTIONS_H

#include <string>
#include <list>

namespace numhop {

//void splitExprRows(const std::string &expr, const char &comment, std::list<std::string> &rExpressions);
void extractExpressionRows(const std::string &expr, const char &comment, std::list<std::string> &rExpressions);
void removeWhitespaces(std::string &rString);
void stripLeadingTrailingWhitespaces(std::string &rString);
bool stripLeadingTrailingParanthesis(std::string &rString, bool &rDidStrip);
void fixMultiDivision(std::string &rString);
void fixMultiSubtraction(std::string &rString);
char stripInitialSign(std::string &rString);

inline bool contains(const std::string &str, const char c)
{
    return (str.find_first_of(c) != std::string::npos);
}

inline bool contains(const std::string &str, const std::string &match)
{
    return (str.find_first_of(match) != std::string::npos);
}

}

#endif // HELPFUNCTIONS_H
