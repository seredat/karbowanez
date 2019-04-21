// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero project
// Copyright (c) 2014-2018, The Forknote developers
// Copyright (c) 2018, Ryo Currency Project
// Copyright (c) 2016-2018, The Karbowanec developers
//
// This file is part of Karbo.
//
// Karbo is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Karbo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Karbo.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace CryptoNote {
namespace parameters {

const uint64_t DIFFICULTY_TARGET                             = 240; // seconds
const uint64_t CRYPTONOTE_MAX_BLOCK_NUMBER                   = 500000000;
const size_t   CRYPTONOTE_MAX_BLOCK_BLOB_SIZE                = 500000000;
const size_t   CRYPTONOTE_MAX_TX_SIZE                        = 1000000000;
const uint64_t CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX       = 111; // addresses start with "K"
const size_t   CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW          = 10;
const size_t   CRYPTONOTE_TX_SPENDABLE_AGE                   = 6;
const uint64_t CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT            = DIFFICULTY_TARGET * 7;
const uint64_t CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT_V1         = DIFFICULTY_TARGET * 3;
const size_t   BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW             = 60;
const size_t   BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW_V1          = 11;

// MONEY_SUPPLY - total number coins to be generated
const uint64_t MONEY_SUPPLY                                  = UINT64_C(10000000000000000000);
const uint64_t COIN                                          = UINT64_C(1000000000000);
const uint64_t TAIL_EMISSION_REWARD                          = UINT64_C(1000000000000);
const size_t CRYPTONOTE_COIN_VERSION                         = 1;
const unsigned EMISSION_SPEED_FACTOR                         = 18;
static_assert(EMISSION_SPEED_FACTOR <= 8 * sizeof(uint64_t), "Bad EMISSION_SPEED_FACTOR");

const size_t   CRYPTONOTE_REWARD_BLOCKS_WINDOW               = 100;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE     = 1000000; //size of block (bytes) after which reward for block calculated using block size
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V2  = 1000000;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1  = 100000;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_CURRENT = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE;
const size_t   CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE        = 600;
const size_t   CRYPTONOTE_DISPLAY_DECIMAL_POINT              = 12;

const uint64_t MINIMUM_FEE_V1                                = UINT64_C(100000000);
const uint64_t MINIMUM_FEE_V2                                = UINT64_C(100000000000);
const uint32_t MINIMUM_FEE_V2_HEIGHT                         = 216394;
const uint64_t MINIMUM_FEE                                   = MINIMUM_FEE_V2;
const uint64_t MAXIMUM_FEE                                   = UINT64_C(100000000000);

const uint64_t DEFAULT_DUST_THRESHOLD                        = UINT64_C(100000000);
const uint64_t MIN_TX_MIXIN_SIZE                             = 2;
const uint64_t MAX_TX_MIXIN_SIZE_V1                          = 50;
const uint64_t MAX_TX_MIXIN_SIZE_V2                          = 20;
const uint64_t MAX_TX_MIXIN_SIZE                             = MAX_TX_MIXIN_SIZE_V2;
const uint32_t MIN_TX_MIXIN_V1_HEIGHT                        = 216245;
const uint32_t MIN_TX_MIXIN_V2_HEIGHT                        = 216394;
const uint64_t MAX_TRANSACTION_SIZE_LIMIT                    = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_CURRENT / 4 - CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE;

const uint64_t EXPECTED_NUMBER_OF_BLOCKS_PER_DAY             = 24 * 60 * 60 / DIFFICULTY_TARGET;
const size_t   DIFFICULTY_WINDOW                             = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY; // blocks
const size_t   DIFFICULTY_WINDOW_V2                          = 17;  // blocks
const size_t   DIFFICULTY_WINDOW_V3                          = 60;  // blocks
const size_t   DIFFICULTY_CUT                                = 60;  // timestamps to cut after sorting
const size_t   DIFFICULTY_LAG                                = 15;  // !!!
static_assert(2 * DIFFICULTY_CUT <= DIFFICULTY_WINDOW - 2, "Bad DIFFICULTY_WINDOW or DIFFICULTY_CUT");

const uint64_t POISSON_CHECK_TRIGGER = 10; // Reorg size that triggers poisson timestamp check
const uint64_t POISSON_CHECK_DEPTH = 60;   // Main-chain depth of the poisson check. The attacker will have to tamper 50% of those blocks
const double POISSON_LOG_P_REJECT = -75.0; // Reject reorg if the probablity that the timestamps are genuine is below e^x, -75 = 10^-33

const size_t   MAX_BLOCK_SIZE_INITIAL                        = 1000000;
const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_NUMERATOR         = 100 * 1024;
const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_DENOMINATOR       = 365 * 24 * 60 * 60 / DIFFICULTY_TARGET;

const uint64_t CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS     = 1;
const uint64_t CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS    = DIFFICULTY_TARGET * CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS;

const uint64_t CRYPTONOTE_MEMPOOL_TX_LIVETIME                = 60 * 60 * 24;     //seconds, one day
const uint64_t CRYPTONOTE_MEMPOOL_TX_FROM_ALT_BLOCK_LIVETIME = 60 * 60 * 24 * 7; //seconds, one week
const uint64_t CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL = 7;  // CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL * CRYPTONOTE_MEMPOOL_TX_LIVETIME = time to forget tx

const size_t   FUSION_TX_MAX_SIZE                            = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1 * 30 / 100;
const size_t   FUSION_TX_MIN_INPUT_COUNT                     = 12;
const size_t   FUSION_TX_MIN_IN_OUT_COUNT_RATIO              = 4;

const uint32_t UPGRADE_HEIGHT_V2                             = 60000;
const uint32_t UPGRADE_HEIGHT_V3                             = 216000;
const uint32_t UPGRADE_HEIGHT_V4                             = 266000;
const uint32_t UPGRADE_HEIGHT_LWMA3                          = 300000;
const uint32_t UPGRADE_HEIGHT_V5                             = 4294967294;
const unsigned UPGRADE_VOTING_THRESHOLD                      = 90; // percent
const uint32_t UPGRADE_VOTING_WINDOW                         = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;  // blocks
const uint32_t UPGRADE_WINDOW                                = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;  // blocks
static_assert(0 < UPGRADE_VOTING_THRESHOLD && UPGRADE_VOTING_THRESHOLD <= 100, "Bad UPGRADE_VOTING_THRESHOLD");
static_assert(UPGRADE_VOTING_WINDOW > 1, "Bad UPGRADE_VOTING_WINDOW");

const char     CRYPTONOTE_BLOCKS_FILENAME[]                  = "blocks.dat";
const char     CRYPTONOTE_BLOCKINDEXES_FILENAME[]            = "blockindexes.dat";
const char     CRYPTONOTE_BLOCKSCACHE_FILENAME[]             = "blockscache.dat";
const char     CRYPTONOTE_POOLDATA_FILENAME[]                = "poolstate.bin";
const char     P2P_NET_DATA_FILENAME[]                       = "p2pstate.bin";
const char     CRYPTONOTE_BLOCKCHAIN_INDICES_FILENAME[]      = "blockchainindices.dat";
const char     MINER_CONFIG_FILE_NAME[]                      = "miner_conf.json";
} // parameters

