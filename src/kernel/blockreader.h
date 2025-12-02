// Copyright (c) 2022-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KERNEL_BLOCKREADER_H
#define BITCOIN_KERNEL_BLOCKREADER_H

#include <kernel/chainparams.h>
#include <kernel/notifications_interface.h>
#include <memory>
#include <node/blockstorage.h>
#include <util/fs.h>
#include <util/signalinterrupt.h>

namespace blockreader {

class BlockReader
{
private:
    std::unique_ptr<node::BlockManager> m_blockman;
    const util::SignalInterrupt& m_interrupt;
    std::unique_ptr<kernel::Notifications> m_notifications;
    std::shared_ptr<CChain> m_chain_snapshot;
    bool UpdateChainSnapshot();

public:
    struct Options {
        const CChainParams& chainparams;
        const fs::path blocks_dir;
        const fs::path data_dir;
    };

    BlockReader(const Options& options, util::SignalInterrupt& interrupt);
    ~BlockReader() = default;
    BlockReader(const BlockReader&) = delete;
    BlockReader& operator=(const BlockReader&) = delete;

    std::shared_ptr<const CChain> GetChainSnapshot() const { return std::atomic_load(&m_chain_snapshot); }

    bool ReadBlock(CBlock& block, const CBlockIndex& index);
    bool ReadBlockUndo(CBlockUndo& block_undo, const CBlockIndex& index);

    bool Refresh();
};

} // namespace blockreader

#endif // BITCOIN_KERNEL_BLOCKREADER_H
