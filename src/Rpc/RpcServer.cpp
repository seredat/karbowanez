// Copyright (c) 2012-2016, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2016, The Forknote developers
// Copyright (c) 2016-2019, The Karbowanec developers
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

#include "RpcServer.h"
#include "version.h"

#include <future>
#include <unordered_map>

// CryptoNote
#include <crypto/random.h>
#include "BlockchainExplorerData.h"
#include "Common/StringTools.h"
#include "Common/Base58.h"
#include "CryptoNoteCore/TransactionUtils.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteCore/Core.h"
#include "CryptoNoteCore/IBlock.h"
#include "CryptoNoteCore/Miner.h"
#include "CryptoNoteCore/TransactionExtra.h"
#include "CryptoNoteProtocol/ICryptoNoteProtocolQuery.h"

#include "P2p/NetNode.h"

#include "CoreRpcServerErrorCodes.h"
#include "JsonRpc.h"

#undef ERROR

static const Crypto::SecretKey I = { { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

namespace CryptoNote {

namespace {

template <typename Command>
RpcServer::HandlerFunction binMethod(bool (RpcServer::*handler)(typename Command::request const&, typename Command::response&)) {
  return [handler](RpcServer* obj, const HttpRequest& request, HttpResponse& response) {

    boost::value_initialized<typename Command::request> req;
    boost::value_initialized<typename Command::response> res;

    if (!loadFromBinaryKeyValue(static_cast<typename Command::request&>(req), request.getBody())) {
      return false;
    }

    bool result = (obj->*handler)(req, res);
    response.setBody(storeToBinaryKeyValue(res.data()));
    return result;
  };
}

template <typename Command>
RpcServer::HandlerFunction jsonMethod(bool (RpcServer::*handler)(typename Command::request const&, typename Command::response&)) {
  return [handler](RpcServer* obj, const HttpRequest& request, HttpResponse& response) {

    boost::value_initialized<typename Command::request> req;
    boost::value_initialized<typename Command::response> res;

    if (!loadFromJson(static_cast<typename Command::request&>(req), request.getBody())) {
      return false;
    }

    bool result = (obj->*handler)(req, res);
    std::string cors_domain = obj->getCorsDomain();
    if (!cors_domain.empty()) {
      response.addHeader("Access-Control-Allow-Origin", cors_domain);
      response.addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
      response.addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    }
    response.addHeader("Content-Type", "application/json");
    response.setBody(storeToJson(res.data()));
    return result;
  };
}

}

std::unordered_map<std::string, RpcServer::RpcHandler<RpcServer::HandlerFunction>> RpcServer::s_handlers = {
  
  // binary handlers
  { "/getblocks.bin", { binMethod<COMMAND_RPC_GET_BLOCKS_FAST>(&RpcServer::on_get_blocks), false } },
  { "/queryblocks.bin", { binMethod<COMMAND_RPC_QUERY_BLOCKS>(&RpcServer::on_query_blocks), false } },
  { "/queryblockslite.bin", { binMethod<COMMAND_RPC_QUERY_BLOCKS_LITE>(&RpcServer::on_query_blocks_lite), false } },
  { "/get_o_indexes.bin", { binMethod<COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES>(&RpcServer::on_get_indexes), false } },
  { "/getrandom_outs.bin", { binMethod<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS>(&RpcServer::on_get_random_outs), false } },
  { "/get_pool_changes.bin", { binMethod<COMMAND_RPC_GET_POOL_CHANGES>(&RpcServer::on_get_pool_changes), false } },
  { "/get_pool_changes_lite.bin", { binMethod<COMMAND_RPC_GET_POOL_CHANGES_LITE>(&RpcServer::on_get_pool_changes_lite), false } },

  // http get json handlers
  { "/getinfo", { jsonMethod<COMMAND_RPC_GET_INFO>(&RpcServer::on_get_info), true } },
  { "/getheight", { jsonMethod<COMMAND_RPC_GET_HEIGHT>(&RpcServer::on_get_height), true } },
  { "/feeaddress", { jsonMethod<COMMAND_RPC_GET_FEE_ADDRESS>(&RpcServer::on_get_fee_address), true } },
  { "/peers", { jsonMethod<COMMAND_RPC_GET_PEER_LIST>(&RpcServer::on_get_peer_list), true } }, // deprecated
  { "/getpeers", { jsonMethod<COMMAND_RPC_GET_PEER_LIST>(&RpcServer::on_get_peer_list), true } },
  { "/paymentid", { jsonMethod<COMMAND_RPC_GEN_PAYMENT_ID>(&RpcServer::on_get_payment_id), true } },

  // rpc post json handlers
  { "/gettransactions", { jsonMethod<COMMAND_RPC_GET_TRANSACTIONS>(&RpcServer::on_get_transactions), true } },
  { "/sendrawtransaction", { jsonMethod<COMMAND_RPC_SEND_RAW_TRANSACTION>(&RpcServer::on_send_raw_transaction), false } },
  { "/getblocks", { jsonMethod<COMMAND_RPC_GET_BLOCKS_FAST>(&RpcServer::on_get_blocks), true } },
  { "/queryblocks", { jsonMethod<COMMAND_RPC_QUERY_BLOCKS>(&RpcServer::on_query_blocks), false } },
  { "/queryblockslite", { jsonMethod<COMMAND_RPC_QUERY_BLOCKS_LITE>(&RpcServer::on_query_blocks_lite), false } },
  { "/get_o_indexes", { jsonMethod<COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES>(&RpcServer::on_get_indexes), false } },
  { "/getrandom_outs", { jsonMethod<COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS>(&RpcServer::on_get_random_outs), false } },
  { "/get_pool_changes", { jsonMethod<COMMAND_RPC_GET_POOL_CHANGES>(&RpcServer::on_get_pool_changes), false } },
  { "/get_pool_changes_lite", { jsonMethod<COMMAND_RPC_GET_POOL_CHANGES_LITE>(&RpcServer::on_get_pool_changes_lite), false } },
  { "/get_block_details_by_height", { jsonMethod<COMMAND_RPC_GET_BLOCK_DETAILS_BY_HEIGHT>(&RpcServer::on_get_block_details_by_height), true } },
  { "/get_blocks_details_by_heights", { jsonMethod<COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HEIGHTS>(&RpcServer::on_get_blocks_details_by_heights), true } },
  { "/get_blocks_details_by_hashes", { jsonMethod<COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HASHES>(&RpcServer::on_get_blocks_details_by_hashes), true } },
  { "/get_blocks_hashes_by_timestamps", { jsonMethod<COMMAND_RPC_GET_BLOCKS_HASHES_BY_TIMESTAMPS>(&RpcServer::on_get_blocks_hashes_by_timestamps), true } },
  { "/get_transaction_details_by_hashes", { jsonMethod<COMMAND_RPC_GET_TRANSACTIONS_DETAILS_BY_HASHES>(&RpcServer::on_get_transactions_details_by_hashes), true } },
  { "/get_transaction_hashes_by_payment_id", { jsonMethod<COMMAND_RPC_GET_TRANSACTION_HASHES_BY_PAYMENT_ID>(&RpcServer::on_get_transaction_hashes_by_paymentid), true } },
  
  // disabled in restricted rpc mode
  { "/start_mining", { jsonMethod<COMMAND_RPC_START_MINING>(&RpcServer::on_start_mining), false } },
  { "/stop_mining", { jsonMethod<COMMAND_RPC_STOP_MINING>(&RpcServer::on_stop_mining), false } },
  { "/stop_daemon", { jsonMethod<COMMAND_RPC_STOP_DAEMON>(&RpcServer::on_stop_daemon), true } },

  // json rpc
  { "/json_rpc", { std::bind(&RpcServer::processJsonRpcRequest, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), true } }
};

RpcServer::RpcServer(System::Dispatcher& dispatcher, Logging::ILogger& log, CryptoNote::Core& core, NodeServer& p2p, ICryptoNoteProtocolQuery& protocolQuery) :
  HttpServer(dispatcher, log), logger(log, "RpcServer"), m_core(core), m_p2p(p2p), m_protocolQuery(protocolQuery), blockchainExplorerDataBuilder(core, protocolQuery) {
}

void RpcServer::processRequest(const HttpRequest& request, HttpResponse& response) {
  logger(Logging::TRACE) << "RPC request came: \n" << request << std::endl;

  auto url = request.getUrl();

  auto it = s_handlers.find(url);
  if (it == s_handlers.end()) {
    response.setStatus(HttpResponse::STATUS_404);
    return;
  }

  if (!it->second.allowBusyCore && !isCoreReady()) {
    response.setStatus(HttpResponse::STATUS_500);
    response.setBody("Core is busy");
    return;
  }

  it->second.handler(this, request, response);
}

bool RpcServer::processJsonRpcRequest(const HttpRequest& request, HttpResponse& response) {

  using namespace JsonRpc;

  response.addHeader("Content-Type", "application/json");
  if (!m_cors_domain.empty()) {
    response.addHeader("Access-Control-Allow-Origin", m_cors_domain);
    response.addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    response.addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  }	

  JsonRpcRequest jsonRequest;
  JsonRpcResponse jsonResponse;

  try {
    logger(Logging::TRACE) << "JSON-RPC request: " << request.getBody();
    jsonRequest.parseRequest(request.getBody());
    jsonResponse.setId(jsonRequest.getId()); // copy id

    static std::unordered_map<std::string, RpcServer::RpcHandler<JsonMemberMethod>> jsonRpcHandlers = {
	
      { "getblockcount", { makeMemberMethod(&RpcServer::on_getblockcount), true } },
      { "getblockhash", { makeMemberMethod(&RpcServer::on_getblockhash), true } },
      { "getblocktemplate", { makeMemberMethod(&RpcServer::on_getblocktemplate), true } },
      { "getblockheaderbyhash", { makeMemberMethod(&RpcServer::on_get_block_header_by_hash), true } },
      { "getblockheaderbyheight", { makeMemberMethod(&RpcServer::on_get_block_header_by_height), true } },
      { "getblockbyheight", { makeMemberMethod(&RpcServer::on_get_block_details_by_height), true } },
      { "getblockbyhash", { makeMemberMethod(&RpcServer::on_get_block_details_by_hash), true } },
      { "getblocksbyheights", { makeMemberMethod(&RpcServer::on_get_blocks_details_by_heights), true } },
      { "getblocksbyhashes", { makeMemberMethod(&RpcServer::on_get_blocks_details_by_hashes), true } },
      { "getblockshashesbytimestamps", { makeMemberMethod(&RpcServer::on_get_blocks_hashes_by_timestamps), true } },
      { "getblockslist", { makeMemberMethod(&RpcServer::on_blocks_list_json), true } },
      { "getlastblockheader", { makeMemberMethod(&RpcServer::on_get_last_block_header), true } },
      { "gettransaction", { makeMemberMethod(&RpcServer::on_get_transaction_details_by_hash), true } },
      { "gettransactionspool", { makeMemberMethod(&RpcServer::on_get_transactions_pool), true } },
      { "gettransactionsbypaymentid", { makeMemberMethod(&RpcServer::on_get_transactions_by_payment_id), true } },
      { "gettransactionhashesbypaymentid", { makeMemberMethod(&RpcServer::on_get_transaction_hashes_by_paymentid), true } },
      { "gettransactionsbyhashes", { makeMemberMethod(&RpcServer::on_get_transactions_details_by_hashes), true } },
      { "getcurrencyid", { makeMemberMethod(&RpcServer::on_get_currency_id), true } },
      { "checktransactionkey", { makeMemberMethod(&RpcServer::on_check_transaction_key), true } },
      { "checktransactionbyviewkey", { makeMemberMethod(&RpcServer::on_check_transaction_with_view_key), true } },
      { "checktransactionproof", { makeMemberMethod(&RpcServer::on_check_transaction_proof), true } },
      { "checkreserveproof", { makeMemberMethod(&RpcServer::on_check_reserve_proof), true } },
      { "validateaddress", { makeMemberMethod(&RpcServer::on_validate_address), true } },
      { "verifymessage", { makeMemberMethod(&RpcServer::on_verify_message), true } },
      { "submitblock", { makeMemberMethod(&RpcServer::on_submitblock), false } },

    };

    auto it = jsonRpcHandlers.find(jsonRequest.getMethod());
    if (it == jsonRpcHandlers.end()) {
      throw JsonRpcError(JsonRpc::errMethodNotFound);
    }

    if (!it->second.allowBusyCore && !isCoreReady()) {
      throw JsonRpcError(CORE_RPC_ERROR_CODE_CORE_BUSY, "Core is busy");
    }

    it->second.handler(this, jsonRequest, jsonResponse);

  } catch (const JsonRpcError& err) {
    jsonResponse.setError(err);
  } catch (const std::exception& e) {
    jsonResponse.setError(JsonRpcError(JsonRpc::errInternalError, e.what()));
  }

  response.setBody(jsonResponse.getBody());
  logger(Logging::TRACE) << "JSON-RPC response: " << jsonResponse.getBody();
  return true;
}

bool RpcServer::restrictRpc(const bool is_restricted) {
  m_restricted_rpc = is_restricted;
  return true;
}

bool RpcServer::enableCors(const std::string domain) {
  m_cors_domain = domain;
  return true;
}

std::string RpcServer::getCorsDomain() {
  return m_cors_domain;
}

bool RpcServer::setFeeAddress(const std::string& fee_address, const AccountPublicAddress& fee_acc) {
  m_fee_address = fee_address;
  m_fee_acc = fee_acc;
  return true;
}

bool RpcServer::setViewKey(const std::string& view_key) {
  Crypto::Hash private_view_key_hash;
  size_t size;
  if (!Common::fromHex(view_key, &private_view_key_hash, sizeof(private_view_key_hash), size) || size != sizeof(private_view_key_hash)) {
    logger(Logging::INFO) << "Could not parse private view key";
    return false;
  }
  m_view_key = *(struct Crypto::SecretKey *) &private_view_key_hash;
  return true;
}

bool RpcServer::setContactInfo(const std::string& contact) {
  m_contact_info = contact;
  return true;
}

bool RpcServer::isCoreReady() {
  return m_core.currency().isTestnet() || m_p2p.get_payload_object().isSynchronized();
}

bool RpcServer::checkIncomingTransactionForFee(const BinaryArray& tx_blob) {
	Crypto::Hash tx_hash = NULL_HASH;
	Crypto::Hash tx_prefixt_hash = NULL_HASH;
	Transaction tx;
	if (!parseAndValidateTransactionFromBinaryArray(tx_blob, tx, tx_hash, tx_prefixt_hash)) {
		logger(Logging::INFO) << "Could not parse tx from blob";
		return false;
	}
	
	// always relay fusion transactions
	uint64_t inputs_amount = 0;
	get_inputs_money_amount(tx, inputs_amount);
	uint64_t outputs_amount = get_outs_money_amount(tx);

	const uint64_t fee = inputs_amount - outputs_amount;
	if (fee == 0 && m_core.currency().isFusionTransaction(tx, tx_blob.size(), m_core.get_current_blockchain_height() - 1)) {
		logger(Logging::DEBUGGING) << "Masternode received fusion transaction, relaying with no fee check";
		return true;
	}

	CryptoNote::TransactionPrefix transaction = *static_cast<const TransactionPrefix*>(&tx);

	std::vector<uint32_t> out;
	uint64_t amount;

	if (!CryptoNote::findOutputsToAccount(transaction, m_fee_acc, m_view_key, out, amount)) {
		logger(Logging::INFO) << "Could not find outputs to masternode fee address";
		return false;
	}

	if (amount != 0) {
		logger(Logging::INFO) << "Masternode received relayed transaction fee: " << m_core.currency().formatAmount(amount) << " KRB";
		return true;
	}
	return false;
}

//
// Binary handlers
//

bool RpcServer::on_get_blocks(const COMMAND_RPC_GET_BLOCKS_FAST::request& req, COMMAND_RPC_GET_BLOCKS_FAST::response& res) {
  // TODO code duplication see InProcessNode::doGetNewBlocks()
  if (req.block_ids.empty()) {
    res.status = "Failed";
    return false;
  }

  if (req.block_ids.back() != m_core.getBlockIdByHeight(0)) {
    res.status = "Failed";
    return false;
  }

  uint32_t totalBlockCount;
  uint32_t startBlockIndex;
  std::vector<Crypto::Hash> supplement = m_core.findBlockchainSupplement(req.block_ids, COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT, totalBlockCount, startBlockIndex);

  res.current_height = totalBlockCount;
  res.start_height = startBlockIndex;

  for (const auto& blockId : supplement) {
    assert(m_core.have_block(blockId));
    auto completeBlock = m_core.getBlock(blockId);
    assert(completeBlock != nullptr);

    res.blocks.resize(res.blocks.size() + 1);
    res.blocks.back().block = Common::asString(toBinaryArray(completeBlock->getBlock()));

    res.blocks.back().txs.reserve(completeBlock->getTransactionCount());
    for (size_t i = 0; i < completeBlock->getTransactionCount(); ++i) {
      res.blocks.back().txs.push_back(Common::asString(toBinaryArray(completeBlock->getTransaction(i))));
    }
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_query_blocks(const COMMAND_RPC_QUERY_BLOCKS::request& req, COMMAND_RPC_QUERY_BLOCKS::response& res) {
  uint32_t startHeight;
  uint32_t currentHeight;
  uint32_t fullOffset;

  if (!m_core.queryBlocks(req.block_ids, req.timestamp, startHeight, currentHeight, fullOffset, res.items)) {
    res.status = "Failed to perform query";
    return false;
  }

  res.start_height = startHeight;
  res.current_height = currentHeight;
  res.full_offset = fullOffset;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_query_blocks_lite(const COMMAND_RPC_QUERY_BLOCKS_LITE::request& req, COMMAND_RPC_QUERY_BLOCKS_LITE::response& res) {
  uint32_t startHeight;
  uint32_t currentHeight;
  uint32_t fullOffset;
  if (!m_core.queryBlocksLite(req.blockIds, req.timestamp, startHeight, currentHeight, fullOffset, res.items)) {
    res.status = "Failed to perform query";
    return false;
  }

  res.startHeight = startHeight;
  res.currentHeight = currentHeight;
  res.fullOffset = fullOffset;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_indexes(const COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::request& req, COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES::response& res) {
  std::vector<uint32_t> outputIndexes;
  if (!m_core.get_tx_outputs_gindexs(req.txid, outputIndexes)) {
    res.status = "Failed";
    return true;
  }

  res.o_indexes.assign(outputIndexes.begin(), outputIndexes.end());
  res.status = CORE_RPC_STATUS_OK;
  logger(Logging::TRACE) << "COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES: [" << res.o_indexes.size() << "]";
  return true;
}

bool RpcServer::on_get_random_outs(const COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::request& req, COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::response& res) {
  res.status = "Failed";
  if (!m_core.get_random_outs_for_amounts(req, res)) {
    return true;
  }

  res.status = CORE_RPC_STATUS_OK;

  std::stringstream ss;
  typedef COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::outs_for_amount outs_for_amount;
  typedef COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS::out_entry out_entry;

  std::for_each(res.outs.begin(), res.outs.end(), [&](outs_for_amount& ofa)  {
    ss << "[" << ofa.amount << "]:";

    assert(ofa.outs.size() && "internal error: ofa.outs.size() is empty");

    std::for_each(ofa.outs.begin(), ofa.outs.end(), [&](out_entry& oe)
    {
      ss << oe.global_amount_index << " ";
    });
    ss << ENDL;
  });
  std::string s = ss.str();
  logger(Logging::TRACE) << "COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS: " << ENDL << s;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_pool_changes(const COMMAND_RPC_GET_POOL_CHANGES::request& req, COMMAND_RPC_GET_POOL_CHANGES::response& rsp) {
  rsp.status = CORE_RPC_STATUS_OK;
  std::vector<CryptoNote::Transaction> addedTransactions;
  rsp.isTailBlockActual = m_core.getPoolChanges(req.tailBlockId, req.knownTxsIds, addedTransactions, rsp.deletedTxsIds);
  for (auto& tx : addedTransactions) {
    BinaryArray txBlob;
    if (!toBinaryArray(tx, txBlob)) {
      rsp.status = "Internal error";
      break;;
    }

    rsp.addedTxs.emplace_back(std::move(txBlob));
  }
  return true;
}


bool RpcServer::on_get_pool_changes_lite(const COMMAND_RPC_GET_POOL_CHANGES_LITE::request& req, COMMAND_RPC_GET_POOL_CHANGES_LITE::response& rsp) {
  rsp.status = CORE_RPC_STATUS_OK;
  rsp.isTailBlockActual = m_core.getPoolChangesLite(req.tailBlockId, req.knownTxsIds, rsp.addedTxs, rsp.deletedTxsIds);

  return true;
}

bool RpcServer::on_get_blocks_details_by_heights(const COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HEIGHTS::request& req, COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HEIGHTS::response& rsp) {
  std::vector<BlockDetails> blockDetails;
  for (const uint32_t& height : req.blockHeights) {
    if (m_core.get_current_blockchain_height() <= height) {
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
        std::string("To big height: ") + std::to_string(height) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height() - 1) };
    }
    Crypto::Hash block_hash = m_core.getBlockIdByHeight(height);
    Block blk;
    if (!m_core.getBlockByHash(block_hash, blk)) {
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get block by height " + std::to_string(height) + '.' };
    }
    BlockDetails detail;
    if (!blockchainExplorerDataBuilder.fillBlockDetails(blk, detail)) {
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't fill block details." };
    }
    blockDetails.push_back(detail);
  }
  rsp.blocks = std::move(blockDetails);

  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_blocks_details_by_hashes(const COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HASHES::request& req, COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HASHES::response& rsp) {
  std::vector<BlockDetails> blockDetails;
  for (const Crypto::Hash& hash : req.blockHashes) {
    Block blk;
    if (!m_core.getBlockByHash(hash, blk)) {
      //throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get block by hash " + Common::PodToHex(hash) + '.' };
    }
    BlockDetails detail;
    if (!blockchainExplorerDataBuilder.fillBlockDetails(blk, detail)) {
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't fill block details." };
    }
    blockDetails.push_back(detail);
  }
  rsp.blocks = std::move(blockDetails);

  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_details_by_height(const COMMAND_RPC_GET_BLOCK_DETAILS_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCK_DETAILS_BY_HEIGHT::response& rsp) {
  BlockDetails blockDetails;
  if (m_core.get_current_blockchain_height() <= req.blockHeight) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(req.blockHeight) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height() - 1) };
  }
  Crypto::Hash block_hash = m_core.getBlockIdByHeight(req.blockHeight);
  Block blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by height " + std::to_string(req.blockHeight) + '.' };
}
  if (!blockchainExplorerDataBuilder.fillBlockDetails(blk, blockDetails)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't fill block details." };
  }
  rsp.block = blockDetails;

  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_details_by_hash(const COMMAND_RPC_GET_BLOCK_DETAILS_BY_HASH::request& req, COMMAND_RPC_GET_BLOCK_DETAILS_BY_HASH::response& rsp) {
  BlockDetails blockDetails;
  Crypto::Hash block_hash;
  if (!parse_hash256(req.hash, block_hash)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_WRONG_PARAM,
      "Failed to parse hex representation of block hash. Hex = " + req.hash + '.' };
  }
  Block blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by hash. Hash = " + req.hash + '.' };
  }
  if (!blockchainExplorerDataBuilder.fillBlockDetails(blk, blockDetails)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't fill block details." };
  }
  rsp.block = blockDetails;

  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_blocks_hashes_by_timestamps(const COMMAND_RPC_GET_BLOCKS_HASHES_BY_TIMESTAMPS::request& req, COMMAND_RPC_GET_BLOCKS_HASHES_BY_TIMESTAMPS::response& rsp) {
  uint32_t count;
  std::vector<Crypto::Hash> blockHashes;
  if (!m_core.get_blockchain_storage().getBlockIdsByTimestamp(req.timestampBegin, req.timestampEnd, req.limit, blockHashes, count)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get blocks within timestamps " + std::to_string(req.timestampBegin) + " - " + std::to_string(req.timestampEnd) + "." };
  }
  rsp.blockHashes = std::move(blockHashes);
  rsp.count = count;
  
  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transactions_details_by_hashes(const COMMAND_RPC_GET_TRANSACTIONS_DETAILS_BY_HASHES::request& req, COMMAND_RPC_GET_TRANSACTIONS_DETAILS_BY_HASHES::response& rsp) {
  std::vector<TransactionDetails> transactionsDetails;
  transactionsDetails.reserve(req.transactionHashes.size());

  std::list<Crypto::Hash> missed_txs;
  std::list<Transaction> txs;
  m_core.getTransactions(req.transactionHashes, txs, missed_txs, true);

  if (!txs.empty()) {
    for (const Transaction& tx : txs) {
      TransactionDetails txDetails;
      if (!blockchainExplorerDataBuilder.fillTransactionDetails(tx, txDetails)) {
        throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
          "Internal error: can't fill transaction details." };
      }
      transactionsDetails.push_back(txDetails);
    }

    rsp.transactions = std::move(transactionsDetails);
    rsp.status = CORE_RPC_STATUS_OK;
  }
  if (txs.empty() || !missed_txs.empty()) {
    std::ostringstream ss;
    std::string separator;
    for (auto h : missed_txs) {
      ss << separator << Common::podToHex(h);
      separator = ",";
    }
    rsp.status = "transaction(s) not found: " + ss.str() + ".";
  }

  return true;
}

bool RpcServer::on_get_transaction_details_by_hash(const COMMAND_RPC_GET_TRANSACTION_DETAILS_BY_HASH::request& req, COMMAND_RPC_GET_TRANSACTION_DETAILS_BY_HASH::response& rsp) {
  std::list<Crypto::Hash> missed_txs;
  std::list<Transaction> txs;
  std::vector<Crypto::Hash> hashes;
  hashes.push_back(req.hash);
  m_core.getTransactions(hashes, txs, missed_txs, true);

  if (txs.empty() || !missed_txs.empty()) {
    std::string hash_str = Common::podToHex(missed_txs.back());
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM,
      "transaction wasn't found. Hash = " + hash_str + '.' };
  }

  TransactionDetails transactionsDetails;
  if (!blockchainExplorerDataBuilder.fillTransactionDetails(txs.back(), transactionsDetails)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't fill transaction details." };
  }

  rsp.transaction = std::move(transactionsDetails);

  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transaction_hashes_by_paymentid(const COMMAND_RPC_GET_TRANSACTION_HASHES_BY_PAYMENT_ID::request& req, COMMAND_RPC_GET_TRANSACTION_HASHES_BY_PAYMENT_ID::response& rsp) {
  rsp.transactionHashes = m_core.getTransactionHashesByPaymentId(req.paymentId);

  rsp.status = CORE_RPC_STATUS_OK;
  return true;
}

//
// JSON handlers
//

bool RpcServer::on_get_info(const COMMAND_RPC_GET_INFO::request& req, COMMAND_RPC_GET_INFO::response& res) {
  res.height = m_core.get_current_blockchain_height();
  res.difficulty = m_core.getNextBlockDifficulty();
  res.transactions_count = m_core.get_blockchain_total_transactions() - res.height; //without coinbase
  res.transactions_pool_size = m_core.get_pool_transactions_count();
  res.alt_blocks_count = m_core.get_alternative_blocks_count();
  uint64_t total_conn = m_p2p.get_connections_count();
  res.outgoing_connections_count = m_p2p.get_outgoing_connections_count();
  res.incoming_connections_count = total_conn - res.outgoing_connections_count;
  res.rpc_connections_count = get_connections_count();
  res.white_peerlist_size = m_p2p.getPeerlistManager().get_white_peers_count();
  res.grey_peerlist_size = m_p2p.getPeerlistManager().get_gray_peers_count();
  res.last_known_block_index = std::max(static_cast<uint32_t>(1), m_protocolQuery.getObservedHeight()) - 1;
  Crypto::Hash last_block_hash = m_core.getBlockIdByHeight(res.height - 1);
  res.top_block_hash = Common::podToHex(last_block_hash);
  res.version = PROJECT_VERSION_LONG;
  res.fee_address = m_fee_address.empty() ? std::string() : m_fee_address;
  res.contact = m_contact_info.empty() ? std::string() : m_contact_info;
  res.min_fee = m_core.getMinimalFee();
  res.start_time = (uint64_t)m_core.getStartTime();

  uint64_t alreadyGeneratedCoins = m_core.getTotalGeneratedAmount();
  // that large uint64_t number is unsafe in JavaScript environment and therefore as a JSON value so we display it as a formatted string
  res.already_generated_coins = m_core.currency().formatAmount(alreadyGeneratedCoins);
  res.block_major_version = m_core.getCurrentBlockMajorVersion();
  std::vector<size_t> blocksSizes;
  if (!m_core.getBackwardBlocksSizes(res.height - 1, blocksSizes, parameters::CRYPTONOTE_REWARD_BLOCKS_WINDOW)) {
    return false;
  }
  uint64_t sizeMedian = Common::medianValue(blocksSizes);
  uint64_t nextReward = 0;
  int64_t emissionChange = 0;
  if (!m_core.getBlockReward(res.block_major_version, sizeMedian, 0, alreadyGeneratedCoins, 0, nextReward, emissionChange)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get already generated coins for prev. block." };
  }
  res.next_reward = nextReward;
  
  if (!m_core.getBlockCumulativeDifficulty(res.height - 1, res.cumulative_difficulty)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get last cumulative difficulty." };
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_height(const COMMAND_RPC_GET_HEIGHT::request& req, COMMAND_RPC_GET_HEIGHT::response& res) {
  res.height = m_core.get_current_blockchain_height();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transactions(const COMMAND_RPC_GET_TRANSACTIONS::request& req, COMMAND_RPC_GET_TRANSACTIONS::response& res) {
  std::vector<Crypto::Hash> vh;
  for (const auto& tx_hex_str : req.txs_hashes) {
    BinaryArray b;
    if (!Common::fromHex(tx_hex_str, b))
    {
      res.status = "Failed to parse hex representation of transaction hash";
      return true;
    }
    if (b.size() != sizeof(Crypto::Hash))
    {
      res.status = "Failed, size of data mismatch";
    }
    vh.push_back(*reinterpret_cast<const Crypto::Hash*>(b.data()));
  }
  std::list<Crypto::Hash> missed_txs;
  std::list<Transaction> txs;
  m_core.getTransactions(vh, txs, missed_txs);

  for (auto& tx : txs) {
    res.txs_as_hex.push_back(Common::toHex(toBinaryArray(tx)));
  }

  for (const auto& miss_tx : missed_txs) {
    res.missed_txs.push_back(Common::podToHex(miss_tx));
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_send_raw_transaction(const COMMAND_RPC_SEND_RAW_TRANSACTION::request& req, COMMAND_RPC_SEND_RAW_TRANSACTION::response& res) {
  BinaryArray tx_blob;
  if (!Common::fromHex(req.tx_as_hex, tx_blob))
  {
    logger(Logging::INFO) << "[on_send_raw_tx]: Failed to parse transaction from hexbuff: " << req.tx_as_hex;
    res.status = "Failed";
    return true;
  }

  Crypto::Hash transactionHash = Crypto::cn_fast_hash(tx_blob.data(), tx_blob.size());
  logger(Logging::DEBUGGING) << "transaction " << transactionHash << " came in on_send_raw_tx";

  tx_verification_context tvc = boost::value_initialized<tx_verification_context>();
  if (!m_core.handle_incoming_tx(tx_blob, tvc, false))
  {
    logger(Logging::INFO) << "[on_send_raw_tx]: Failed to process tx";
    res.status = "Failed";
    return true;
  }

  if (tvc.m_verification_failed)
  {
    logger(Logging::INFO) << "[on_send_raw_tx]: transaction verification failed";
    res.status = "Failed";
    return true;
  }

  if (!tvc.m_should_be_relayed)
  {
    logger(Logging::INFO) << "[on_send_raw_tx]: transaction accepted, but not relayed";
    res.status = "Not relayed";
    return true;
  }

  if (!m_fee_address.empty() && m_view_key != NULL_SECRET_KEY) {
	if (!checkIncomingTransactionForFee(tx_blob)) {
	  logger(Logging::INFO) << "Transaction not relayed due to lack of node fee";		
      res.status = "Not relayed due to lack of node fee";
      return true;
	}
  }

  NOTIFY_NEW_TRANSACTIONS::request r;
  r.txs.push_back(Common::asString(tx_blob));
  m_core.get_protocol()->relay_transactions(r);
  //TODO: make sure that tx has reached other nodes here, probably wait to receive reflections from other nodes
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_start_mining(const COMMAND_RPC_START_MINING::request& req, COMMAND_RPC_START_MINING::response& res) {
  if (m_restricted_rpc) {
	res.status = "Failed, restricted handle";
	return false;
  }
  
  AccountPublicAddress adr;
  if (!m_core.currency().parseAccountAddressString(req.miner_address, adr)) {
    res.status = "Failed, wrong address";
    return true;
  }

  if (!m_core.get_miner().start(adr, static_cast<size_t>(req.threads_count))) {
    res.status = "Failed, mining not started";
    return true;
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_stop_mining(const COMMAND_RPC_STOP_MINING::request& req, COMMAND_RPC_STOP_MINING::response& res) {
  if (m_restricted_rpc) {
	res.status = "Failed, restricted handle";
	return false;
  }

  if (!m_core.get_miner().stop()) {
    res.status = "Failed, mining not stopped";
    return true;
  }
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_stop_daemon(const COMMAND_RPC_STOP_DAEMON::request& req, COMMAND_RPC_STOP_DAEMON::response& res) {
  if (m_restricted_rpc) {
	res.status = "Failed, restricted handle";
	return false;
  }
  if (m_core.currency().isTestnet()) {
    m_p2p.sendStopSignal();
    res.status = CORE_RPC_STATUS_OK;
  } else {
    res.status = CORE_RPC_ERROR_CODE_INTERNAL_ERROR;
    return false;
  }
  return true;
}

bool RpcServer::on_get_fee_address(const COMMAND_RPC_GET_FEE_ADDRESS::request& req, COMMAND_RPC_GET_FEE_ADDRESS::response& res) {
  if (m_fee_address.empty()) {
	res.status = CORE_RPC_STATUS_OK;
	return false; 
  }
  res.fee_address = m_fee_address;
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_peer_list(const COMMAND_RPC_GET_PEER_LIST::request& req, COMMAND_RPC_GET_PEER_LIST::response& res) {
	std::list<PeerlistEntry> pl_wite;
	std::list<PeerlistEntry> pl_gray;
	m_p2p.getPeerlistManager().get_peerlist_full(pl_gray, pl_wite);
	for (const auto& pe : pl_wite) {
		std::stringstream ss;
		ss << pe.adr;
		res.peers.push_back(ss.str());
	}
	res.status = CORE_RPC_STATUS_OK;
	return true;
}

bool RpcServer::on_get_payment_id(const COMMAND_RPC_GEN_PAYMENT_ID::request& req, COMMAND_RPC_GEN_PAYMENT_ID::response& res) {
  Crypto::Hash result;
  Random::randomBytes(32, result.data);
  res.payment_id = Common::podToHex(result);
  return true;
}

//------------------------------------------------------------------------------------------------------------------------------
// JSON RPC methods
//------------------------------------------------------------------------------------------------------------------------------

bool RpcServer::on_blocks_list_json(const COMMAND_RPC_GET_BLOCKS_LIST::request& req, COMMAND_RPC_GET_BLOCKS_LIST::response& res) {
  if (m_core.get_current_blockchain_height() <= req.height) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(req.height) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height()) };
  }

  uint32_t print_blocks_count = 10;
  if(req.count <= 1000)
    print_blocks_count = req.count;
  
  uint32_t last_height = req.height - print_blocks_count;
  if (req.height <= print_blocks_count)  {
    last_height = 0;
  }

  for (uint32_t i = req.height; i >= last_height; i--) {
    Crypto::Hash block_hash = m_core.getBlockIdByHeight(i);
    Block blk;
    if (!m_core.getBlockByHash(block_hash, blk)) {
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
        "Internal error: can't get block by height. Height = " + std::to_string(i) + '.' };
    }

    size_t tx_cumulative_block_size;
    m_core.getBlockSize(block_hash, tx_cumulative_block_size);
    size_t blokBlobSize = getObjectBinarySize(blk);
    size_t minerTxBlobSize = getObjectBinarySize(blk.baseTransaction);
    difficulty_type blockDiff;
    m_core.getBlockDifficulty(static_cast<uint32_t>(i), blockDiff);

    block_short_response block_short;
    block_short.timestamp = blk.timestamp;
    block_short.height = i;
    block_short.hash = Common::podToHex(block_hash);
    block_short.cumulative_size = blokBlobSize + tx_cumulative_block_size - minerTxBlobSize;
    block_short.transactions_count = blk.transactionHashes.size() + 1;
    block_short.difficulty = blockDiff;
    block_short.min_fee = m_core.getMinimalFeeForHeight(i);

    res.blocks.push_back(block_short);

    if (i == 0)
      break;
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transactions_pool(const COMMAND_RPC_GET_TRANSACTIONS_POOL::request& req, COMMAND_RPC_GET_TRANSACTIONS_POOL::response& res) {
  auto pool = m_core.getMemoryPool();
  for (const CryptoNote::tx_memory_pool::TransactionDetails txd : pool) {
    transaction_pool_response mempool_transaction;
    mempool_transaction.hash = Common::podToHex(txd.id);
    mempool_transaction.fee = txd.fee;
    mempool_transaction.amount_out = getOutputAmount(txd.tx);;
    mempool_transaction.size = txd.blobSize;
    mempool_transaction.receive_time = txd.receiveTime;
    res.transactions.push_back(mempool_transaction);
  }
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_transactions_by_payment_id(const COMMAND_RPC_GET_TRANSACTIONS_BY_PAYMENT_ID::request& req, COMMAND_RPC_GET_TRANSACTIONS_BY_PAYMENT_ID::response& res) {
	if (!req.payment_id.size()) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong parameters, expected payment_id" };
	}
	logger(Logging::DEBUGGING, Logging::WHITE) << "RPC request came: Search by Payment ID: " << req.payment_id;

	Crypto::Hash paymentId;
	std::vector<Transaction> transactions;

	if (!parse_hash256(req.payment_id, paymentId)) {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"Failed to parse Payment ID: " + req.payment_id + '.' };
	}

	if (!m_core.getTransactionsByPaymentId(paymentId, transactions)) {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
			"Internal error: can't get transactions by Payment ID: " + req.payment_id + '.' };
	}

	for (const Transaction& tx : transactions) {
		transaction_short_response transaction_short;
		uint64_t amount_in = 0;
		get_inputs_money_amount(tx, amount_in);
		uint64_t amount_out = get_outs_money_amount(tx);

		transaction_short.hash = Common::podToHex(getObjectHash(tx));
		transaction_short.fee = amount_in - amount_out;
		transaction_short.amount_out = amount_out;
		transaction_short.size = getObjectBinarySize(tx);
		res.transactions.push_back(transaction_short);
	}

	res.status = CORE_RPC_STATUS_OK;
	return true;
}

bool RpcServer::on_getblockcount(const COMMAND_RPC_GETBLOCKCOUNT::request& req, COMMAND_RPC_GETBLOCKCOUNT::response& res) {
  res.count = m_core.get_current_blockchain_height();
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_getblockhash(const COMMAND_RPC_GETBLOCKHASH::request& req, COMMAND_RPC_GETBLOCKHASH::response& res) {
  if (req.size() != 1) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong parameters, expected height" };
  }

  uint32_t h = static_cast<uint32_t>(req[0]);
  Crypto::Hash blockId = m_core.getBlockIdByHeight(h);
  if (blockId == NULL_HASH) {
    throw JsonRpc::JsonRpcError{ 
      CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(h) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height())
    };
  }

  res = Common::podToHex(blockId);
  return true;
}

namespace {
  uint64_t slow_memmem(void* start_buff, size_t buflen, void* pat, size_t patlen)
  {
    void* buf = start_buff;
    void* end = (char*)buf + buflen - patlen;
    while ((buf = memchr(buf, ((char*)pat)[0], buflen)))
    {
      if (buf>end)
        return 0;
      if (memcmp(buf, pat, patlen) == 0)
        return (char*)buf - (char*)start_buff;
      buf = (char*)buf + 1;
    }
    return 0;
  }
}

bool RpcServer::on_getblocktemplate(const COMMAND_RPC_GETBLOCKTEMPLATE::request& req, COMMAND_RPC_GETBLOCKTEMPLATE::response& res) {
  if (req.reserve_size > TX_EXTRA_NONCE_MAX_COUNT) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_RESERVE_SIZE, "To big reserved size, maximum 255" };
  }

  AccountPublicAddress acc = boost::value_initialized<AccountPublicAddress>();

  if (!req.wallet_address.size() || !m_core.currency().parseAccountAddressString(req.wallet_address, acc)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_WALLET_ADDRESS, "Failed to parse wallet address" };
  }

  Block b = boost::value_initialized<Block>();
  CryptoNote::BinaryArray blob_reserve;
  blob_reserve.resize(req.reserve_size, 0);
  if (!m_core.get_block_template(b, acc, res.difficulty, res.height, blob_reserve)) {
    logger(Logging::ERROR) << "Failed to create block template";
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
  }

  BinaryArray block_blob = toBinaryArray(b);
  Crypto::PublicKey tx_pub_key = CryptoNote::getTransactionPublicKeyFromExtra(b.baseTransaction.extra);
  if (tx_pub_key == NULL_PUBLIC_KEY) {
    logger(Logging::ERROR) << "Failed to find tx pub key in coinbase extra";
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to find tx pub key in coinbase extra" };
  }

  if (0 < req.reserve_size) {
    res.reserved_offset = slow_memmem((void*)block_blob.data(), block_blob.size(), &tx_pub_key, sizeof(tx_pub_key));
    if (!res.reserved_offset) {
      logger(Logging::ERROR) << "Failed to find tx pub key in blockblob";
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
    }
    res.reserved_offset += sizeof(tx_pub_key) + 3; //3 bytes: tag for TX_EXTRA_TAG_PUBKEY(1 byte), tag for TX_EXTRA_NONCE(1 byte), counter in TX_EXTRA_NONCE(1 byte)
    if (res.reserved_offset + req.reserve_size > block_blob.size()) {
      logger(Logging::ERROR) << "Failed to calculate offset for reserved bytes";
      throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to create block template" };
    }
  } else {
    res.reserved_offset = 0;
  }

  BinaryArray hashing_blob;
  if (!get_block_hashing_blob(b, hashing_blob)) {
	  logger(Logging::ERROR) << "Failed to get blockhashing_blob";
	  throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: failed to get blockhashing_blob" };
  }

  res.blocktemplate_blob = Common::toHex(block_blob);
  res.blockhashing_blob = Common::toHex(hashing_blob);
  res.status = CORE_RPC_STATUS_OK;

  return true;
}

bool RpcServer::on_get_currency_id(const COMMAND_RPC_GET_CURRENCY_ID::request& /*req*/, COMMAND_RPC_GET_CURRENCY_ID::response& res) {
  Crypto::Hash currencyId = m_core.currency().genesisBlockHash();
  res.currency_id_blob = Common::podToHex(currencyId);
  return true;
}

bool RpcServer::on_submitblock(const COMMAND_RPC_SUBMITBLOCK::request& req, COMMAND_RPC_SUBMITBLOCK::response& res) {
  if (req.size() != 1) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Wrong param" };
  }

  BinaryArray blockblob;
  if (!Common::fromHex(req[0], blockblob)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_BLOCKBLOB, "Wrong block blob" };
  }

  block_verification_context bvc = boost::value_initialized<block_verification_context>();

  m_core.handle_incoming_block_blob(blockblob, bvc, true, true);

  if (!bvc.m_added_to_main_chain) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_BLOCK_NOT_ACCEPTED, "Block not accepted" };
  }

  res.status = CORE_RPC_STATUS_OK;
  return true;
}


namespace {
  uint64_t get_block_reward(const Block& blk) {
    uint64_t reward = 0;
    for (const TransactionOutput& out : blk.baseTransaction.outputs) {
      reward += out.amount;
    }
    return reward;
  }
}

void RpcServer::fill_block_header_response(const Block& blk, bool orphan_status, uint32_t height, const Crypto::Hash& hash, block_header_response& responce) {
  responce.major_version = blk.majorVersion;
  responce.minor_version = blk.minorVersion;
  responce.timestamp = blk.timestamp;
  responce.prev_hash = Common::podToHex(blk.previousBlockHash);
  responce.nonce = blk.nonce;
  responce.orphan_status = orphan_status;
  responce.height = height;
  responce.depth = m_core.get_current_blockchain_height() - height - 1;
  responce.hash = Common::podToHex(hash);
  m_core.getBlockDifficulty(static_cast<uint32_t>(height), responce.difficulty);
  responce.reward = get_block_reward(blk);
}

bool RpcServer::on_get_last_block_header(const COMMAND_RPC_GET_LAST_BLOCK_HEADER::request& req, COMMAND_RPC_GET_LAST_BLOCK_HEADER::response& res) {
  uint32_t last_block_height;
  Crypto::Hash last_block_hash;
  
  m_core.get_blockchain_top(last_block_height, last_block_hash);

  Block last_block;
  if (!m_core.getBlockByHash(last_block_hash, last_block)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Internal error: can't get last block hash." };
  }
  Crypto::Hash tmp_hash = m_core.getBlockIdByHeight(last_block_height);
  bool is_orphaned = last_block_hash != tmp_hash;
  fill_block_header_response(last_block, is_orphaned, last_block_height, last_block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_header_by_hash(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH::response& res) {
  Crypto::Hash block_hash;

  if (!parse_hash256(req.hash, block_hash)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_WRONG_PARAM,
      "Failed to parse hex representation of block hash. Hex = " + req.hash + '.' };
  }

  Block blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by hash. Hash = " + req.hash + '.' };
  }

  if (blk.baseTransaction.inputs.front().type() != typeid(BaseInput)) {
    throw JsonRpc::JsonRpcError{
      CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: coinbase transaction in the block has the wrong type" };
  }

  uint32_t block_height = boost::get<BaseInput>(blk.baseTransaction.inputs.front()).blockIndex;
  Crypto::Hash tmp_hash = m_core.getBlockIdByHeight(block_height);
  bool is_orphaned = block_hash != tmp_hash;
  fill_block_header_response(blk, is_orphaned, block_height, block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_get_block_header_by_height(const COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::request& req, COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT::response& res) {
  if (m_core.get_current_blockchain_height() <= req.height) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_TOO_BIG_HEIGHT,
      std::string("To big height: ") + std::to_string(req.height) + ", current blockchain height = " + std::to_string(m_core.get_current_blockchain_height()) };
  }

  Crypto::Hash block_hash = m_core.getBlockIdByHeight(static_cast<uint32_t>(req.height));
  Block blk;
  if (!m_core.getBlockByHash(block_hash, blk)) {
    throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR,
      "Internal error: can't get block by height. Height = " + std::to_string(req.height) + '.' };
  }
  
  Crypto::Hash tmp_hash = m_core.getBlockIdByHeight(req.height);
  bool is_orphaned = block_hash != tmp_hash;
  fill_block_header_response(blk, is_orphaned, req.height, block_hash, res.block_header);
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_check_transaction_key(const COMMAND_RPC_CHECK_TRANSACTION_KEY::request& req, COMMAND_RPC_CHECK_TRANSACTION_KEY::response& res) {
	// parse txid
	Crypto::Hash txid;
	if (!parse_hash256(req.transaction_id, txid)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse txid" };
	}
	// parse address
	CryptoNote::AccountPublicAddress address;
	if (!m_core.currency().parseAccountAddressString(req.address, address)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse address " + req.address + '.' };
	}
	// parse txkey
	Crypto::Hash tx_key_hash;
	size_t size;
	if (!Common::fromHex(req.transaction_key, &tx_key_hash, sizeof(tx_key_hash), size) || size != sizeof(tx_key_hash)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse txkey" };
	}
	Crypto::SecretKey tx_key = *(struct Crypto::SecretKey *) &tx_key_hash;

	// fetch tx
	Transaction tx;
	std::vector<Crypto::Hash> tx_ids;
	tx_ids.push_back(txid);
	std::list<Crypto::Hash> missed_txs;
	std::list<Transaction> txs;
	m_core.getTransactions(tx_ids, txs, missed_txs, true);

	if (1 == txs.size()) {
		tx = txs.front();
	}
	else {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"Couldn't find transaction with hash: " + req.transaction_id + '.' };
	}
	CryptoNote::TransactionPrefix transaction = *static_cast<const TransactionPrefix*>(&tx);

	// obtain key derivation
	Crypto::KeyDerivation derivation;
	if (!Crypto::generate_key_derivation(address.viewPublicKey, tx_key, derivation))
	{
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to generate key derivation from supplied parameters" };
	}
	
	// look for outputs
	uint64_t received(0);
	size_t keyIndex(0);
	std::vector<TransactionOutput> outputs;
	try {
		for (const TransactionOutput& o : transaction.outputs) {
			if (o.target.type() == typeid(KeyOutput)) {
				const KeyOutput out_key = boost::get<KeyOutput>(o.target);
				Crypto::PublicKey pubkey;
				derive_public_key(derivation, keyIndex, address.spendPublicKey, pubkey);
				if (pubkey == out_key.key) {
					received += o.amount;
					outputs.push_back(o);
				}
			}
			++keyIndex;
		}
	}
	catch (...)
	{
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Unknown error" };
	}
	res.amount = received;
	res.outputs = outputs;
	res.status = CORE_RPC_STATUS_OK;
	return true;
}

bool RpcServer::on_check_transaction_with_view_key(const COMMAND_RPC_CHECK_TRANSACTION_WITH_PRIVATE_VIEW_KEY::request& req, COMMAND_RPC_CHECK_TRANSACTION_WITH_PRIVATE_VIEW_KEY::response& res) {
	// parse txid
	Crypto::Hash txid;
	if (!parse_hash256(req.transaction_id, txid)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse txid" };
	}
	// parse address
	CryptoNote::AccountPublicAddress address;
	if (!m_core.currency().parseAccountAddressString(req.address, address)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse address " + req.address + '.' };
	}
	// parse view key
	Crypto::Hash view_key_hash;
	size_t size;
	if (!Common::fromHex(req.view_key, &view_key_hash, sizeof(view_key_hash), size) || size != sizeof(view_key_hash)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse private view key" };
	}
	Crypto::SecretKey viewKey = *(struct Crypto::SecretKey *) &view_key_hash;

	// fetch tx
	Transaction tx;
	std::vector<Crypto::Hash> tx_ids;
	tx_ids.push_back(txid);
	std::list<Crypto::Hash> missed_txs;
	std::list<Transaction> txs;
	m_core.getTransactions(tx_ids, txs, missed_txs, true);

	if (1 == txs.size()) {
		tx = txs.front();
	}
	else {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"Couldn't find transaction with hash: " + req.transaction_id + '.' };
	}
	CryptoNote::TransactionPrefix transaction = *static_cast<const TransactionPrefix*>(&tx);
	
	// get tx pub key
	Crypto::PublicKey txPubKey = getTransactionPublicKeyFromExtra(transaction.extra);

	// obtain key derivation
	Crypto::KeyDerivation derivation;
	if (!Crypto::generate_key_derivation(txPubKey, viewKey, derivation))
	{
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to generate key derivation from supplied parameters" };
	}

	// look for outputs
	uint64_t received(0);
	size_t keyIndex(0);
	std::vector<TransactionOutput> outputs;
	try {
		for (const TransactionOutput& o : transaction.outputs) {
			if (o.target.type() == typeid(KeyOutput)) {
				const KeyOutput out_key = boost::get<KeyOutput>(o.target);
				Crypto::PublicKey pubkey;
				derive_public_key(derivation, keyIndex, address.spendPublicKey, pubkey);
				if (pubkey == out_key.key) {
					received += o.amount;
					outputs.push_back(o);
				}
			}
			++keyIndex;
		}
	}
	catch (...)
	{
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Unknown error" };
	}
	res.amount = received;
	res.outputs = outputs;
	
	Crypto::Hash blockHash;
	uint32_t blockHeight;
	if (m_core.getBlockContainingTx(txid, blockHash, blockHeight)) {
		res.confirmations = m_protocolQuery.getObservedHeight() - blockHeight;
	}

	res.status = CORE_RPC_STATUS_OK;
	return true;
}

bool RpcServer::on_check_transaction_proof(const COMMAND_RPC_CHECK_TRANSACTION_PROOF::request& req, COMMAND_RPC_CHECK_TRANSACTION_PROOF::response& res) {
	// parse txid
	Crypto::Hash txid;
	if (!parse_hash256(req.transaction_id, txid)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse txid" };
	}
	// parse address
	CryptoNote::AccountPublicAddress address;
	if (!m_core.currency().parseAccountAddressString(req.destination_address, address)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse address " + req.destination_address + '.' };
	}
	// parse pubkey r*A & signature
	const size_t header_len = strlen("ProofV1");
	if (req.signature.size() < header_len || req.signature.substr(0, header_len) != "ProofV1") {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Signature header check error" };
	}
	Crypto::PublicKey rA;
	Crypto::Signature sig;
	const size_t rA_len = Tools::Base58::encode(std::string((const char *)&rA, sizeof(Crypto::PublicKey))).size();
	const size_t sig_len = Tools::Base58::encode(std::string((const char *)&sig, sizeof(Crypto::Signature))).size();
	std::string rA_decoded;
	std::string sig_decoded;
	if (!Tools::Base58::decode(req.signature.substr(header_len, rA_len), rA_decoded)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Signature decoding error" };
	}
	if (!Tools::Base58::decode(req.signature.substr(header_len + rA_len, sig_len), sig_decoded)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Signature decoding error" };
	}
	if (sizeof(Crypto::PublicKey) != rA_decoded.size() || sizeof(Crypto::Signature) != sig_decoded.size()) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Signature decoding error" };
	}
	memcpy(&rA, rA_decoded.data(), sizeof(Crypto::PublicKey));
	memcpy(&sig, sig_decoded.data(), sizeof(Crypto::Signature));

	// fetch tx pubkey
	Transaction tx;

	std::vector<uint32_t> out;
	std::vector<Crypto::Hash> tx_ids;
	tx_ids.push_back(txid);
	std::list<Crypto::Hash> missed_txs;
	std::list<Transaction> txs;
	m_core.getTransactions(tx_ids, txs, missed_txs, true);

	if (1 == txs.size()) {
		tx = txs.front();
	}
	else {
		throw JsonRpc::JsonRpcError{
			CORE_RPC_ERROR_CODE_WRONG_PARAM,
			"transaction wasn't found. Hash = " + req.transaction_id + '.' };
	}
	CryptoNote::TransactionPrefix transaction = *static_cast<const TransactionPrefix*>(&tx);

	Crypto::PublicKey R = getTransactionPublicKeyFromExtra(transaction.extra);
	if (R == NULL_PUBLIC_KEY)
	{
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Tx pubkey was not found" };
	}

	// check signature
	bool r = Crypto::check_tx_proof(txid, R, address.viewPublicKey, rA, sig);
	res.signature_valid = r;

	if (r) {

		// obtain key derivation by multiplying scalar 1 to the pubkey r*A included in the signature
		Crypto::KeyDerivation derivation;
		if (!Crypto::generate_key_derivation(rA, I, derivation)) {
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Failed to generate key derivation" };
		}

		// look for outputs
		uint64_t received(0);
		size_t keyIndex(0);
		std::vector<TransactionOutput> outputs;
		try {
			for (const TransactionOutput& o : transaction.outputs) {
				if (o.target.type() == typeid(KeyOutput)) {
					const KeyOutput out_key = boost::get<KeyOutput>(o.target);
					Crypto::PublicKey pubkey;
					derive_public_key(derivation, keyIndex, address.spendPublicKey, pubkey);
					if (pubkey == out_key.key) {
						received += o.amount;
						outputs.push_back(o);
					}
				}
				++keyIndex;
			}
		}
		catch (...)
		{
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Unknown error" };
		}
		res.received_amount = received;
		res.outputs = outputs;

		Crypto::Hash blockHash;
		uint32_t blockHeight;
		if (m_core.getBlockContainingTx(txid, blockHash, blockHeight)) {
			res.confirmations = m_protocolQuery.getObservedHeight() - blockHeight;
		}
	}
	else {
		res.received_amount = 0;
	}

	res.status = CORE_RPC_STATUS_OK;
	return true;
}

