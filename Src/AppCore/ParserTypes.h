#ifndef APPCORE_PARSERTYPES_H
#define APPCORE_PARSERTYPES_H

#include <string>
#include <set>
#include <array>
#include <stdint.h>
#include <boost/variant.hpp>

namespace FileSearch {
namespace AppCore{

typedef int64_t ExprInteger;

enum class FileAttribute {
    Hidden, Archive, System, Readonly, Compressed, Encrypted, Sparse, Directory, File, Symlink, LAST_VALUE
};

inline std::array<std::string, static_cast<size_t>(FileAttribute::LAST_VALUE)> GetFileAttributeStrings(){
    // list of constant names mapped to enum values
    return{ 
        "hidden",
        "archive",
        "system",
        "readonly",
        "compressed",
        "encrypted",
        "sparse",
        "directory",
        "file",
        "symlink"
    };
};

typedef std::set<FileAttribute> FileAttributeSet;
typedef boost::variant<ExprInteger, std::string, FileAttribute, FileAttributeSet, bool> ExprValue;

enum OperationType {
    OP_VALUE,
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_CONTAINS,
    OP_AND,
    OP_OR,
    OP_IS,
    OP_UMINUS,
    OP_FUNCTIONCALL,
    OP_VARIABLE,
    OP_EQUAL,
    OP_NOTEQUAL,
    OP_LESS,
    OP_LESSOREQUAL,
    OP_GREATER,
    OP_GREATEROREQUAL,
    OP_NOT
};

struct ExprNode;

typedef ExprNode* ExprNodePtr;

struct ExprNode {
    OperationType operationType;
    ExprValue value;
    ExprNodePtr left;
    ExprNodePtr right;

    ExprNode(OperationType type, ExprValue _value, ExprNodePtr&& _left = nullptr, ExprNodePtr&& _right = nullptr);
};


}
}
#endif