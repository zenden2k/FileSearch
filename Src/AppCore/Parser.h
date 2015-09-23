#ifndef TOKEN_H
#define TOKEN_H

#pragma once

#include <string>
#include <memory>
#include <boost/noncopyable.hpp>
#include "ParserTypes.h"
#include "Expression.h"

namespace FileSearch {
namespace AppCore{

class ParserPrivate;

/**
    Простой парсер рекурсивного спуска
    Хотел сделать на Boost.Spirit, но тот наотрез отказался компилироваться в VS2013
    */

class Parser: public boost::noncopyable {
public:
    Parser();
    virtual ~Parser();
    std::shared_ptr<Expression> compile(const std::string& expr);
private:
    std::unique_ptr<ParserPrivate> d_ptr;
};

class ParserException : public std::runtime_error {
public:
    ParserException(const std::string& msg, int pos = -1) : std::runtime_error(msg), pos_(pos) {}
    ParserException(const ParserException& ex) : std::runtime_error(ex), pos_(ex.pos_) {}
    int pos() {
        return pos_;
    }
protected:
    int pos_;
};

class InvalidFunctionArgument : public ParserException {
public:
    InvalidFunctionArgument(const std::string& msg, ExprValue value);
    InvalidFunctionArgument(const InvalidFunctionArgument& ex);
private:
    ExprValue value_;
};

}
}
#endif