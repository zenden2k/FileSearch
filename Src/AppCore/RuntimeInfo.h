#ifndef APPCORE_RUNTIMEINFO_H
#define APPCORE_RUNTIMEINFO_H

#include <string>
#include <set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include "Expression.h"

namespace FileSearch {
namespace AppCore{

struct ExprNode;

class RuntimeInfo : public boost::noncopyable {
public:
    RuntimeInfo();
    virtual ~RuntimeInfo();
    void init();
    typedef  std::function<ExprValue(ExprValue)> UnaryFunction;
    typedef  std::function<ExprValue(const std::string&)> PropertyCalcCallback;

    void registerFunction(const std::string& funcName, UnaryFunction&& func);
    bool functionExists(const std::string& funcName) const;
    void registerConstant(const std::string& name, const ExprValue& value);
    bool constantExists(const std::string& name) const;
    ExprValue getConstant(const std::string& name) const;

    ExprValue callFunction(ExprNode* node, ExprValue left);
    bool propertyExists(const std::string& propertyName) const;

private:
    std::unordered_map<std::string, UnaryFunction> unaryFunctions_;
    std::set<std::string> knownProperties_;
    std::unordered_map<std::string, ExprValue> constants_;
};

}
}
#endif