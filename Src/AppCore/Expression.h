#ifndef APPCORE_EXPRESSION_H
#define APPCORE_EXPRESSION_H

#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <functional>
#include <memory>
#include "ParserTypes.h"

namespace FileSearch {
namespace AppCore{

struct ExprNode;
class RuntimeInfo;

class EvaluateException : public std::runtime_error {
public:
    EvaluateException(const std::string& msg, int pos = -1) : std::runtime_error(msg), pos_(pos) {}
    EvaluateException(const EvaluateException& ex) : std::runtime_error(ex), pos_(ex.pos_) {}
    int pos() {
        return pos_;
    }
protected:
    int pos_;
};

typedef std::function<ExprValue()> PropertyCalcCallback;
typedef std::unordered_map<std::string, PropertyCalcCallback> PropertyMap;

class Expression : public boost::noncopyable {
public:
    Expression(ExprNodePtr root, std::shared_ptr<RuntimeInfo> runtimeInfo);
    virtual ~Expression();
    std::shared_ptr<RuntimeInfo> runtimeInfo() const;
    friend class Parser;

    ExprValue evaluate(PropertyMap&& properties = PropertyMap());
protected:
    ExprNodePtr root_;
    std::shared_ptr<RuntimeInfo> runtimeInfo_;
    ExprValue result_;
    PropertyCalcCallback propertyCallback_;
    PropertyMap properties_;

    ExprValue calcTree(const ExprNodePtr& tree);
    bool operationContains(const ExprNodePtr& node);
    bool operationCompare(const ExprNodePtr& node);
    bool operationEquals(const ExprNodePtr& node);
    bool operationLogical(const ExprNodePtr& node);
    bool operationIs(const ExprNodePtr& node);
    bool operationIsNot(const ExprNodePtr& node);
    bool operationNotEquals(const ExprNodePtr& node);
    void delTree(ExprNodePtr tree);
    ExprValue getPropertyValue(const std::string& propertyName);
};

}
}
#endif