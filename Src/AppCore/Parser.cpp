#include "parser.h"

#include <boost/format.hpp>
#include "RuntimeInfo.h"

namespace FileSearch {
namespace AppCore{

enum ErrorType {
    etUnknownKeyword,
    etUnknownSymbol,
    etUnexpectedEndOfExpression,
    etEndOfExpressionExpexted,
    etOpeningBracketExpected,
    etClosingBracketExpected,
    etUnexpected,
    etMaxRecursionExceed
};


class ParserPrivate {
public:
    enum TokenType {
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_MULTIPLY,
        TOKEN_DIVIDE,
        TOKEN_AND,
        TOKEN_OR,
        TOKEN_L_BRACKET,
        TOKEN_R_BRACKET,
        TOKEN_NUMBER,
        TOKEN_SYMBOL, // variable or function
        TOKEN_EQUAL,
        TOKEN_NOT_EQUAL,
        TOKEN_CONTAINS,
        TOKEN_NOT,
        TOKEN_IS,
        TOKEN_LESSOREQUAL,
        TOKEN_LESS,
        TOKEN_GREATER,
        TOKEN_GREATEROREQUAL,
        TOKEN_LITERAL,
        TOKEN_END
    } tokenType;

    enum { MAX_RECURSION_DEPTH = 100 };

    ParserPrivate(): runtimeInfo_(new RuntimeInfo()), pos(0), expr(nullptr) { 
        
    }
    ~ParserPrivate() {
    }

    ExprNodePtr createNode(OperationType type, ExprNodePtr _left = nullptr, ExprNodePtr _right = nullptr) {
        return _createNode(type, ExprValue(), std::move(_left), std::move(_right));
    }

    ExprNodePtr createValueNode(OperationType type, ExprValue _value ) {
        return _createNode(type, _value, nullptr, nullptr);
    }

    ExprNodePtr _createNode(OperationType type, ExprValue _value, ExprNodePtr&& _left, ExprNodePtr&& _right) {
        ExprNodePtr pNode(new ExprNode(type, _value, std::move(_left), std::move(_right)));
        history.push_back(pNode);
        return pNode;
    }

    void clearHistory() {
        for (auto& node: history) {
            delete node;
        }
        history.clear();
    }

    void raiseError(ErrorType errNum) {
        static const char* const errs[8] = {
            nullptr,
            nullptr,
            "Unexpected end of expression",
            "End of expression expected",
            "'(' expected",
            "')' expected",
            nullptr,
            "Max recursion exceed"
        };
        std::string msg;

        int len = strlen(curToken);

        if (*curToken == '\0') {
            strcpy(curToken, "EOL");
        }

        switch (errNum) {
            case etUnknownKeyword:
                msg = str(boost::format("Unknown keyword: '%s'") % curToken);
                break;

            case etUnknownSymbol:
                msg = str(boost::format("Unknown symbol: '%s'") % curToken);
                break;

            case etUnexpected:
                msg = str(boost::format("Unexpected '%s'") % curToken);
                break;
            default:; // just for resharper to be happy
                msg = errs[errNum];
        }

        throw ParserException(msg, pos - len);
    }

