This page is a list of known issues in the qt 1.0.12 wallet.

1.) Legacy transaction data - the legacy transaction data (from previous version of the Qt wallet) was not complete and thus
the new wallet cannot reliable present this data in the transaction tab. With the limited and incomplete data, we do our best
to reconstruct the transaction history, but there will be some things that are not correct. In some cases, the confirmation
flag will be set incorrectly for some transactions. For new transactions, the transaction tab will be reliable.

2.) When clicking on a transaction, there will be a warning that says "Wallet failed to verify data. The above is from a local
cache and may be invalid!". This error happens every time and can be ignored.

3.) When clicking on a transaction in the transaction tab, the index displayed in the modal window will be one less than what
is displayed in the transaction table.
