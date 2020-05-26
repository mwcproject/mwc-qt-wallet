

Transaction fees in MWC are calculated using the following algorithm:

    base_fee * max(1, 4*numOutputs + numKernels - numInputs)


#base_fee:
    1000000 nanocoins

#numOutputs:
    Will always be at least 1 as there is one output for the receiver of the coin.
    numOutputs will include the configured change outputs, if the sender will get some coins in change.
    So when spending all "spendable" coins in a wallet, numOutputs will be 1. It is also possible to spend some number of outputs which exactly equal the amount to spend plus the transaction fee. In this scenario, numOutputs will also be one. Only when the sender should expect change, will numOutputs include the change outputs setting.

#numKernels:
    Always 1 and cannot be changed at this time.

#numInputs:
    Calculated using the spendable outputs from the wallet. The transaction fee is calculated so that you will be charged a lower fee, if more inputs are used. So the spendable outputs are sorted in ascending order by coin amount prior to selecting the outputs to spend. Then the outputs which cover the amount to spend plus the fee will be used in the transaction.

    If numInputs is >= 4*(numOutputs) + 1, the base_fee is used as that is the minimum transaction fee allowed.


For example, suppose yoou have the following spendable outputs in your wallet:
        Tx 50, Amt 3.985
        Tx 49, Amt 4.7
        Tx 48, Amt 1.0

    If you want to send 2 coins, Tx 48 and Tx 50 will be used in the transaction so numInputs will be 2.
    Using the default configured change outputs of 1, numOutputs will be 2 because there will be 1 output for the receiver, and 1 output for the sender's change.
    The transaction fee will be:
        (4*(1 + 1) + 1 - 2) * 1000000 = 7000000

    This fee is then converted from nanocoins so the user will see that the fee is: 0.007

HODL exception to above:
    For HODL, the outputs to use for a transaction are selected so that the fewest number of outputs is used and you don't end up spending more HODL outputs than necessary. So in the example above if all of the spendable outputs were HODL outputs, Tx 50 would be chosen as the transaction input so only 1 output needs to be used to send 2 coins. In this case the transaction fee would be:
        (4*(1 + 1) + 1 - 1) * 1000000 = 8000000

    When this fee is converted from nanocoin, the user will see 0.008 as the transaction fee.

#Spendable Outputs:
    The transaction fee uses only spendable outputs. It will not use outputs which are locked or which haven't been fully confirmed.
    The wallet "Number of confirmations" setting is used to see if an output is spendable.

#Confirmation Screen:
    Transaction Fee is only displayed on the confirmation screens for sending online and offline using a file. It is not displayed for account to account fund transfers as there is not an existing confirmation screen for that scenario.

#Notes:
    Before a transaction is sent, there was already in place code to retrieve outputs for the send transaction when there are HODL outputs or "Manual output Locking from spending" is enabled. The outputs to use in this case have to be retrieved by Qt Wallet as only Qt Wallet has information on HODL or output locking. This existing code was updated to also calculate the txn fee based upon the outputs it selects for use in the transaction.

    When this existing code does not find any outputs, it means that there are no HODL outputs and output locking is not enabled. In this case, a new method calculates the transaction fee based upon the spendable outputs in the wallet for the selected account.

    In either case, if the transaction fee cannot be calculated but there are outputs to send to mwc713 wallet, "unknown" is displayed for the transaction fee in the confirmation screen.

    The new code to calculate the transaction fee will never stop the send from going through or displaying an error. It will either calculate the transaction fee so that it can be displayed on the Send confirmation screen, or a transaction fee of "unknown" will be displayed, if the fee cannot be calculated.

    However the existing code, that executes when there are HODL outputs or output locking is enabled, does display error messages when there are more than 500 outputs are being used by the transaction.

#Testing:
    When testing, I first always checked my outputs to see the amounts they had and selected the amount to send based upon which scenario I was trying to test. I would then calculate the txn fee based upon the outputs I thought should be used, then verified the transaction fee displayed on the Send confirmation screen was the same, and verified that the transaction amount for the new transaction displayed in the transaction screen included the same transaction fee.

#Test Scenarios:
    ALL - ensured all spendable coins were used

    amount + fee = all spendable coins

    amount + fee = amount of lowest spendable outputs

    send only uses confirmed outputs

    send only uses unlocked outputs

    transaction fee is calculated correctly for outputs being used as per above diatribe

    transaction fee displayed in confirmation screen matches transaction fee added to amount of transaction in transaction window

    the transaction fee should still be correct when the outputs to be selected are changed because the confirmation number setting is changed. Tested using 10 and 1 as values.

    fee changes appropriately based upon different change output settings (I didn't unit test). If you increase the change outputs, the transaction fee should increase for sends of the same amount with the same number of inputs (assuming amount being sent is less than the total amount available). 

#Error Scenarios:
    amount entered exceeds spendable coins - already detected in previous code base

    amount + fee exceeds spendable coins - calculate the fee in advance and select an amount to send that is just under the total number of spendable coins so you can get past the amount check when you hit Next. When the fee is calculated and included with the amount, it will exceed the total number of spendable coins. "unknown" should get displayed for the txn fee and you should get an error from the wallet which is then displayed by Qt Wallet.

    no spendable outputs detected - should only be allowed to get past initial amount check when there are locked outputs. Then unknown transaction fee should appear in the confirmation screen, if we even get that far.

    more than 500 outputs used in transactions - error message is displayed if locking enabled or HODL outputs. Not sure what wallet does in the other case. I saw some code where it looks like it might page through the outputs when there are more than 500. (I didn't unit test)


I did a send on the mining wallet which was successful but very slow.


