#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>

#include "numhop.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Local variable storage wrapper class
// local means local variables in this application,
// they are seen as external variables inside libnumhop
class LocalVSWrapper : public numhop::ExternalVariableStorage
{
public:
  LocalVSWrapper(std::map<std::string, double> *pExtVars)
  {
    mpVars = pExtVars;
  }

  double externalValue(std::string name, bool &rFound) const
  {
    if (mpVars->count(name) > 0) {
      rFound = true;
      return mpVars->at(name);
    }
    rFound = false;
    return 0;
  }

  bool setExternalValue(std::string name, double value)
  {
    std::map<std::string, double>::iterator it = mpVars->find(name);
    if (it != mpVars->end()) {
      //cout << "external found" << endl;
      it->second = value;
      return true;
    }
    return false;
  }

private:
  std::map<std::string, double> *mpVars;
};

void test_allok(const std::string &exprs, const double expected_result, numhop::VariableStorage &variableStorage){

  std::list<std::string> exprlist;
  numhop::extractExpressionRows(exprs, '#', exprlist);

  INFO("Full expression: " << exprs);
  double value_first_time, value_second_time; 
  std::list<std::string>::iterator it;
  for (it=exprlist.begin(); it!=exprlist.end(); ++it) {
    INFO("Current sub expression: " << *it);
    numhop::Expression e;
    bool interpretOK = numhop::interpretExpressionStringRecursive(*it, e);
    REQUIRE(interpretOK == true);
            
    bool first_eval_ok, second_eval_ok;;
    value_first_time = e.evaluate(variableStorage, first_eval_ok);
    REQUIRE(first_eval_ok == true);
    // evaluate again, should give same result does not work with a=a+1
    //value_second_time = e.evaluate(variableStorage, second_eval_ok);
    //REQUIRE(second_eval_ok == true);

    //REQUIRE(value_first_time == value_second_time);
  }
  
  REQUIRE(value_first_time == Approx(expected_result));
}

void test_interpret_fail(const std::string &expr)
{
  INFO("Full expression: " << expr);
  numhop::Expression e;
  bool interpretOK = numhop::interpretExpressionStringRecursive(expr, e);
  REQUIRE(interpretOK == false);
  //todo what happens if e is evaluated ?          
}

void test_eval_fail(const std::string &expr, numhop::VariableStorage &variableStorage)
{
  INFO("Full expression: " << expr);
  numhop::Expression e;
  bool interpretOK = numhop::interpretExpressionStringRecursive(expr, e);
  REQUIRE(interpretOK == true);
  bool first_eval_ok;
  double value_first_time = e.evaluate(variableStorage, first_eval_ok);
  REQUIRE(first_eval_ok == false);         
}



TEST_CASE("Variable Assignment") {
  numhop::VariableStorage vs;
  test_allok("a=5;a=8;a;", 8, vs);
  test_allok("a=6;\n a=7.14\n a;", 7.14, vs);
  test_allok("a=6;\n a=a+1\n", 7, vs);
}

TEST_CASE("External Variables") {
  numhop::VariableStorage vs;

  std::map<std::string, double> externalVars;
  LocalVSWrapper ev_wrapper(&externalVars);
  vs.setExternalStorage(&ev_wrapper);
  externalVars.insert(std::pair<std::string,double>("dog", 55));
  externalVars.insert(std::pair<std::string,double>("cat", 66));

  test_allok("dog", 55, vs);
  test_allok("cat", 66, vs);

  test_allok("dog=4; 1-(-2-3-(-dog-5.1))", -3.1, vs);
  test_allok("-dog", -4, vs);
  REQUIRE(externalVars["dog"] == 4);

  test_allok("cat \n dog \r dog=5;cat=2.1;a=3;b=dog*cat*a;b", 31.5, vs);
  REQUIRE(externalVars["cat"] == 2.1);
}

TEST_CASE("Reserved Variable") {
  numhop::VariableStorage vs;
  vs.reserveVariable("pi", 3.1415);
  std::map<std::string, double> ev;
  LocalVSWrapper ev_wrapper(&ev);
  vs.setExternalStorage(&ev_wrapper);

  // Add external pi variable
  ev.insert(std::pair<std::string,double>("pi", 10000));

  // Here the reserved pi should be used, not the external one
  test_allok("pi", 3.1415, vs);
  test_allok("a=pi*2", 6.283, vs);

  // It should not be possible to change the external pi, or the reserved value
  test_eval_fail("pi=123", vs);
  test_allok("pi", 3.1415, vs);
  REQUIRE(ev["pi"] == 10000 );
}

