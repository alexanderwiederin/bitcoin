// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KERNEL_BLOCKREADER_OPTS_H
#define BITCOIN_KERNEL_BLOCKREADER_OPTS_H

#include "util/fs.h"

class CChainParams;

namespace kernel {
struct BlockReaderOpts {
    const CChainParams& chainparams;
    const fs::path blocks_dir;
    const fs::path data_dir;

    BlockReaderOpts(const CChainParams& cp, const fs::path& bd, const fs::path& dd)
        : chainparams(cp), blocks_dir(bd), data_dir(dd) {}
};
} // namespace kernel

#endif // !BITCOIN_KERNEL_BLOCKREADER_OPTS_H
