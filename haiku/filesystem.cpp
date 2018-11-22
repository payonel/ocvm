#include "filesystem.h"

#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace haiku
{
    namespace filesystem
    {
        std::string name(const std::string& path)
        {
            if (!path.empty())
            {
                auto last_slash = path.find_last_of("/");
                if (last_slash != std::string::npos)
                {
                    std::string after_slash = path.substr(last_slash + 1);
                    if (after_slash.empty())
                    {
                        // path ended in slash, such as /a/b/c/
                        return haiku::filesystem::name(path.substr(0, last_slash));
                    }
                    // part after last slash not empty, and thus is name
                    return after_slash;
                }
                // no slash, path is whole name
            }

            return path;
        }

        std::string ensure_slash(const std::string& path)
        {
            if (path.empty() || path.at(path.size() - 1) == '/')
                return path;
            return path + "/";
        }

        bool copy_file(const std::string& from, const std::string& to, std::error_code& ec)
        {
            ec.assign(1, std::system_category());
            std::ifstream in(from);
            if (!in)
                return false;

            std::ofstream out(to);
            if (!out)
                return false;

            out << in.rdbuf();
            in.close();
            out.close();

            ec.assign(0, std::system_category());
            return true;
        }

        void copy(const std::string& from, const std::string& to, copy_options options, std::error_code& ec) noexcept
        {
            if (!exists(from, ec))
                return;

            bool from_is_dir = haiku::filesystem::is_directory(from, ec);

            std::string to_target = to;
            if (exists(to, ec))
            {
                if (is_directory(to, ec))
                {
                    to_target = ensure_slash(to) + name(from);
                }
                else
                {
                    if (from_is_dir)
                    {
                        ec.assign(1, std::system_category());
                        return; // cannot copy dir to file
                    }
                }
            }

            if (from_is_dir)
            {
                haiku::filesystem::create_directories(to_target, ec);
                if (ec.value() != 0)
                    return;

                for (const auto& ele : directory_iterator(from, ec))
                {
                    std::string from_file = ensure_slash(from) + ele.path();
                    std::string to_file_target = ensure_slash(to_target) + ele.path();
                    if (!copy_file(from_file, to_file_target, ec))
                        return;
                }
            }
            else
            {
                copy_file(from, to_target, ec);
            }
        }

        void rename(const std::string& from, const std::string& to, std::error_code& ec) noexcept
        {
            ec.assign(1, std::system_category());
            if (haiku::filesystem::exists(to, ec) || !haiku::filesystem::exists(from, ec))
                return;

            haiku::filesystem::copy(from, to, copy_options::skip_existing, ec);
            if (ec.value() != 0)
                return;

            if (!haiku::filesystem::remove(from, ec))
                ec.assign(1, std::system_category());
        }

        std::vector<std::string> path_segments(const std::string& path)
        {
            std::vector<std::string> vec;

            size_t last = 0;
            while (last < path.size())
            {
                size_t next = path.find("/", last);
                size_t len = (next == std::string::npos ? path.size() : next) - last;
                std::string part = path.substr(last, len);
                vec.push_back(part);
                last += len + 1;
            }

            return vec;
        }

        void create_directories(const std::string& path, std::error_code& ec) noexcept
        {
            auto segments = path_segments(path);
            std::string next;

            for (const auto& part : segments)
            {
                next += part + "/";
                DIR* dir = ::opendir(next.c_str());
                if (dir)
                {
                    ::closedir(dir);
                }
                else
                {
                    if (::mkdir(next.c_str(), 0755) == -1)
                    {
                        ec.assign(1, std::system_category());
                        return;
                    }
                }
            }
            ec.assign(0, std::system_category());
        }

        bool exists(const std::string& path, std::error_code& ec) noexcept
        {
            std::ifstream ifs;
            ifs.open(path);
            return static_cast<bool>(ifs) || is_directory(path, ec);
        }

        bool is_directory(const std::string& path, std::error_code& ec) noexcept
        {
            auto* dir = ::opendir(path.c_str());
            if (dir)
            {
                ::closedir(dir);
                return true;
            }
            ec.assign(1, std::system_category());
            return false;
        }

        size_t file_size(const std::string& path, std::error_code& ec) noexcept
        {
            ec.assign(1, std::system_category());
            std::ifstream ifs;
            ifs.open(path);
            if (ifs)
            {
                ifs.seekg(0, std::ios_base::end);
                size_t pos = static_cast<size_t>(ifs.tellg());
                ifs.close();
                ec.assign(0, std::system_category());
                return pos;
            }

            return 0;
        }

        file_time_type last_write_time(const std::string& path, std::error_code& ec) noexcept
        {
            struct ::stat attr;
            ::stat(path.c_str(), &attr);
            file_time_type ft(std::chrono::seconds(attr.st_mtime));
            return ft;
        }

        file_status symlink_status(const std::string& path, std::error_code& ec) noexcept
        {
            if (is_directory(path, ec))
            {
                return file_status(file_type::directory);
            }
            else if (exists(path, ec))
            {
                return file_status(file_type::regular);
            }
            return file_status(file_type::none);
        }

        bool remove(const std::string& path, std::error_code& ec) noexcept
        {
            std::string cmd = "rm -rf ";
            cmd += path;

            int result = ::system(cmd.c_str());
            ec.assign(result, std::system_category());
            if (result != 0)
            {
                ec.assign(1, std::system_category());
                return false;
            }

            return true;
        }

        std::string current_path(std::error_code& ec) noexcept
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != nullptr)
                return cwd;
            else
                return "";
        }

        directory_iterator::directory_iterator(const std::string& path, std::error_code& ec) noexcept
        {
            _root = path;
            DIR* dir = opendir(path.c_str());
            if (dir)
            {
                dirent* ent = nullptr;
                while ((ent = readdir(dir)))
                {
                    std::string name = ent->d_name;
                    if (name != "." && name != "..")
                        _entries.emplace(name);
                }
                closedir(dir);
            }
        }

        const directory_iterator::directory_entry& directory_iterator::operator*() const
        {
            return _entries.front();
        }

        bool directory_iterator::operator!=(const directory_iterator& rhs) const
        {
            if (_entries.size() != rhs._entries.size())
                return true;

            if (_entries.size() == 0)
                return false; // both are empty, so they must be equal

            if (_root != rhs._root)
                return true;

            return _entries.front().path() == rhs._entries.front().path();
        }

        directory_iterator& directory_iterator::operator++()
        {
            _entries.pop();
            return *this;
        }

        directory_iterator directory_iterator::begin() noexcept
        {
            return *this;
        }

        directory_iterator directory_iterator::end() noexcept
        {
            return directory_iterator();
        }

        file_type file_status::type() const
        {
            return _type;
        }
    };
};

