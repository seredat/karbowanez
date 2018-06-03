// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "IWalletLegacy.h"
#include "ITransfersContainer.h"

#include <unordered_map>
#include <unordered_set>
#include <time.h>
#include <boost/functional/hash.hpp>

#include "CryptoNoteCore/CryptoNoteBasic.h"
#include "crypto/crypto.h"

namespace CryptoNote {
class ISerializer;

typedef std::pair<Crypto::PublicKey, size_t> TransactionOutputId;
}

namespace std {

template<> 
struct hash<CryptoNote::TransactionOutputId> {
  size_t operator()(const CryptoNote::TransactionOutputId &_v) const {    
    return hash<Crypto::PublicKey>()(_v.first) ^ _v.second;
  } 
}; 

}

namespace CryptoNote {


struct UnconfirmedTransferDetails {

  UnconfirmedTransferDetails() :
    amount(0), sentTime(0), transactionId(WALLET_LEGACY_INVALID_TRANSACTION_ID) {}

  CryptoNote::Transaction tx;
  uint64_t amount;
  uint64_t outsAmount;
  time_t sentTime;
  TransactionId transactionId;
  std::vector<TransactionOutputId> usedOutputs;
};

struct UnconfirmedSpentDepositDetails {
  TransactionId transactionId;
  uint64_t depositsSum;
  uint64_t fee;
};

class WalletUnconfirmedTransactions
{
public:

  explicit WalletUnconfirmedTransactions(uint64_t uncofirmedTransactionsLiveTime);

  bool serialize(CryptoNote::ISerializer& s);
  bool deserializeV1(CryptoNote::ISerializer& s);

  bool findTransactionId(const Crypto::Hash& hash, TransactionId& id);
  void erase(const Crypto::Hash& hash);
  void add(const CryptoNote::Transaction& tx, TransactionId transactionId, 
    uint64_t amount, const std::vector<TransactionOutputInformation>& usedOutputs);
  void updateTransactionId(const Crypto::Hash& hash, TransactionId id);

  void addCreatedDeposit(DepositId id, uint64_t totalAmount);
  void addDepositSpendingTransaction(const Crypto::Hash& transactionHash, const UnconfirmedSpentDepositDetails& details);

  void eraseCreatedDeposit(DepositId id);

  uint64_t countCreatedDepositsSum() const;
  uint64_t countSpentDepositsProfit() const;
  uint64_t countSpentDepositsTotalAmount() const;

  uint64_t countUnconfirmedOutsAmount() const;
  uint64_t countUnconfirmedTransactionsAmount() const;
  bool isUsed(const TransactionOutputInformation& out) const;
  void reset();

  std::vector<TransactionId> deleteOutdatedTransactions();

private:

  void collectUsedOutputs();
  void deleteUsedOutputs(const std::vector<TransactionOutputId>& usedOutputs);

  bool eraseUnconfirmedTransaction(const Crypto::Hash& hash);
  bool eraseDepositSpendingTransaction(const Crypto::Hash& hash);

  bool findUnconfirmedTransactionId(const Crypto::Hash& hash, TransactionId& id);
  bool findUnconfirmedDepositSpendingTransactionId(const Crypto::Hash& hash, TransactionId& id);

  typedef std::unordered_map<Crypto::Hash, UnconfirmedTransferDetails, boost::hash<Crypto::Hash>> UnconfirmedTxsContainer;
  typedef std::unordered_set<TransactionOutputId> UsedOutputsContainer;

  UnconfirmedTxsContainer m_unconfirmedTxs;
  UsedOutputsContainer m_usedOutputs;
  uint64_t m_uncofirmedTransactionsLiveTime;

  std::unordered_map<DepositId, uint64_t> m_createdDeposits;
  std::unordered_map<Crypto::Hash, UnconfirmedSpentDepositDetails> m_spentDeposits;
};

} // namespace CryptoNote
