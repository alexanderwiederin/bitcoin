# Blockfiles-Only Mode Implementation Changes

This document explains the changes made to implement a "blockfiles-only" mode in Bitcoin Core, where the UTXO database (LevelDB) is completely skipped during chainstate initialization and operation.

## Overview

The implementation adds functionality to run Bitcoin Core in a mode where it only processes block files without maintaining a UTXO (Unspent Transaction Output) database. This is useful for applications that need to read block data but don't need to validate or track transaction outputs.

## Changes Made

### 1. Added Skip UTXO Database Flag to ChainstateManager (validation.h)

```cpp
// In ChainstateManager class
bool m_skip_utxo_db{false};

// Methods to control UTXO database skipping
void SetSkipUTXODatabase(bool skip) { m_skip_utxo_db = skip; }
bool IsSkippingUTXODatabase() const { return m_skip_utxo_db; }
```

This flag indicates whether the UTXO database should be skipped entirely. When set to true, the chainstate initialization process will bypass LevelDB initialization.

### 2. Created a Dummy CCoinsView Implementation (validation.cpp)

```cpp
// Create a dummy coins view class for when UTXO database is skipped
class DummyCoinsView : public CCoinsView {
public:
    std::optional<Coin> GetCoin(const COutPoint& outpoint) const override { return std::nullopt; }
    bool HaveCoin(const COutPoint& outpoint) const override { return false; }
    uint256 GetBestBlock() const override { return uint256(); }
    bool BatchWrite(CoinsViewCacheCursor& cursor, const uint256& hashBlock) override { return true; }
};
```

This dummy implementation provides no-op implementations of all CCoinsView virtual methods, allowing the system to function without an actual UTXO database.

### 3. Modified ConnectBlock in Chainstate (validation.cpp & validation.h)

Added new parameter to ConnectBlock method:

```cpp
bool ConnectBlock(const CBlock& block, BlockValidationState& state, CBlockIndex* pindex,
                 CCoinsViewCache& view, bool fJustCheck = false, bool fSkipUTXOValidation = false)
```

And added an early-exit path when UTXO validation should be skipped:

```cpp
if (fSkipUTXOValidation) {
    if (!CheckBlock(block, state, m_chainman.GetConsensus())) {
        return false;
    }

    pindex->nStatus |= BLOCK_HAVE_DATA;

    return true;
}
```

This allows blocks to be processed and indexed without performing UTXO validation.

### 4. Modified LoadChainstate in node/chainstate.cpp

Added an early-return path when the UTXO database should be skipped:

```cpp
// Early return if skipping UTXO database - only initialize block-related components
if (chainman.IsSkippingUTXODatabase()) {
    LogPrintf("Blockfiles-only mode: Skipping UTXO database initialization\n");

    // Only load block index but skip all UTXO database operations
    if (!chainman.LoadBlockIndex()) {
        if (chainman.m_interrupt) return {ChainstateLoadStatus::INTERRUPTED, {}};
        return {ChainstateLoadStatus::FAILURE, _("Error loading block database")};
    }

    // Initialize chain with empty UTXO set
    if (!chainman.BlockIndex().empty() &&
            !chainman.ActiveChainstate().LoadGenesisBlock()) {
        return {ChainstateLoadStatus::FAILURE, _("Error initializing block database")};
    }

    return {ChainstateLoadStatus::SUCCESS, {}};
}
```

## Benefits

1. **No LevelDB Initialization**: Completely avoids initializing the LevelDB database for UTXO storage.
2. **Reduced Resource Usage**: Lower memory and CPU consumption when UTXO validation is not needed.
3. **Compatibility with Concurrent Access**: Prevents potential thread locking issues when multiple processes want to access block data.
4. **Faster Startup**: Skipping UTXO validation significantly reduces startup time.

## Use Cases

- Applications that only need to read historical block data
- Block explorers that maintain their own transaction database
- Tools that scan the blockchain for specific information without validating transactions
- Concurrent access to blockfiles by multiple applications