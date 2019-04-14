// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <NodeRpcProxy/NodeRpcProxy.h>
#include <GreenWallet/Types.h>

int main(int argc, char **argv);

void run(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node,
         Config &config);
