// Copyright (c) 2018, The TurtleCoin Developers
// Copyright (c) 2018-2019, The Karbo Developers
// 
// Please see the included LICENSE file for more information.

#include <GreenWallet/Types.h>

void addToAddressBook();

void sendFromAddressBook(std::shared_ptr<WalletInfo> walletInfo,
                         uint32_t height, std::string feeAddress);

void deleteFromAddressBook();

void listAddressBook();

const Maybe<std::string> getAddressBookPaymentID();

const Maybe<const std::string> getAddressBookAddress();

const Maybe<const AddressBookEntry> getAddressBookEntry(AddressBook addressBook);

const std::string getAddressBookName(AddressBook addressBook);

AddressBook getAddressBook(const bool &is_sys_dir,
                           const std::string &default_data_dir);

bool saveAddressBook(AddressBook addressBook,
                     const bool &is_sys_dir,
                     const std::string &default_data_dir);

bool isAddressBookEmpty(AddressBook addressBook);