const char     CRYPTONOTE_NAME[]                             = "karbowanec";
const char     GENESIS_COINBASE_TX_HEX[]                     = "010a01ff0001fac484c69cd608029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101f904925cc23f86f9f3565188862275dc556a9bdfb6aec22c5aca7f0177c45ba8";

const uint8_t  CURRENT_TRANSACTION_VERSION                   =  1;
const uint8_t  BLOCK_MAJOR_VERSION_1                         =  1;
const uint8_t  BLOCK_MAJOR_VERSION_2                         =  2;
const uint8_t  BLOCK_MAJOR_VERSION_3                         =  3;
const uint8_t  BLOCK_MAJOR_VERSION_4                         =  4;
const uint8_t  BLOCK_MAJOR_VERSION_5                         =  5;
const uint8_t  BLOCK_MINOR_VERSION_0                         =  0;
const uint8_t  BLOCK_MINOR_VERSION_1                         =  1;

const size_t   BLOCKS_IDS_SYNCHRONIZING_DEFAULT_COUNT        =  10000;  //by default, blocks ids count in synchronizing
const size_t   BLOCKS_SYNCHRONIZING_DEFAULT_COUNT            =  128;    //by default, blocks count in blocks downloading
const size_t   COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT         =  1000;

const int      P2P_DEFAULT_PORT                              =  32347;
const int      RPC_DEFAULT_PORT                              =  32348;
const int      RPC_DEFAULT_SSL_PORT                          =  32448;
const int      WALLET_RPC_DEFAULT_PORT                       =  15000;
const int      WALLET_RPC_DEFAULT_SSL_PORT                   =  15100;
const int      GATE_RPC_DEFAULT_PORT                         =  16000;
const int      GATE_RPC_DEFAULT_SSL_PORT                     =  16100;
const char     RPC_DEFAULT_CHAIN_FILE[]                      = "rpc_server.crt";
const char     RPC_DEFAULT_KEY_FILE[]                        = "rpc_server.key";
const char     RPC_DEFAULT_DH_FILE[]                         = "rpc_server.pem";