    bool getNextToken(bool allowRaiseError = true) {
        *curToken = '\0';

        while (expr[pos] == ' ') {
            pos++;
        }

        if (expr[pos] == '\0') {
            curToken[0] = '\0';
            tokenType = TOKEN_END;
            return true;
        } else if (IsDelim()) {
            curToken[0] = expr[pos++];
            curToken[1] = '\0';
            switch (*curToken) {
            case '+': tokenType = TOKEN_PLUS;
                return true;
            case '-': tokenType = TOKEN_MINUS;
                return true;
            case '*': tokenType = TOKEN_MULTIPLY;
                return true;
            case '/': tokenType = TOKEN_DIVIDE;
                return true;
            case '(': tokenType = TOKEN_L_BRACKET;
                return true;
            case ')': tokenType = TOKEN_R_BRACKET;
                return true;
            case '!':
                if (expr[pos] == '=') {
                    tokenType = TOKEN_NOT_EQUAL;
                    pos++;
                    return true;
                } else {
                    raiseError(etUnexpected);
                }
            case '=':
                if (expr[pos] == '=') {
                    tokenType = TOKEN_EQUAL;
                    pos++;
                    return true;
                } else {
                    raiseError(etUnexpected);
                }
                break;

            case '<':
                if (expr[pos] == '=') {
                    tokenType = TOKEN_LESSOREQUAL;
                    pos++;
                } else {
                    tokenType = TOKEN_LESS;
                }

                return true;
            case '>':
                if (expr[pos] == '=') {
                    tokenType = TOKEN_GREATEROREQUAL;
                    pos++;
                } else {
                    tokenType = TOKEN_GREATER;
                }
                return true;
            case '"':
                int i = 0;
                if (expr[pos] == '\0') {
                    raiseError(etUnexpectedEndOfExpression);
                }
                while (expr[pos] != '"') {
                    if (expr[pos] == '\0') {
                        raiseError(etUnexpectedEndOfExpression);
                    }
                    if (expr[pos] == '\\' && expr[pos + 1] == '"') {
                        pos++;
                    }

                    curToken[i++] = expr[pos++];
                }
                curToken[i++] = '\0';
                pos++;
                tokenType = TOKEN_LITERAL;
                return true;
            }
        } 
        else if (isDigit()) {
            int i = 0;
            while (isDigit()) {
                curToken[i++] = expr[pos++];
            }
            if (expr[pos] != '\0' && expr[pos] != ' ' && !IsDelim(expr[pos])) {
                curToken[i++] = expr[pos];
                curToken[i] = '\0';
                raiseError(etUnexpected);
            }
            /* Floating point numbers
            if (isPoint()) {
                curToken[i++] = expr[pos++];
                while (isDigit()) {
                    curToken[i++] = expr[pos++];
                }
            }*/
            curToken[i] = '\0';
            tokenType = TOKEN_NUMBER;
            return true;
        } else if (isLetter()) {
            int i = 0;
            while (isAlNum()) {
                curToken[i++] = expr[pos++];
            }
            curToken[i] = '\0';

            int len = strlen(curToken);
            for (i = 0; i < len; i++) {
                if (curToken[i] >= 'A' && curToken[i] <= 'Z') {
                    curToken[i] += 'a' - 'A';
                }
            }

            if (!strcmp(curToken, "contains")) {
                tokenType = TOKEN_CONTAINS;
                return true;
            } if (!strcmp(curToken, "is")) {
                tokenType = TOKEN_IS;
                return true;
            } if (!strcmp(curToken, "not")) {
                tokenType = TOKEN_NOT;
                return true;
            } if (!strcmp(curToken, "or")) {
                tokenType = TOKEN_OR;
                return true;
            }  if (!strcmp(curToken, "and")) {
                tokenType = TOKEN_AND;
                return true;
            } else {
                tokenType = TOKEN_SYMBOL;
                return true;
            }
        } else {
            curToken[0] = expr[pos++];
            curToken[1] = '\0';
            if (allowRaiseError) {
                raiseError(etUnknownSymbol);
            }
        }

        return false;
    }

    /*
    expr0 содержит наименее приоритетные операторы
    ...
    expr7 - наиболее приоритетные.
    */
    ExprNodePtr expr0(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = expr1(depth+1);

        for (;;) {
            if (tokenType == TOKEN_OR) {
                getNextToken();
                temp = createNode(OP_OR, temp, expr1(depth + 1));
            } else {
                break;

            }
        }

        return temp;
    }

    ExprNodePtr expr1(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = expr2(depth + 1);

        for (;;) {
            if (tokenType == TOKEN_AND) {
                getNextToken();
                temp = createNode(OP_AND, temp, expr2(depth + 1));
            } else {
                break;
            }
        }
       
        return temp;
    }

