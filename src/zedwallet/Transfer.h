// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <memory>

#include <zedwallet/Types.h>

void transfer(std::shared_ptr<WalletInfo> walletInfo, uint32_t height);

void doTransfer(std::string address, uint64_t amount, uint64_t fee,
                std::string extra, std::shared_ptr<WalletInfo> walletInfo,
                uint32_t height);

void sendMultipleTransactions(CryptoNote::WalletGreen &wallet,
                              std::vector<CryptoNote::TransactionParameters>
                              transfers);

void splitTx(CryptoNote::WalletGreen &wallet,
             CryptoNote::TransactionParameters p);

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo);

bool parseAmount(std::string amountString);

bool parseAddress(std::string address);

bool parseFee(std::string feeString);

std::string getExtraFromPaymentID(std::string paymentID);

Maybe<std::string> getPaymentID(std::string msg);

Maybe<std::string> getExtra();

Maybe<std::string> getDestinationAddress();

Maybe<uint64_t> getFee();

Maybe<uint64_t> getTransferAmount();
