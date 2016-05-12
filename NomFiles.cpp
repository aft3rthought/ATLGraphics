

#include "./NomFiles.h"

#ifdef PLATFORM_WINDOWS
#include <wrl.h>
#include <ppltasks.h>
#endif

namespace atl
{
    application_folder::application_folder(const std::string & in_rootDirectory)
        :
        pm_rootDirectory(in_rootDirectory)
    {}

    file_data::file_data()
        :
        internal_status(file_data_status::waiting_to_load)
    {}

    file_data::file_data(const application_folder & in_application_folder, const char * in_file_path) : file_data()
    {
        load(in_application_folder, in_file_path);
    }

    void file_data::free()
    {
        internal_status = file_data_status::freed;
        internal_data.clear();
        internal_data.shrink_to_fit();
    }

    void file_data::load(const application_folder & in_application_folder, const char * in_file_path)
    {
        if(internal_status == file_data_status::waiting_to_load ||
           internal_status == file_data_status::ready ||
           internal_status == file_data_status::freed)
        {
            internal_status = file_data_status::loading;
#ifdef PLATFORM_WINDOWS
            auto storage_folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
            char16 wide_string_buffer[4098];
            auto wide_string_length = MultiByteToWideChar(CP_UTF8, 0, in_file_path, strlen(in_file_path), wide_string_buffer, 4096);
            wide_string_buffer[wide_string_length] = 0;
            wide_string_buffer[wide_string_length + 1] = 0;

            auto task = Concurrency::create_task(storage_folder->GetFileAsync(::Platform::StringReference(wide_string_buffer, wide_string_length))).then(
                [this](Windows::Storage::StorageFile ^ loaded_file)
            {
                if(loaded_file != nullptr)
                {
                    Concurrency::create_task(Windows::Storage::FileIO::ReadBufferAsync(loaded_file)).then(
                        [this](Windows::Storage::Streams::IBuffer ^ buffer)
                    {
                        internal_data.resize(buffer->Length);
                        // TODO: I saw in some windows documentation, a try catch here. Should probably do that, and set internal_status to error
                        auto data_reader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
                        data_reader->ReadBytes(::Platform::ArrayReference<unsigned char>(internal_data.data(), internal_data.size()));
                        delete data_reader; // As a best practice, explicitly close the dataReader resource as soon as it is no longer needed.
                        internal_status = file_data_status::ready;
                    });
                }
                else
                {
                    internal_status = file_data_status::error;
                }
            });
#endif
        }
    }
}
