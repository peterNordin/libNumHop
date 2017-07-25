#ifndef HELPFUNCTIONS_H
#define HELPFUNCTIONS_H

#include <string>
#include <list>
#include <vector>

namespace numhop {

void extractExpressionRows(const std::string &script, const char &commentChar, std::list<std::string> &rScriptExpressions);
void removeAllWhitespaces(std::string &rString);
void stripLeadingTrailingWhitespaces(std::string &rString);
bool stripLeadingTrailingParanthesis(std::string &rString, bool &rDidStrip);
char stripInitialSign(std::string &rString);
void stripInitialPlus(std::string &rString);

template <typename ContainerT, typename KeyT>
bool contains(const ContainerT& container, const KeyT& key)
{
    return container.find(key) != container.end();
}

template <>
inline bool contains<std::string, char>(const std::string& container, const char& key)
{
    return container.find(key) != std::string::npos;
}

inline bool containsAnyof(const std::string &str, const std::string &match)
{
    return (str.find_first_of(match) != std::string::npos);
}

inline bool starts_with(const std::string &str, const std::string &what) {
  return str.compare(0, what.size(), what) == 0;
}

inline std::vector<std::string> extractArguments(const std::string &text)
{
  std::vector<std::string> args;
  size_t b = text.find_first_of('(');
  size_t n = text.find_first_of(',', b);
  while(n != std::string::npos) {
    args.push_back(text.substr(b+1, n-b));
    b = n + 1;
    n = text.find_first_of(',', b);
  }
  n = text.find_first_of(')', b);
  // todo check n
  args.push_back(text.substr(b+1, n-b));
  return args; 
}

enum class ScriptSequenceType {NummericExpression, If};
ScriptSequenceType checkRowSequenceType(const std::string &text) {
  if (starts_with(text, "if(")) {
    return ScriptSequenceType::If;
  } else {
    return ScriptSequenceType::NummericExpression;
  }
}


class ScriptItem
{
public:
  virtual void evaluate(VariableStorage &rVariableStorage, bool &rEvalOK) = 0;
  virtual std::list<std::string>::iterator interpret(std::list<std::string>::iterator start, std::list<std::string>::iterator end) = 0;
};

class ScriptSequence
{

public:
  std::list<ScriptItem*> mItems;
};

class ExpressionsItem : public ScriptItem
{
public:
  void evaluate(VariableStorage &rVariableStorage, bool &rEvalOK);
  std::list<std::string>::iterator interpret(std::list<std::string>::iterator start, std::list<std::string>::iterator end) {
    std::list<std::string>::iterator it;
    for (it=start; it!=end; ++it) {
      if (checkRowSequenceType(*it) == ScriptSequenceType::NummericExpression) {
        numhop::Expression e;
        numhop::interpretExpressionStringRecursive(*it, e);
        expressions.push_back(e);
      } else {
        return it;
      }
    }
    return end;
  }

protected:
  std::vector<Expression> expressions;
};


class IfItem : public ScriptItem
{
public:
  void evaluate(VariableStorage &rVariableStorage, bool &rEvalOK) {
    std::vector< std::pair<Expression, ScriptSequence> >::iterator it;
    for (it=conditions.begin(); it!=conditions.end(); ++it) {
      if ((it->first.evaluate(rVariableStorage, rEvalOK) == 1) && rEvalOK ) {
        it->first.evaluate(rVariableStorage, rEvalOK);
        break;
      }
    }
  }

  void interpret(std::list<std::string>::iterator start, std::list<std::string>::iterator &end ) {

    size_t if_ctr = 0;
    std::list<std::string>::iterator section_start;
    std::list<std::string>::iterator section_end;
    if (starts_with(*start, "if(") && if_ctr==0) {
      ++if_ctr;
      std::vector<std::string> args = extractArguments(*start);
      if (args.size() == 1) {
        Expression e;
        numhop::interpretExpressionStringRecursive(args[0], e);
        ScriptSequence ss;
        conditions.push_back(std::pair<Expression, ScriptSequence>(e,ss));
        section_start = ++start;
        section_end = section_start;

      }
      // else error


    } else if (starts_with(row, "elif(") && if_ctr==1) {
      // First process last section
      conditions.back().second = processScript(section_start, section_stop);


      //same as if
    } else if (starts_with(row, "else") && if_ctr==1) {
      // First process last section
      conditions.back().second = processScript(section_start, section_stop);

      // todo check rest of line empty
      Expression e;
      interpretRecursively("1", e);
      ScriptSequence ss;
      conditions.push_back(std::pair<Expression, ScriptSequence>(e,ss));
    } else if (starts_with(row, "endif") && if_ctr==1) {
      // todo check rest of line empty
      --if_ctr;

      if (if_ctr == 0) {
        // Process last section
        conditions.back().second = processScript(section_start, section_stop);
      }

    } else if( if_ctr>=1) {
      // Increment if section end iterator
      ++section_end;

    } else {
      // Fail
    }
  }

private:
  std::vector< std::pair<Expression, ScriptSequence> > conditions;


};

ScriptSequence* processScript(std::list<std::string>::iterator start, std::list<std::string>::iterator end) {

  ScriptSequence* pScriptSequence = new ScriptSequence();
  std::list<std::string>::iterator it;
  for (it=start; it!=end; ++it) {

    ScriptItem* pScriptItem=0;
    std::string& row = *it;
    ScriptSequenceType type = checkRowSequenceType(row);
    if (type == ScriptSequenceType::If) {
      pScriptItem = new IfItem();
      it = IfItem->interpret(it, end);
    } else {
      pScriptItem = new ExpressionsItem();
      it = pScriptItem->interpret(it, end);
    }

    pScriptSequence->mItems.push_back(pScriptItem);

  }
}

#endif // HELPFUNCTIONS_H
