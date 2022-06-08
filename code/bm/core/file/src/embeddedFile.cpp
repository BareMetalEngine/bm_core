/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "embeddedFile.h"
#include "bm/core/containers/include/hashSet.h"
#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

//--

namespace prv
{
    class LocalEmbeddedFileRegistry : public IEmbeddedFilesRegistry
    {
    public:
        LocalEmbeddedFileRegistry() {};
        virtual ~LocalEmbeddedFileRegistry() {};

        virtual const EmbeddedFile* findFile(const StringView name, EmbeddedFileType type) const override final
        {
            auto lock = CreateLock(m_lock);

            const auto storageIndex = (int)type;
            DEBUG_CHECK_RETURN_V(storageIndex >= 0 && storageIndex < MAX_FILE_TYPES, nullptr);

            const auto& storage = m_storage[storageIndex];

            const EmbeddedFile* ret = nullptr;
            storage.fileMap.find(name, ret);

            return ret;
        }

        virtual FileReaderPtr openFile(const StringView name) const override final
        {
            return nullptr;
        }

        virtual void iterateFiles(EmbeddedFileType type, const std::function<void(const EmbeddedFile*)>& func) const override final
        {
            auto lock = CreateLock(m_lock);

            const auto storageIndex = (int)type;
            DEBUG_CHECK_RETURN(storageIndex >= 0 && storageIndex < MAX_FILE_TYPES);

            const auto& storage = m_storage[storageIndex];
            for (const auto* file : storage.fileMap.values())
                func(file);
        }

        virtual void registerFile(const EmbeddedFile* file) override final
        {
            auto lock = CreateLock(m_lock);

            DEBUG_CHECK_RETURN(file);
            DEBUG_CHECK_RETURN(file->name && *file->name);
            const auto fileName = StringView(file->name);

            const auto storageIndex = (int)file->type;
            DEBUG_CHECK_RETURN(storageIndex >= 0 && storageIndex < MAX_FILE_TYPES);

            auto& storage = m_storage[storageIndex];
            DEBUG_CHECK_RETURN_EX(!storage.fileMap.contains(fileName), TempString("Embedded file '{}' already registered", fileName));

            storage.fileMap[fileName] = file;
        }

        virtual void unregisterFile(const EmbeddedFile* file) override final
        {
            auto lock = CreateLock(m_lock);

            DEBUG_CHECK_RETURN(file);
            DEBUG_CHECK_RETURN(file->name && *file->name);
            const auto fileName = StringView(file->name);

            const auto storageIndex = (int)file->type;
            DEBUG_CHECK_RETURN(storageIndex >= 0 && storageIndex < MAX_FILE_TYPES);

            auto& storage = m_storage[storageIndex];
            storage.fileMap.remove(fileName);
        }

    private:
        SpinLock m_lock;

        static const auto MAX_FILE_TYPES = (int)EmbeddedFileType::MAX;

        struct Storage
        {
            HashMap<StringView, const EmbeddedFile*> fileMap;
        };

        Storage m_storage[MAX_FILE_TYPES];
    };

} // prv

IEmbeddedFilesRegistry::IEmbeddedFilesRegistry()
{}

IEmbeddedFilesRegistry::~IEmbeddedFilesRegistry()
{}

//--

static prv::LocalEmbeddedFileRegistry GEmbeddedFiles;

IEmbeddedFilesRegistry& EmbeddedFiles()
{
    return GEmbeddedFiles;
}

//--

END_INFERNO_NAMESPACE()
