#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include "VariableStorage.h"

namespace numhop {

enum ExpressionOperatorT {AssignmentT, AdditionT, SubtractionT, MultiplicationT, DivisionT, PowerT, ValueT, UndefinedT};
class Expression
{
public:
    Expression();
    Expression(const Expression &other);
    Expression(const std::string &exprString, ExpressionOperatorT op, const std::string &lhs="", const std::string &rhs="");
    ~Expression();

    Expression& operator= (const Expression &other);

    bool empty() const;
    bool isValue() const;

    const std::string &exprString() const;
    ExpressionOperatorT operatorType() const;

    double evaluate(VariableStorage &rVariableStorage, bool &rEvalOK);

    std::string print();
    void setHadOuterParanthesis(bool tf);

protected:
    void copyFromOther(const Expression &other);

    std::string mExpressionString;
    bool mHadOuterParanthesis;
    Expression *mpLhs, *mpRhs;
    ExpressionOperatorT mOperator;

};

bool interpretExpressionStringRecursive(std::string exprString, Expression &rExp);

}

#endif // EXPRESSION_H
