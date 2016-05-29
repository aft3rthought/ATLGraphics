

#pragma once

#include <string>
#include <vector>
#include <functional>

namespace atl
{
    struct application_folder
    {
    private:
        std::string pm_rootDirectory;

    public:
        const std::string & rootDirectory() const { return pm_rootDirectory; }

        application_folder(const std::string & in_rootDirectory);
    };

    enum class file_data_status
    {
        waiting_to_load,
        loading,
        ready,
        freed,
        error
    };

    struct file_data
    {
        file_data();
        file_data(const application_folder & in_application_folder, const char * in_file_path);
        void load(const application_folder & in_application_folder, const char * in_file_path);
        void free();

        file_data_status status() const { return internal_status; }
        const std::vector<unsigned char> & data() const { return internal_data; }

    private:
        file_data_status internal_status;
        std::vector<unsigned char> internal_data;
    };
}