bool RpcServer::on_check_reserve_proof(const COMMAND_RPC_CHECK_RESERVE_PROOF::request& req, COMMAND_RPC_CHECK_RESERVE_PROOF::response& res) {
	// parse address
	CryptoNote::AccountPublicAddress address;
	if (!m_core.currency().parseAccountAddressString(req.address, address)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_WRONG_PARAM, "Failed to parse address " + req.address + '.' };
	}
	
	// parse sugnature
	const char header[] = "ReserveProofV1";
	const size_t header_len = strlen(header);
	if (req.signature.size() < header_len || req.signature.substr(0, header_len) != header) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Signature header check error" };
	}

	std::string sig_decoded;
	if (!Tools::Base58::decode(req.signature.substr(header_len), sig_decoded)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Signature decoding error" };
	}

	BinaryArray ba;
	if (!Common::fromHex(sig_decoded, ba)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Proof decoding error" };
	}
	reserve_proof proof_decoded;
	if (!fromBinaryArray(proof_decoded, ba)) {
		throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "BinaryArray decoding error" };
	}

	std::vector<reserve_proof_entry>& proofs = proof_decoded.proofs;
	
	// compute signature prefix hash
	std::string prefix_data = req.message;
	prefix_data.append((const char*)&address, sizeof(CryptoNote::AccountPublicAddress));
	for (size_t i = 0; i < proofs.size(); ++i) {
		prefix_data.append((const char*)&proofs[i].key_image, sizeof(Crypto::PublicKey));
	}
	Crypto::Hash prefix_hash;
	Crypto::cn_fast_hash(prefix_data.data(), prefix_data.size(), prefix_hash);

	// fetch txes
	std::vector<Crypto::Hash> transactionHashes;
	for (size_t i = 0; i < proofs.size(); ++i) {
		transactionHashes.push_back(proofs[i].transaction_id);
	}
	std::list<Crypto::Hash> missed_txs;
	std::list<Transaction> txs;
	m_core.getTransactions(transactionHashes, txs, missed_txs);
	std::vector<Transaction> transactions;
	std::copy(txs.begin(), txs.end(), std::inserter(transactions, transactions.end()));

	// check spent status
	res.total = 0;
	res.spent = 0;
	res.locked = 0;
	for (size_t i = 0; i < proofs.size(); ++i) {
		const reserve_proof_entry& proof = proofs[i];

		CryptoNote::TransactionPrefix tx = *static_cast<const TransactionPrefix*>(&transactions[i]);
    
		bool unlocked = m_core.is_tx_spendtime_unlocked(tx.unlockTime, req.height);

		if (proof.index_in_transaction >= tx.outputs.size()) {
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "index_in_tx is out of bound" };
		}

		const KeyOutput out_key = boost::get<KeyOutput>(tx.outputs[proof.index_in_transaction].target);

		// get tx pub key
		Crypto::PublicKey txPubKey = getTransactionPublicKeyFromExtra(tx.extra);

		// check singature for shared secret
		if (!Crypto::check_tx_proof(prefix_hash, address.viewPublicKey, txPubKey, proof.shared_secret, proof.shared_secret_sig)) {
			//throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Failed to check singature for shared secret" };
			res.good = false;
			return true;
		}

		// check signature for key image
		const std::vector<const Crypto::PublicKey *>& pubs = { &out_key.key };
		if (!Crypto::check_ring_signature(prefix_hash, proof.key_image, &pubs[0], 1, &proof.key_image_sig)) {
			//throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Failed to check signature for key image" };
			res.good = false;
			return true;
		}

		// check if the address really received the fund
		Crypto::KeyDerivation derivation;
		if (!Crypto::generate_key_derivation(proof.shared_secret, I, derivation)) {
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Failed to generate key derivation" };
		}
		try {
			Crypto::PublicKey pubkey;
			derive_public_key(derivation, proof.index_in_transaction, address.spendPublicKey, pubkey);
			if (pubkey == out_key.key) {
				uint64_t amount = tx.outputs[proof.index_in_transaction].amount;
				res.total += amount;

        if (!unlocked) {
          res.locked += amount;
        }

        if (req.height != 0) {
          if (m_core.is_key_image_spent(proof.key_image, req.height)) {
            res.spent += amount;
          }
        } else {
          if (m_core.is_key_image_spent(proof.key_image)) {
            res.spent += amount;
          }
				}
			}
		}
		catch (...)
		{
			throw JsonRpc::JsonRpcError{ CORE_RPC_ERROR_CODE_INTERNAL_ERROR, "Unknown error" };
		}
		
	}

	// check signature for address spend keys
	Crypto::Signature sig = proof_decoded.signature;
	if (!Crypto::check_signature(prefix_hash, address.spendPublicKey, sig)) {
		res.good = false;
		return true;
	}

	res.good = true;

	return true;
}