const size_t   P2P_LOCAL_WHITE_PEERLIST_LIMIT                =  1000;
const size_t   P2P_LOCAL_GRAY_PEERLIST_LIMIT                 =  5000;

const size_t   P2P_CONNECTION_MAX_WRITE_BUFFER_SIZE          = 64 * 1024 * 1024; // 64 MB
const uint32_t P2P_DEFAULT_CONNECTIONS_COUNT                 = 8;
const size_t   P2P_DEFAULT_WHITELIST_CONNECTIONS_PERCENT     = 70;
const uint32_t P2P_DEFAULT_HANDSHAKE_INTERVAL                = 60;            // seconds
const uint32_t P2P_DEFAULT_PACKET_MAX_SIZE                   = 50000000;      // 50000000 bytes maximum packet size
const uint32_t P2P_DEFAULT_PEERS_IN_HANDSHAKE                = 250;
const uint32_t P2P_DEFAULT_CONNECTION_TIMEOUT                = 5000;          // 5 seconds
const uint32_t P2P_DEFAULT_PING_CONNECTION_TIMEOUT           = 2000;          // 2 seconds
const uint64_t P2P_DEFAULT_INVOKE_TIMEOUT                    = 60 * 2 * 1000; // 2 minutes
const size_t   P2P_DEFAULT_HANDSHAKE_INVOKE_TIMEOUT          = 5000;          // 5 seconds
const uint32_t P2P_FAILED_ADDR_FORGET_SECONDS                = (60 * 60);     //1 hour
const uint32_t P2P_IP_BLOCKTIME                              = (60 * 60 * 24);//24 hour
const uint32_t P2P_IP_FAILS_BEFORE_BLOCK                     = 10;
const uint32_t P2P_IDLE_CONNECTION_KILL_INTERVAL             = (5 * 60);      //5 minutes

const char     P2P_STAT_TRUSTED_PUB_KEY[]                    = "8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115";

const char* const SEED_NODES[] = { 
  "seed1.karbowanec.com:32347",
  "seed2.karbowanec.com:32347",
  "seed.karbo.cloud:32347",
  "seed.karbo.org:32347",
  "seed.karbo.io:32347",
  "95.46.98.64:32347",
  "108.61.198.115:32347",
  "45.32.232.11:32347",
  "46.149.182.151:32347"
};

struct CheckpointData {
  uint32_t height;
  const char* blockId;
};

