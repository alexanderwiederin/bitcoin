# Implementing Blockfiles-Only Mode for Bitcoin Core

This document outlines the implementation plan for creating a chainstate manager that only processes blockfiles without using the UTXO database (LevelDB).

## Problem Statement

We need a way to create a chainstate manager where we are only interested in blockfiles and don't care about the UTXO chainstate at all. This requires changes to Bitcoin Core to avoid LevelDB initialization and usage completely, as LevelDB can only be used by a single thread at a time.

## Implementation Plan

### 1. Bitcoin Core Changes (C++)

#### A. Add New Configuration Option (src/node/chainstate.h)

```cpp
class ChainstateManager {
private:
    bool m_skip_utxo_db{false};
    
public:
    // New method to set flag
    void SetSkipUTXODatabase(bool skip) { m_skip_utxo_db = skip; }
    bool IsSkippingUTXODatabase() const { return m_skip_utxo_db; }
    
    // Existing methods...
};
```

#### B. Modify ChainstateManager Constructor (src/node/chainstate.cpp)

```cpp
ChainstateManager::ChainstateManager(...) {
    // Early branch if skipping UTXO database
    if (m_skip_utxo_db) {
        // Only initialize block-related components
        
        // 1. Set up block tree DB (still needed for block index)
        m_blockman.m_block_tree_db = std::make_unique<CBlockTreeDB>(cache_size, false, fReindex);
        
        // 2. Initialize blockman without UTXO components
        m_blockman.LoadBlockIndex(chainparams);
        
        // 3. Create dummy chain but don't initialize UTXO database
        if (!m_blockman.m_block_index.empty()) {
            m_chain.SetTip(FindMostWorkChain());
        }
        
        // Skip all UTXO database initialization
        return;
    }
    
    // Normal initialization with full UTXO database...
    // (existing code)
}
```

#### C. Modify Import Blocks Function (src/node/chainstate.cpp)

```cpp
bool ChainstateManager::ImportBlocks(...) {
    if (m_skip_utxo_db) {
        // Simplified version that only processes block files
        
        // For each block file:
        while (true) {
            // 1. Read block from disk
            CBlock block;
            if (!ReadBlockFromDisk(block, ...)) {
                break;
            }
            
            // 2. Process block header and update index
            CBlockIndex* pindex = nullptr;
            BlockValidationState state;
            if (!AcceptBlockHeader(block, state, chainparams, &pindex)) {
                // Handle error
                continue;
            }
            
            // 3. Add to block index but skip full validation
            m_blockman.AddToBlockIndex(block.GetBlockHeader(), pindex);
            
            // 4. Update block file info
            RecordBlockchainState();
        }
        
        return true;
    }
    
    // Normal block import with UTXO validation
    // (existing code)
}
```

#### D. Modify Block Validation (src/validation.cpp)

```cpp
// Add skip parameter to ConnectBlock
bool ConnectBlock(const CBlock& block, BlockValidationState& state, CBlockIndex* pindex,
                 CCoinsViewCache& view, const CChainParams& chainparams, bool fJustCheck, bool fSkipUTXOValidation) {
    
    // Early return for UTXO database skipping
    if (fSkipUTXOValidation) {
        // Still validate block structure and header
        // But skip all UTXO-related checks
        
        // Basic block structure validation
        if (!CheckBlock(block, state, chainparams.GetConsensus())) {
            return false;
        }
        
        // Mark block as valid but not fully validated
        pindex->nStatus |= BLOCK_HAVE_DATA;
        
        return true;
    }
    
    // Normal validation with UTXO checks
    // (existing code)
}
```

#### E. Create Dummy UTXO Views (src/validation.cpp)

```cpp
// Create a dummy coins view class
class DummyCoinsView : public CCoinsView {
public:
    bool GetCoin(const COutPoint& outpoint, Coin& coin) const override { return false; }
    bool HaveCoin(const COutPoint& outpoint) const override { return false; }
    uint256 GetBestBlock() const override { return uint256(); }
    void BatchWrite(CCoinsMap& map, const uint256& hashBlock) override {}
};

// Use this when m_skip_utxo_db is true
```

### 2. Rust-Bitcoinkernel Changes

#### A. Add New Option to ChainstateManagerOptions (src/chainstate.rs)

```rust
impl ChainstateManagerOptions {
    /// Skip UTXO database initialization completely
    /// Only process blockfiles without UTXO validation
    pub fn set_skip_utxo_database(mut self, skip: bool) -> Self {
        unsafe {
            ffi::bitcoinkernel_chainstate_manager_options_set_skip_utxo_database(
                self.inner.as_ptr(),
                skip,
            );
        }
        self
    }
}
```

#### B. Add FFI Declarations (src/ffi.rs)

```rust
extern "C" {
    // Add new function declaration
    pub(crate) fn bitcoinkernel_chainstate_manager_options_set_skip_utxo_database(
        options: *mut c_void,
        skip: bool,
    );
}
```

#### C. Implement C++ FFI Function (src/rust/bitcoinkernel/chainstate.cpp)

```cpp
extern "C" EXPORT void bitcoinkernel_chainstate_manager_options_set_skip_utxo_database(
    ChainstateManagerOptions* options,
    bool skip) {
    options->set_skip_utxo_database(skip);
}
```

#### D. Add Method to ChainstateManagerOptions (src/rust/bitcoinkernel/chainstate.h)

```cpp
class ChainstateManagerOptions {
private:
    bool m_skip_utxo_database{false};
    
public:
    void set_skip_utxo_database(bool skip) { m_skip_utxo_database = skip; }
    bool get_skip_utxo_database() const { return m_skip_utxo_database; }
    
    // Pass this setting to ChainstateManager in apply_to
    void apply_to(ChainstateManager& chainman) const {
        // Existing code...
        chainman.SetSkipUTXODatabase(m_skip_utxo_database);
    }
};
```

### 3. Your Application Changes (kernel-test)

#### A. Update main.rs to Use the New Option

```rust
// Create options - with blockfiles-only mode
let mut options = ChainstateManagerOptions::new(&context, data_dir, blocks_dir)
    .expect("Failed to create options");

// Use in-memory chainstate DB (not actually used but set for compatibility)
options = options.set_chainstate_db_in_memory(true);

// Skip UTXO database completely - blockfiles-only mode
options = options.set_skip_utxo_database(true);

println!("Options created successfully - configured for blockfiles only");
```

## Implementation Strategy

1. First modify Bitcoin Core's ChainstateManager to add the skip option
2. Implement the dummy views and modify validation code
3. Update the rust-bitcoinkernel bindings to expose the new option
4. Modify your application code to use the new option

## Benefits

This implementation:
- Completely avoids LevelDB initialization for UTXO database
- Still processes blockfiles and maintains block index
- Prevents thread locking issues with LevelDB
- Creates a "blockfiles-only mode" for your application

## Notes

- The changes are invasive but isolated to specific components, making them manageable to implement and maintain
- Bitcoin Core can still add new blocks to the chain normally while your application accesses blockfiles data
- LevelDB will never be initialized, preventing any thread locking issues