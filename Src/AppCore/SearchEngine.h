#ifndef APPCORE_SEARCHENGINE_H
#define APPCORE_SEARCHENGINE_H

#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include "ParserTypes.h"

namespace FileSearch {
namespace AppCore {

class FoundFile {
public:
    FoundFile(const boost::filesystem::path& path, boost::filesystem::file_status status);

    std::string path() const {
        return path_.string();
    }
    std::string filename() const {
        return path_.filename().string();
    }
    int64_t size() const;

    FileAttributeSet attributes() const {
        return attributes_;
    }

protected:
    const int64_t SIZE_NOT_SET = -1;
    const int64_t SIZE_UNKNOWN = -2;
    mutable int64_t size_;
    boost::filesystem::path path_;
    FileAttributeSet attributes_;
};

class Expression;
class SearchEngine : public boost::noncopyable {
public:
    SearchEngine();
    virtual ~SearchEngine();
    int searchFiles(const std::string& startDirectory, std::shared_ptr<Expression> expr, std::atomic<bool>& stopFlag, std::function<void(const FoundFile&)>&& callback);
};

}
}
#endif