const std::initializer_list<CheckpointData> CHECKPOINTS = { 
  {3436,	"fa0348c379f63af68304f9f256ea99cc8560eda5a25740dc7ff94631fc7fcf5d" },
  {16970,	"456e43e923a02516559d89f567c3fa2068051a9fdac562a937eeaaaf3e9aab46" },
  {30000,	"4d9762f83ebebb462f1787862aa001e954dadafd203d2fdb973e4b2e52891cf4" },
  {50000,	"4616b7bad31127ce6fefc9b70f346c61507f8e74d29a8cfe0bdd7d047ba33ca2" },
  {60001,	"8e39967eb50b8a922cbfe22fe02989218345cbd61ae651ddbecf00834910ff50" },
  {98466,	"2a4f9183e801849c2b6b4a12324889efe3f22297c2ac8511c0644007eab9c728" },
  {98842,	"d66758508f0072fdd3c5b754167999b09ee3d0d39488c9cb9d3bb4f9f8f59c54" },
  {103700,	"5663cf2168371c745f163e5b4189ebac71242573cebf7d4c76fd1ac88da8b3bf" },
  {106700,	"678abbf293d1dae9eac831d5fc877a4f53bc98bb86c37e2e5c139af936fb2e1e" },
  {180660,	"6c921a5d3770fa798b038ae26ffd5d9b47ebd733dff4585df7f1632a2101f5a8" },
  {187600,	"f00c6e4b0f9630b52729976f10e532899517333d2ecba22318f820f4512eca69" },
  {200000,	"afdec3ea5af4f22299d4f0889af3cf27f1c9e20bee2d354d6a74400f61b3694d" },
  {213934,	"191f89f771c17ba13b4b6f9950cb013a6608dd8591abfbc67fdbf0197ab7b8da" },
  {214860,	"6084a42b411bcef34a66b48eea9af2f6cdfc12be548a7c423bf2ea927a23b0cf" },
  {216245,	"3af4f447f9f6d9cdf89794f1393708843c8dc14164c809d5233f1546fec3b338" },
  {216341,	"157aeca8472f07207cd0652e7bdfbccf99606d7304ca11dbfcc946f1b2747837" },
  {216394,	"1c42ca7b1cd3763028939b8f2d9e4f2f12d9dc261545fcd6adbe32d11678a823" },
  {216592,	"8061aab9e88e7a3f4181a0579d31a4c0560231d91f2b9e84828ae71208f634c6" },
  {217000,	"1a1b6866c5a725070cd7afb0ad93bd879e1619211248903a990a162ac0c58400" },
  {231000,	"a88c3b29ad95a7a7e06ab71ac668604889fd5710365d9687857b0c7f143543a4" },
  {256700,	"0dd0f2ff1a01ab01d92f9be4732775e8b461fdd037dfe258d53b690f7c10df6f" },
  {262662,	"05a363ef6d0d552ec181ef0b2b0f22878fe629ae492f8f251ca30df1e47e7eb4" },
  {266062,	"316a263ea3d7dc8eac0f0480952db806f798a706d81508819403ccbe20778ae1" },
  {266111,	"8705ab8a54403564375e663dea8310bb2d92eb57a165fa8967c611fa34f98950" },
  {269269,	"81ac921c64b11a8a724e4eb1c9f6e9556844f880e03173825b93ff20f65489d8" },
  {278340,	"efc338ccce81b4d7c0e57ed27ac8a8daa61182eada5802f82de154e341057397" },
  {285642,	"b9944f28c173e4b40b54cc9a74a14dddf4029a4e6be11e22a1833132e2bf3126" },
  {291494,	"a42efe20d0641336c242ba6049d7768baf83f23230da042fb3f980139edb908a" },
  {293740,	"dbe17265f0f27f43297bd52e7388b32e7a2f0d29db0cee7441ebf8b5b35db61e" },
  {297090,	"0eb8b7bea247fcd9da2160855d6779b1e4f378a30a34c898e481923772b6e004" },
  {300891,	"439834db795032e467a2ea6a06e8f7a034fc6d16c6959608673d68916f2e4cfa" },
  {304844,	"0710cd72e6ce7cf20ce4b77473d57bab95a422f545e8d353e6e7ee48ae1f8567" },
  {304924,	"f5531e3c074bc49f11484f276084d2287c2ce3823e9b6000c1a6b5c5fcae1142" },
  {306299,	"772d68beae36c7c72e0173c049eb984d397967b52b859e43038a012aa746477f" }
};

} // CryptoNote

#define ALLOW_DEBUG_COMMANDS



