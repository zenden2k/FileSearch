#include "Expression.h"

#include "Parser.h"
#include "RuntimeInfo.h"
#include "Utils.h"
#include <boost/format.hpp>
#include <boost/locale/collator.hpp>
#include <boost/algorithm/string.hpp>


namespace FileSearch {
namespace AppCore{

class EqualsVisitor : public boost::static_visitor<bool> {
public:
    template <typename T, typename U>
    bool operator()(const T &, const U &) const
    {
        throw EvaluateException("Cannot compare different types");
    }

    template <typename T>
    bool operator()(const T & lhs, const T & rhs) const
    {
        return lhs == rhs;
    }
};

class NotEqualsVisitor : public boost::static_visitor<bool> {
public:
    template <typename T, typename U>
    bool operator()(const T &, const U &) const
    {
        throw EvaluateException("Cannot compare different types");
    }

    template <typename T>
    bool operator()(const T & lhs, const T & rhs) const
    {
        return lhs != rhs;
    }
};

Expression::Expression(ExprNodePtr root, std::shared_ptr<RuntimeInfo> runtimeInfo) : root_(std::move(root)), runtimeInfo_(runtimeInfo) {

}

Expression::~Expression() {
    delTree(root_);
}

std::shared_ptr<RuntimeInfo> Expression::runtimeInfo() const {
    return runtimeInfo_;
}

ExprValue Expression::evaluate(PropertyMap&& properties) {
    properties_ = std::move(properties);
    result_ = calcTree(root_);
    return result_;
}

ExprValue Expression::calcTree(const ExprNodePtr& tree) {
    switch (tree->operationType) {
    case  OP_VARIABLE:
        return getPropertyValue(boost::get<std::string>(tree->value));
    case OP_VALUE:
        return tree->value;
    case OP_PLUS:
        try {
            return boost::get<ExprInteger>(calcTree(tree->left)) + boost::get<ExprInteger>(calcTree(tree->right));
        } catch (boost::bad_get&) {
            throw EvaluateException("Invalid argument for operator '+'");
        }
    case OP_MINUS:
        try {
            return boost::get<ExprInteger>(calcTree(tree->left)) - boost::get<ExprInteger>(calcTree(tree->right));
        } catch (boost::bad_get&) {
            throw EvaluateException("Invalid argument for operator '-'");
        }
    case OP_MULTIPLY:
        try {
            return boost::get<ExprInteger>(calcTree(tree->left)) * boost::get<ExprInteger>(calcTree(tree->right));
        } catch (boost::bad_get&) {
            throw EvaluateException("Invalid argument for operator '*'");
        }
    case OP_DIVIDE:
        try {
            return boost::get<ExprInteger>(calcTree(tree->left)) * boost::get<ExprInteger>(calcTree(tree->right));
        } catch (boost::bad_get&) {
            throw EvaluateException("Invalid argument for operator '*'");
        }
    case OP_CONTAINS:
        return operationContains(tree);
    case OP_LESS:
    case OP_LESSOREQUAL:
    case OP_GREATER:
    case OP_GREATEROREQUAL:
        return operationCompare(tree);
    case OP_EQUAL:
        return operationEquals(tree);
    case OP_NOTEQUAL:
        return operationNotEquals(tree);
    case OP_UMINUS:
        try {
            return -boost::get<ExprInteger>(calcTree(tree->left));
        } catch (boost::bad_get&) {
            throw EvaluateException("Invalid argument for unary minus operator");
        }
    case OP_NOT:
        return operationIsNot(tree);
        /*try {
            return !boost::get<bool>(calcTree(tree->left));
        } catch (boost::bad_get&) {
            throw EvaluateException("Invalid argument for 'not' operator");
        }*/
    case OP_OR:
    case OP_AND:
        return operationLogical(tree);
    case OP_IS:
        return operationIs(tree);
    case OP_FUNCTIONCALL:
        {
        ExprValue left = calcTree(tree->left);
        return runtimeInfo_->callFunction(tree, left);
        }
    default:
        throw ParserException(str(boost::format("Unknown operation %d") % static_cast<int>(tree->operationType)), 0);
    }
}

bool Expression::operationContains(const ExprNodePtr& node) {
    std::string left, right;
    try {
        left = boost::get<std::string>(calcTree(node->left));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid argument for operation 'contains'", left);
    }
    try {
        right = boost::get<std::string>(calcTree(node->right));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid argument for operation 'contains'", right);
    }
    namespace bl = boost::locale;
    bl::comparator<char, bl::collator_base::secondary> cmpr;
    std::wstring leftW = Utils::Utf8ToWideString(left);
    std::wstring rightW = Utils::Utf8ToWideString(right);

    auto range = boost::algorithm::ifind_first(left, right); // case insensitive search
    return Utils::FindSubstrCaseInsensitive(leftW, rightW) != -1;
}

bool Expression::operationCompare(const ExprNodePtr& node) {
    ExprInteger left, right;
    try {
        left = boost::get<ExprInteger>(calcTree(node->left));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid argument for comparison operator", left);
    }
    try {
        right = boost::get<ExprInteger>(calcTree(node->right));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid argument for comparison operator", right);
    }

    switch (node->operationType) {
        case OP_LESS:
            return left < right;
        case OP_LESSOREQUAL:
            return left <= right;
        case OP_GREATER:
            return left > right;
        case OP_GREATEROREQUAL:
            return left >= right;
        default:; // just for Resharper to be happy
    }
    assert(false);
    return false;
}

bool Expression::operationEquals(const ExprNodePtr& node) {
    auto left = calcTree(node->left);
    auto right = calcTree(node->right);
    return boost::apply_visitor(EqualsVisitor(), left, right);
}

bool Expression::operationNotEquals(const ExprNodePtr& node) {
    auto left = calcTree(node->left);
    auto right = calcTree(node->right);
    return boost::apply_visitor(NotEqualsVisitor(), left, right);
}

bool Expression::operationLogical(const ExprNodePtr& node) {
    bool left, right;
    try {
        left = boost::get<bool>(calcTree(node->left));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid argument for logical operator", left);
    }

    if (node->operationType == OP_AND && !left) {
        return false;// Do not calculate right side 
    } else if (node->operationType == OP_OR && left) {
        return true;// Do not calculate right side 
    }
    try { 
        right = boost::get<bool>(calcTree(node->right));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid argument for logical operator", right);
    }

    switch (node->operationType) {
        case OP_AND:
            return left && right;
        case OP_OR:
            return left || right;
        default:; // just for Resharper to be happy
    }
    assert(false);
    return false;
}

bool Expression::operationIs(const ExprNodePtr& node) {
    FileAttributeSet left;
    FileAttribute right;
    try {
        left = boost::get<FileAttributeSet>(calcTree(node->left));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid left argument for operator 'is'", left);
    }
    try {
        right = boost::get<FileAttribute>(calcTree(node->right));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid right argument for operator 'is'", right);
    }
    return left.find(right) != left.end();
}

bool Expression::operationIsNot(const ExprNodePtr& node) {
    FileAttributeSet left;
    FileAttribute right;
    try {
        left = boost::get<FileAttributeSet>(calcTree(node->left));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid left argument for operator 'not'", left);
    }
    try {
        right = boost::get<FileAttribute>(calcTree(node->right));
    } catch (boost::bad_get&) {
        throw InvalidFunctionArgument("Invalid right argument for operator 'not'", right);
    }
    return left.find(right) == left.end();
}

ExprValue Expression::getPropertyValue(const std::string& propertyName) {
    auto callback = properties_.find(propertyName);
    if (callback == properties_.end()) {
        throw EvaluateException(str(boost::format("Property callback not set for property '%s'") % propertyName));
    }
    return callback->second();
}


void Expression::delTree(ExprNodePtr tree) {
    if (tree == nullptr) {
        return;
    }

    delTree(tree->left);
    delTree(tree->right);

    delete tree;
}

}
}