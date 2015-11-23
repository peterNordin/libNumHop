#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <list>
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
    bool mIsNegative;
    Expression *mpLhs, *mpRhs;
    ExpressionOperatorT mOperator;

};



class Expression2
{
public:
    Expression2();
    Expression2(const Expression &other);
    Expression2(const std::string &exprString, ExpressionOperatorT op);
    Expression2(const std::string &leftExprString, const std::string &rightExprString, ExpressionOperatorT op);
    ~Expression2();

    Expression2& operator= (const Expression2 &other);

    bool empty() const;
    bool isValue() const;

    const std::string &exprString() const;
    const std::string &leftExprString() const;
    const std::string &rightExprString() const;
    ExpressionOperatorT operatorType() const;

    double evaluate(VariableStorage &rVariableStorage, bool &rEvalOK);

    std::string print();

protected:
    void copyFromOther(const Expression2 &other);

    std::string mLeftExpressionString, mRightExpressionString;
    bool mHadLeftOuterParanthesis, mHadRightOuterParanthesis;
    std::list<Expression2> mLeftChildExpressions, mRightChildExpressions;
    ExpressionOperatorT mOperator;
};

bool interpretExpressionStringRecursive(std::string exprString, Expression &rExp);
bool interpretExpressionStringRecursive2(std::string exprString, std::list<Expression2> &rExp);
bool interpretExpressionStringRecursive2(std::string exprString, Expression2 &rExp);

}

#endif // EXPRESSION_H
