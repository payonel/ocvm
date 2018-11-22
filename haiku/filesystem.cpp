#include "filesystem.h"

#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>

#include <dirent.h>

namespace haiku
{
    namespace filesystem
    {
        void run_cmd(const std::string& command, std::error_code& ec)
        {
            int result = ::system(command.c_str());
            ec.assign(result, std::system_category());
            if (result != 0)
            {
                ec.assign(1, std::system_category());
            }
        }

        void copy(const std::string& from, const std::string& to, copy_options options, std::error_code& ec) noexcept
        {
            std::string command = "cp  ";
            command += "\"";
            command += from;
            command += "\" \"";
            command += to;
            command += "\"";
            run_cmd(command, ec);
        }

        void rename(const std::string& from, const std::string& to, std::error_code& ec) noexcept
        {
            std::string command = "mv ";
            command += "\"";
            command += from;
            command += "\" \"";
            command += to;
            command += "\"";
            run_cmd(command, ec);
        }

        void create_directories(const std::string& path, std::error_code& ec) noexcept
        {
            std::string command = "mkdir -p \"";
            command += path;
            command += "\"";
            run_cmd(command, ec);
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
            file_time_type ft(std::chrono::seconds(0));
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
            run_cmd(cmd, ec);
            return ec.value() == 0;
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

