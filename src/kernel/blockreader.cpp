// Copyright (c) 2022-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "kernel/blockreader_opts.h"
#include "kernel/reader_impl.h"
#include "logging.h"
#include "util/fs.h"
#include <exception>
#include <kernel/bitcoinkernel.h>
#include <kernel/blockreader.h>
#include <memory>
#include <utility>

namespace {
template <typename C, typename CPP>
struct Handle {
    static C* ref(CPP* cpp_type)
    {
        return reinterpret_cast<C*>(cpp_type);
    }

    static const C* ref(const CPP* cpp_type)
    {
        return reinterpret_cast<const C*>(cpp_type);
    }

    template <typename... Args>
    static C* create(Args&&... args)
    {
        auto cpp_obj{std::make_unique<CPP>(std::forward<Args>(args)...)};
        return reinterpret_cast<C*>(cpp_obj.release());
    }

    static C* copy(const C* ptr)
    {
        auto cpp_obj{std::make_unique<CPP>(get(ptr))};
        return reinterpret_cast<C*>(cpp_obj.release());
    }

    static const CPP& get(const C* ptr)
    {
        return *reinterpret_cast<const CPP*>(ptr);
    }

    static CPP& get(C* ptr)
    {
        return *reinterpret_cast<CPP*>(ptr);
    }

    static void operator delete(void* ptr)
    {
        delete reinterpret_cast<CPP*>(ptr);
    }
};
} // namespace

struct btck_BlockReaderOptions : Handle<btck_BlockReaderOptions, kernel::BlockReaderOpts> {
};
struct btck_BlockReader : Handle<btck_BlockReader, blockreader::BlockReader> {
};
struct btck_ChainParameters : Handle<btck_ChainParameters, std::unique_ptr<const CChainParams>> {
};

extern "C" {

btck_BlockReaderOptions* btck_blockreader_options_create(
    const btck_Context* context,
    const btck_ChainParameters* chain_parameters,
    const char* blocks_directory,
    size_t blocks_directory_len,
    const char* data_directory,
    size_t data_directory_len)
{
    try {
        const auto& chainparams = *btck_ChainParameters::get(chain_parameters);

        fs::path abs_blocks_dir{fs::absolute(fs::PathFromString({blocks_directory, blocks_directory_len}))};
        fs::create_directories(abs_blocks_dir);
        fs::path abs_data_dir{fs::absolute(fs::PathFromString({data_directory, data_directory_len}))};
        fs::create_directories(abs_data_dir);

        return btck_BlockReaderOptions::create(chainparams, abs_blocks_dir, abs_data_dir);
    } catch (const std::exception& e) {
        LogError("Failed to create block reader options: %s", e.what());
        return nullptr;
    }
}

void btck_blockreader_options_destroy(btck_BlockReaderOptions* blockreader_options)
{
    delete blockreader_options;
}

btck_BlockReader* btck_blockreader_create(const btck_BlockReaderOptions* blockreader_options)
{
    try {
        const auto& opts{btck_BlockReaderOptions::get(blockreader_options)};

        return btck_BlockReader::create(opts.chainparams, opts.data_dir, opts.blocks_dir);
    } catch (const std::exception& e) {
        LogError("Failed to create block reader: %s", e.what());
        return nullptr;
    }
}

void btck_blockreader_destroy(btck_BlockReader* blockreader)
{
    delete blockreader;
}
} // extern "C"
