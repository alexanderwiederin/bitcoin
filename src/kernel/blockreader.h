// Copyright (c) 2022-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KERNEL_BLOCKREADER_H
#define BITCOIN_KERNEL_BLOCKREADER_H

#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#else
#include <cstddef>
#include <cstdint>
#endif // !__cplusplus

#include "bitcoinkernel.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct btck_BlockReaderOptions btck_BlockReaderOptions;

typedef struct btck_BlockReader btck_BlockReader;

BITCOINKERNEL_API btck_BlockReaderOptions* BITCOINKERNEL_WARN_UNUSED_RESULT btck_blockreader_options_create(
    const btck_Context* context,
    const btck_ChainParameters* chain_parameters,
    const char* blocks_directory,
    size_t blocks_directory_len,
    const char* data_directory,
    size_t data_directory_len) BITCOINKERNEL_ARG_NONNULL(1, 2, 3, 5);

BITCOINKERNEL_API void btck_blockreader_options_destroy(btck_BlockReaderOptions* blockreader_options);

BITCOINKERNEL_API btck_BlockReader* BITCOINKERNEL_WARN_UNUSED_RESULT btck_blockreader_create(
    const btck_BlockReaderOptions* blockreader_options) BITCOINKERNEL_ARG_NONNULL(1);

BITCOINKERNEL_API void btck_blockreader_destroy(btck_BlockReader* blockreader);

BITCOINKERNEL_API const btck_Chain* BITCOINKERNEL_WARN_UNUSED_RESULT btck_blockreader_get_validated_chain(const btck_BlockReader* blockreader) BITCOINKERNEL_ARG_NONNULL(1);

BITCOINKERNEL_API btck_Block* BITCOINKERNEL_WARN_UNUSED_RESULT btck_blockreader_read_block(
    const btck_BlockReader* blockreader,
    const btck_BlockTreeEntry* block_tree_entry) BITCOINKERNEL_ARG_NONNULL(1, 2);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !BITCOIN_KERNEL_BLOCKREADER_H
