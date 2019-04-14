// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <memory>

#include <zedwallet/Types.h>
#include <zedwallet/WalletConfig.h>

enum BalanceInfo { NotEnoughBalance, EnoughBalance, SetMixinToZero };
void transfer(std::shared_ptr<WalletInfo> walletInfo, uint32_t height,
	bool sendAll = false, std::string nodeAddress = std::string());

void doTransfer(std::string address, uint64_t amount, uint64_t fee,
                std::string extra, std::shared_ptr<WalletInfo> walletInfo,
                uint32_t height, uint64_t mixin = WalletConfig::defaultMixin,
                std::string nodeAddress = std::string(), uint32_t nodeFee = 0);

void sendMultipleTransactions(CryptoNote::WalletGreen &wallet,
                              std::vector<CryptoNote::TransactionParameters>
                              transfers);

void splitTx(CryptoNote::WalletGreen &wallet,
             CryptoNote::TransactionParameters p);

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo, uint32_t nodeFee);

bool parseAmount(std::string amountString);

bool parseAddress(std::string address);

bool parseFee(std::string feeString);

std::string getExtraFromPaymentID(std::string paymentID);

Maybe<std::string> getPaymentID(std::string msg);

Maybe<std::string> getExtra();

Maybe<std::string> getDestinationAddress();

Maybe<uint64_t> getFee();

Maybe<uint64_t> getTransferAmount();

BalanceInfo doWeHaveEnoughBalance(uint64_t amount, uint64_t fee,
	std::shared_ptr<WalletInfo> walletInfo,
	uint64_t height, uint32_t nodeFee);

uint64_t calculateNodeFee(uint64_t amount);