// Copyright (c) 2022-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <kernel/blockreader.h>
#include <kernel/chainparams.h>
#include <kernel/cs_main.h>
#include <kernel/notifications_interface.h>
#include <logging.h>
#include <node/blockstorage.h>
#include <sync.h>
#include <util/signalinterrupt.h>
#include <validation.h>

namespace blockreader {

class NoOpNotifications : public kernel::Notifications
{
public:
    kernel::InterruptResult blockTip(SynchronizationState state, const CBlockIndex& index, double verification_progress) override { return {}; }
    void headerTip(SynchronizationState state, int64_t height, int64_t timestamp, bool presync) override {}
    void progress(const bilingual_str& title, int progress_percent, bool resume_possible) override {}
    void warningSet(kernel::Warning id, const bilingual_str& message) override {}
    void warningUnset(kernel::Warning id) override {}
    void flushError(const bilingual_str& message) override {}
    void fatalError(const bilingual_str& message) override {}
};

BlockReader::BlockReader(const Options& options, util::SignalInterrupt& interrupt) : m_interrupt(interrupt), m_notifications(std::make_unique<NoOpNotifications>())
{
    node::BlockManager::Options blockman_options{
        .chainparams = options.chainparams,
        .blocks_dir = options.blocks_dir,
        .notifications = *m_notifications,
        .block_tree_dir = options.data_dir / "blocks" / "index",
        .read_only = true};

    m_blockman = std::make_unique<node::BlockManager>(m_interrupt, blockman_options);
    if (!UpdateChainSnapshot()) {
        LogError("BlockReader: Failed to load validated chain");
        throw std::runtime_error("Failed to load validated chain");
    }
}

bool BlockReader::UpdateChainSnapshot()
{
    std::vector<CBlockIndex*> validated_blocks;
    {
        LOCK(cs_main);
        if (!m_blockman->LoadBlockIndexDB({})) {
            LogWarning("Failed to load block index database");
            return false;
        }
        for (CBlockIndex* index : m_blockman->GetAllBlockIndices()) {
            if (index->IsValid(BLOCK_VALID_SCRIPTS)) {
                validated_blocks.push_back(index);
            }
        }
    }

    if (!validated_blocks.empty()) {
        std::sort(validated_blocks.begin(), validated_blocks.end(),
                  node::CBlockIndexWorkComparator());

        auto new_chain{std::make_shared<CChain>()};
        new_chain->SetTip(*validated_blocks.back());
        std::atomic_store(&m_chain_snapshot, new_chain);
    }
    return true;
}

bool BlockReader::ReadBlock(CBlock& block, const CBlockIndex& index)
{
    if (m_interrupt) {
        return false;
    }

    LOCK(cs_main);
    if (!m_blockman->ReadBlock(block, index)) {
        LogError("BlockReader: Failed to read block %s from disk",
                 index.phashBlock->ToString());
        return false;
    }

    return true;
}

bool BlockReader::ReadBlockUndo(CBlockUndo& block_undo, const CBlockIndex& index)
{
    if (m_interrupt) {
        return false;
    }

    LOCK(cs_main);
    if (!m_blockman->ReadBlockUndo(block_undo, index)) {
        LogError("BlockReader: Failed to read block undo data for block %s from disk",
                 index.phashBlock->ToString());
        return false;
    }

    return true;
}

bool BlockReader::Refresh()
{
    return UpdateChainSnapshot();
}
} // namespace blockreader
