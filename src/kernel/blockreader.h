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


typedef struct btck_BlockReader btck_BlockReader;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !BITCOIN_KERNEL_BLOCKREADER_H
