#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <queue>

namespace haiku
{
    namespace filesystem
    {
        enum class copy_options : unsigned short {
            none = 0,
            skip_existing = 1, overwrite_existing = 2, update_existing = 4,
            recursive = 8,
            copy_symlinks = 16, skip_symlinks = 32,
            directories_only = 64, create_symlinks = 128, create_hard_links = 256
        };

        enum class file_type : signed char {
            none = 0, not_found = -1, regular = 1, directory = 2, symlink = 3,
            block = 4, character = 5, fifo = 6, socket = 7, unknown = 8
        };

        class file_status
        {
        public:
            file_status(file_type ftype) : _type(ftype) {}
            file_type type() const;
        private:
            file_type _type;
        };

        using file_time_type = std::chrono::system_clock::time_point;

        void copy(const std::string& from, const std::string& to, copy_options options, std::error_code&) noexcept;
        void rename(const std::string& from, const std::string& to, std::error_code&) noexcept;
        void create_directories(const std::string& path, std::error_code& ec) noexcept;
        bool exists(const std::string& path, std::error_code& ec) noexcept;
        bool is_directory(const std::string& path, std::error_code& ec) noexcept;
        size_t file_size(const std::string& path, std::error_code& ec) noexcept;
        file_time_type last_write_time(const std::string& path, std::error_code& ec) noexcept;
        file_status symlink_status(const std::string&, std::error_code&) noexcept;
        bool remove(const std::string&, std::error_code&) noexcept;
        std::string current_path(std::error_code& ec) noexcept;
        
        class directory_iterator
        {
        public:
            class directory_entry
            {
            public:
                directory_entry(const std::string& path) : _path(path) { }
                std::string path() const { return _path; }
            private:
                std::string _path;
            };

            directory_iterator() = default;
            explicit directory_iterator(const std::string& path, std::error_code& ec) noexcept;
            const directory_entry& operator*() const;
            const directory_entry* operator->() const { return &**this; }
            bool operator!=(const directory_iterator&) const;
            directory_iterator& operator++();

            directory_iterator begin() noexcept;
            directory_iterator end() noexcept;
        private:
            std::queue<directory_entry> _entries;
            std::string _root;
        };
    };
};