    ExprNodePtr expr2(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = expr3(depth + 1);

        for (;;) {
            if (tokenType == TOKEN_CONTAINS) {
                getNextToken();
                temp = createNode(OP_CONTAINS, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_IS) {
                getNextToken();
                temp = createNode(OP_IS, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_NOT) {
                getNextToken();
                temp = createNode(OP_NOT, temp, expr3(depth + 1));
            }else if (tokenType == TOKEN_LESS) {
                getNextToken();
                temp = createNode(OP_LESS, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_LESSOREQUAL) {
                getNextToken();
                temp = createNode(OP_LESSOREQUAL, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_GREATER) {
                getNextToken();
                temp = createNode(OP_GREATER, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_GREATEROREQUAL) {
                getNextToken();
                temp = createNode(OP_GREATEROREQUAL, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_EQUAL) {
                getNextToken();
                temp = createNode(OP_EQUAL, temp, expr3(depth + 1));
            } else if (tokenType == TOKEN_NOT_EQUAL) {
                getNextToken();
                temp = createNode(OP_NOTEQUAL, temp, expr3(depth + 1));
            }  else {
                break;
            }

        }
        return temp;
    }

    ExprNodePtr expr3(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = expr4(depth + 1);

        for (;;) {
            if (tokenType == TOKEN_PLUS) {
                getNextToken();
                temp = createNode(OP_PLUS, temp, expr4(depth + 1));
            } else if (tokenType == TOKEN_MINUS) {
                getNextToken();
                temp = createNode(OP_MINUS, temp, expr4(depth + 1));
            } else {
                break;
            }
        }

        return temp;
    }

    ExprNodePtr expr4(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = expr5(depth + 1);
        for (;;) {
            if (tokenType == TOKEN_MULTIPLY) {
                getNextToken();
                temp = createNode(OP_MULTIPLY, temp, expr5(depth + 1));
            } else if (tokenType == TOKEN_DIVIDE) {
                getNextToken();
                temp = createNode(OP_DIVIDE, temp, expr5(depth + 1));
            } else {
                break;
            }
        }

        return temp;
    }

    ExprNodePtr expr5(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = nullptr;
        if (tokenType == TOKEN_PLUS) {
            getNextToken();
            temp = expr6(depth + 1);
        } else if (tokenType == TOKEN_MINUS) {
            getNextToken();
            temp = createNode(OP_UMINUS, expr6(depth + 1));
        }  else {
            temp = expr6(depth + 1);
        }
        return temp;
    }


    ExprNodePtr expr6(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = nullptr;

        if (tokenType == TOKEN_SYMBOL) {
            std::string symbolName = curToken;
            int symbolPos = pos - strlen(curToken);
            if (runtimeInfo_->constantExists(symbolName)) {
                temp = createValueNode(OP_VALUE, runtimeInfo_->getConstant(symbolName));
                getNextToken();
            } else if (runtimeInfo_->functionExists(symbolName)) {
                getNextToken();
                if (tokenType == TOKEN_L_BRACKET) {
                    // function call
                    temp = createValueNode(OP_FUNCTIONCALL, symbolName);
                    getNextToken();
                    temp->left = expr0(depth + 1);
                    if (tokenType != TOKEN_R_BRACKET) {
                        raiseError(etClosingBracketExpected);
                    }
                    getNextToken();
                } else {
                    raiseError(etOpeningBracketExpected);
                }
            } else {
                if (!runtimeInfo_->propertyExists(symbolName)) {
                    throw ParserException(str(boost::format("Undefined property '%s'") % symbolName), symbolPos);
                }
                temp = createValueNode(OP_VARIABLE, symbolName);
                getNextToken();
            }
        } else {
            temp = expr7(depth + 1);
        }

        return temp;
    }


    ExprNodePtr expr7(int depth) {
        if (depth > MAX_RECURSION_DEPTH) {
            raiseError(etMaxRecursionExceed);
        }
        ExprNodePtr temp = nullptr;

        switch (tokenType) {
            case TOKEN_NUMBER:
                temp = createValueNode(OP_VALUE, static_cast<ExprInteger>(std::stoi(curToken)));
                getNextToken();
                break;
            case TOKEN_LITERAL:
                temp = createValueNode(OP_VALUE, static_cast<std::string>(curToken));
                getNextToken();
                break;

            case TOKEN_L_BRACKET:
                getNextToken();
                temp = expr0(depth + 1);
                if (tokenType != TOKEN_R_BRACKET) {
                    raiseError(etClosingBracketExpected);
                }
                getNextToken();
                break;

            default:
                raiseError(etUnexpected);
        }

        return temp;
    }

    bool IsDelim(char c) {
        return (strchr("!+-*/%()[]\"\"<>=", c) != nullptr);
    }
    bool IsDelim(void) {
        return IsDelim(expr[pos]);
    }

    bool isLetter(void) {
        return ((expr[pos] >= 'a' && expr[pos] <= 'z') ||
            (expr[pos] >= 'A' && expr[pos] <= 'Z'));
    }

    bool isAlNum(void) {
        return isalnum(expr[pos])!=0;
    }

    bool isDigit(void) {
        return (expr[pos] >= '0' && expr[pos] <= '9');
    }

    bool isPoint(void) {
        return (expr[pos] == '.');
    }
   
    std::shared_ptr<Expression> compile(const std::string& _expr) {
        try {
            expr = new char[_expr.length() + 1];
            pos = 0;

            strcpy(expr, _expr.c_str());
            *curToken = '\0';

            history.clear();

            getNextToken();
            if (tokenType == TOKEN_END) {
                raiseError(etUnexpectedEndOfExpression);
            }
            ExprNodePtr root = expr0(0);
            if (tokenType != TOKEN_END) {
                raiseError(etEndOfExpressionExpexted);
            }

            history.clear();
            delete expr;
            expr = nullptr;

            return std::make_shared<Expression>(root, runtimeInfo_);
        } catch (...) {
            delete expr;
            expr = nullptr;
            clearHistory();
            throw;
        }
    }

private:
    enum { MAX_TOKEN_LEN = 80, MAX_EXPR_LEN = 255 };
    std::shared_ptr<RuntimeInfo> runtimeInfo_;
    int pos;
    ExprValue result;
    char* expr;
    std::vector<ExprNode *> history;
    char curToken[MAX_TOKEN_LEN];
};

ExprNode::ExprNode(OperationType type, ExprValue _value, ExprNodePtr&& _left, ExprNodePtr&& _right) : value(_value){
    operationType = type;
    left = std::move(_left);
    right = std::move(_right);
}

InvalidFunctionArgument::InvalidFunctionArgument(const std::string& msg, ExprValue value) : ParserException(msg), value_(value){}
InvalidFunctionArgument::InvalidFunctionArgument(const InvalidFunctionArgument& ex) : ParserException(ex){}

Parser::Parser() : d_ptr(new ParserPrivate()) {    
}

Parser::~Parser() {
}

std::shared_ptr<Expression> Parser::compile(const std::string& expr) {
    return d_ptr->compile(expr);
}

}
}