bool RpcServer::on_validate_address(const COMMAND_RPC_VALIDATE_ADDRESS::request& req, COMMAND_RPC_VALIDATE_ADDRESS::response& res) {
  AccountPublicAddress acc = boost::value_initialized<AccountPublicAddress>();
  bool r = m_core.currency().parseAccountAddressString(req.address, acc);
  res.is_valid = r;
  if (r) {
    res.address = m_core.currency().accountAddressAsString(acc);
    res.spend_public_key = Common::podToHex(acc.spendPublicKey);
    res.view_public_key = Common::podToHex(acc.viewPublicKey);
  }
  res.status = CORE_RPC_STATUS_OK;
  return true;
}

bool RpcServer::on_verify_message(const COMMAND_RPC_VERIFY_MESSAGE::request& req, COMMAND_RPC_VERIFY_MESSAGE::response& res) {
	Crypto::Hash hash;
	Crypto::cn_fast_hash(req.message.data(), req.message.size(), hash);

	AccountPublicAddress acc = boost::value_initialized<AccountPublicAddress>();
	if (!m_core.currency().parseAccountAddressString(req.address, acc)) {
		throw JsonRpc::JsonRpcError(CORE_RPC_ERROR_CODE_WRONG_PARAM, std::string("Failed to parse address"));
	}

	const size_t header_len = strlen("SigV1");
	if (req.signature.size() < header_len || req.signature.substr(0, header_len) != "SigV1") {
		throw JsonRpc::JsonRpcError(CORE_RPC_ERROR_CODE_WRONG_PARAM, std::string("Signature header check error"));
	}
	std::string decoded;
	Crypto::Signature s;
	if (!Tools::Base58::decode(req.signature.substr(header_len), decoded) || sizeof(s) != decoded.size()) {
		throw JsonRpc::JsonRpcError(CORE_RPC_ERROR_CODE_WRONG_PARAM, std::string("Signature decoding error"));
		return false;
	}
	memcpy(&s, decoded.data(), sizeof(s));
	res.sig_valid = Crypto::check_signature(hash, acc.spendPublicKey, s);
	res.status = CORE_RPC_STATUS_OK;
	return true;
}

}