TEST_CASE("Expression Parsing") {
  numhop::VariableStorage vs;
  std::string expr = " \t #   \n    a=5;\n #   a=8\n a+1; \r\n a+2 \r a+3 \r\n #Some comment ";

  // Extract individual expression rows, treat # as comment
  // ; \r \n breaks lines
  // The expression above contains 4 actual valid sub expressions
  std::list<std::string> exprlist;
  numhop::extractExpressionRows(expr, '#', exprlist);
  REQUIRE(exprlist.size() == 4);

  // Result should be a+3 with a = 5
  test_allok(expr, 8, vs);
}

TEST_CASE("Various Math") {
  numhop::VariableStorage vs;

  test_allok("2^2", 4, vs);
  test_allok("2^(1+1)", 4, vs);
  test_allok("7/3/4/5", 0.11666667, vs);
  test_allok("7/(3/(4/5))", 1.8666667, vs);
  test_allok("(4/3*14*7/3/4/5*5/(4*3/2))", 1.8148148148148147, vs);
  test_allok("1-2*3-3*4-4*5;", -37, vs);
  test_allok("1-(-2-3-(-4-5))", -3, vs);
  test_allok("-1-2-3*4-4-3", -22, vs);
  test_allok("-1-(2-3)*4-4-3", -4, vs);
  test_allok("-(((-2-2)-3)*4)", 28, vs);

  // Test use of multiple + - 
  test_allok("2--3;", 5, vs);
  test_allok("1+-3", -2, vs);
  test_allok("1-+3", -2, vs);
  test_allok("1++3", 4, vs);
  test_allok("1---3", -2, vs);
  
  test_allok("a=1;b=2;c=3;d=a+b*c;d",7, vs);
  // Reuse b and a from last expression (stored in vs)
  test_allok("b^b;", 4, vs);
  test_allok("b^(a+a)", 4, vs);
  test_allok("a-b;", -1, vs);
  test_allok("a-b+a", 0, vs);
  test_allok("-a+b", 1, vs);
  test_allok("b-a;", 1, vs);
  test_allok("(-a)+b", 1, vs);
  test_allok("b+(-a);", 1, vs);
  test_allok("b+(+a)", 3, vs);
  test_allok("b+a", 3, vs);
  test_allok("+a", 1, vs);
  test_allok("0-(a-b)+b", 3, vs);
  
  test_allok("a=1;b=2;c=3;d=4; a-b+c-d+a", -1, vs);
  test_allok("a=1;b=2;c=3;d=4; a-b-c-d+a", -7, vs);
  test_allok("a=1;b=2;c=3;d=4;a=(3+b)+4^2*c^(2+d)-7*(d-1)", 11648, vs);
  // Reuse resulting a from last expression 
  test_allok("b=2;c=3;d=4;a=(3+b)+4^2*c^(2+d)-7/6/5*(d-1)", 11668.299999999999, vs);

  test_allok("value=5;", 5, vs);
  test_allok("value+2;", 7, vs);
  test_allok("value-2", 3, vs);
  test_allok("value*1e+2", 500, vs);
}

TEST_CASE("Exponential Notation") {
  numhop::VariableStorage vs;
  test_allok("2e-2-1E-2", 0.01, vs);
  test_allok("1e+2+3E+2", 400, vs);
  test_allok("1e2+1E2", 200, vs);
}

TEST_CASE("Boolean Expressions") {
  numhop::VariableStorage vs;

  test_allok("2<3", 1, vs);
  test_allok("2<2", 0, vs);
  test_allok("4.2>2.5", 1, vs);

  // Note the difference on how the minus sign is interpreted
  test_allok("(-4.2)>3", 0, vs);
  // In this second case, the expression is treated as -(4.2>3)
  test_allok("-4.2>3", -1, vs);

  test_allok("1|0", 1, vs);
  test_allok("0|0", 0, vs);
  test_allok("0|0|0|1", 1, vs);
  test_allok("1|1|1|1|1", 1, vs);
  test_allok("2|3|0", 1, vs);
  test_allok("(-2)|3", 1, vs);
  test_allok("(-1)|(-2)", 0, vs);

  test_allok("1&0", 0, vs);
  test_allok("0&0", 0, vs);
  test_allok("(-1)&1.5", 0, vs);
  test_allok("1&1", 1, vs);
  test_allok("1&1&1&0.4", 0, vs);
  test_allok("1&0&1", 0, vs);

  test_allok("2<3 | 4<2", 1, vs);
  test_allok("2<3 & 4<2", 0, vs);
  test_allok("2<3 & 4>2", 1, vs);
  test_allok("x=2.5; (x>2&x<3)*1+(x>3&x<4)*2", 1, vs);
  test_allok("x=3.6; (x>2&x<3)*1+(x>3&x<4)*2", 2, vs);
}

TEST_CASE("Expressions that should fail") {
  numhop::VariableStorage vs;
  test_interpret_fail("2*-2");
  test_interpret_fail("a += 5");
  test_interpret_fail("1+1-");
  test_interpret_fail(" = 5");
  test_eval_fail("0.5huj", vs);
}
