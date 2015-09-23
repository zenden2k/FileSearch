#include "SearchEngine.h"

#include <boost/filesystem.hpp>
#include "Expression.h"
#include "Parser.h"
#include "Utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace FileSearch {
namespace AppCore{

SearchEngine::SearchEngine() {
}

SearchEngine::~SearchEngine() {
}

int SearchEngine::searchFiles(const std::string& startDirectory, std::shared_ptr<Expression> expr, std::atomic<bool>& stopFlag, std::function<void(const FoundFile&)>&& callback) {
    Parser parser;
    namespace fs = boost::filesystem;
    int foundFileCount = 0;
    try {
        fs::recursive_directory_iterator dir(startDirectory), end;

        while (dir != end) {
            FoundFile file(dir->path(), dir->status());

            bool matches = false;
            PropertyMap&& properties = {
                { "name", [&]() { return file.filename(); } },
                { "size", [&]() { return static_cast<ExprInteger>(file.size()); } },
                { "attribute", [&]() -> FileAttributeSet { return file.attributes(); } }
                // Свойства файла вычисляются ленивым образом
                // Можно добавить свойство, например, хэш md5 файла, и он не будет вычисляться, если его нет в выражении, 
                // или если он в правой части сокращенного логического оператора ('and' или 'or')
            };

            try {
                matches = boost::get<bool>(expr->evaluate(std::move(properties)));

            } catch (EvaluateException&) {
                throw;
            } catch (boost::bad_get&) {
                throw EvaluateException("Expression result is not of boolean type");
            }

            if (matches) {
                callback(file);
            }
            foundFileCount++;

            if (stopFlag) {
                break;
            }

            try {
                ++dir;
            } catch (const boost::filesystem::filesystem_error&) {
                // Could be thrown access denied exception, just skip it
            }
        }
    } catch (const boost::filesystem::filesystem_error& ex) {
        throw std::exception((std::string("Cannot iterate over starting directory: ") + ex.what()).c_str());
    }
    return foundFileCount;
}

FoundFile::FoundFile(const boost::filesystem::path& path, boost::filesystem::file_status status) 
    : path_(path), size_(SIZE_NOT_SET) {
        if (is_directory(status)) {
            attributes_.insert(FileAttribute::Directory);
        }
        if (is_regular_file(status)) {
            attributes_.insert(FileAttribute::File);
        }
        if (is_symlink(status)) {
            attributes_.insert(FileAttribute::Symlink);
        }
#ifdef _WIN32
        DWORD attrs = ::GetFileAttributes(U2W(path.string()));
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            if (attrs & FILE_ATTRIBUTE_ARCHIVE) {
                attributes_.insert(FileAttribute::Archive);
            }
            if (attrs & FILE_ATTRIBUTE_HIDDEN) {
                attributes_.insert(FileAttribute::Hidden);
            }
            if (attrs & FILE_ATTRIBUTE_COMPRESSED) {
                attributes_.insert(FileAttribute::Compressed);
            }
            if (attrs & FILE_ATTRIBUTE_ENCRYPTED) {
                attributes_.insert(FileAttribute::Encrypted);
            }
            if (attrs & FILE_ATTRIBUTE_ENCRYPTED) {
                attributes_.insert(FileAttribute::Encrypted);
            }
            if (attrs & FILE_ATTRIBUTE_SYSTEM) {
                attributes_.insert(FileAttribute::System);
            }
            if (attrs & FILE_ATTRIBUTE_SPARSE_FILE) {
                attributes_.insert(FileAttribute::Sparse);
            }
        }
#else
        std::string name = path.filename().string();
        if (name != ".." && name != "."  && name[0] == '.') {
            attributes_.insert(FileAttribute::Hidden);
        }
        // TODO
#endif
    
}

int64_t FoundFile::size() const {
    if (size_ == SIZE_NOT_SET) {
        try {
            size_ = boost::filesystem::file_size(path_);
        } catch (std::exception&) {
            // Could be thrown exception like 'request is not supported'
            size_ = SIZE_UNKNOWN;
        }
    }
    // Return 0 for directories
    return size_ == SIZE_UNKNOWN ? 0 : size_;
}

}
}