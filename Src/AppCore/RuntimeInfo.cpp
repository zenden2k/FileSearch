#include "RuntimeInfo.h"

#include <boost/format.hpp>
#include "parser.h"
#include "Utils.h"

namespace FileSearch {
namespace AppCore{

RuntimeInfo::RuntimeInfo() {
    init();
}

RuntimeInfo::~RuntimeInfo() {

}

void RuntimeInfo::init() {
    knownProperties_.insert("size");
    knownProperties_.insert("name");
    knownProperties_.insert("attribute");

    auto attrStrings = GetFileAttributeStrings();
    int attrCount = attrStrings.size();
    for (int i = 0; i < attrCount; i++) {
        registerConstant(attrStrings[i], static_cast<FileAttribute>(i));
    }

    registerFunction("strlen", [](ExprValue val)->ExprInteger {
        try {
            size_t len = Utils::Utf8ToWideString(boost::get<std::string>(val)).size();
            return len;
        } catch (boost::bad_get&) {
            throw ParserException("Invalid argument type for strlen function");
        }
    });
}

void RuntimeInfo::registerFunction(const std::string& funcName, UnaryFunction&& func) {
    if (functionExists(funcName)) {
        throw ParserException(str(boost::format("Function %s already registered") % funcName));
    }
    unaryFunctions_[funcName] = std::move(func);
}

bool RuntimeInfo::functionExists(const std::string& funcName) const {
    return unaryFunctions_.find(funcName) != unaryFunctions_.end();
}

ExprValue RuntimeInfo::callFunction(ExprNode* node, ExprValue left) {
    std::string funcName = boost::get<std::string>(node->value);
    try {
        return unaryFunctions_[funcName](left);
    } catch (InvalidFunctionArgument&) {
        throw EvaluateException(boost::str(boost::format("Call to function '%s': Invalid argument") % funcName));
    }
}

bool RuntimeInfo::propertyExists(const std::string& propertyName) const {
    return knownProperties_.find(propertyName) != knownProperties_.end();
}

void RuntimeInfo::registerConstant(const std::string& name, const ExprValue& value) {
    if (functionExists(name)) {
        throw ParserException(str(boost::format("Constant %s already registered") % name));
    }
    constants_[name] = value;
}

bool RuntimeInfo::constantExists(const std::string& name) const {
    return constants_.find(name) != constants_.end();
}

ExprValue RuntimeInfo::getConstant(const std::string& name) const {
    auto it = constants_.find(name);
    if (it == constants_.end()) {
        throw std::invalid_argument(boost::str(boost::format("No such constant '%s'")%name));
    }
    return it->second;
}
}
}