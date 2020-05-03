# Claimong process for HODL

Here is a guide how to claim your MWC for HODL program. 
Claims should be processed with Online Wallet (Hot Wallet), but it is possible to claim for Cold Wallet instance for that.

## Claim MWC for Online Wallet

Start your online hot wallet nad select HODL page. If your wallet is enrolled into the HODL program, you should see non zero "HODL amount" and something available to claim.

![](hodl_claims_images/hodl_page.png)

Press Claims Button. Leave checked "I am claiming MWC for this wallet". Press "Continue".

![](hodl_claims_images/claim_hash.png)

Normally there is one transaction available to claim and few historical transactions. Press "Claim MWC" to start the process.

![](hodl_claims_images/claims_page.png)

Claiming process should take few seconds. You should see the success message at the end.

![](hodl_claims_images/49bb35a8.png)

Press OK. You are done. You should see the transaction status "Response accepted"

![](hodl_claims_images/claiming_done.png)

Finalization process will take a long time because it will be done manually offline. When your transaction will be processed,
status will be changed to 'Finalized'.


## Claim MWC for Cold (OFFLINE) wallet.

Please note, this workflow required 2 wallets. Your offline Cold Wallet that you will need to sign the message and Online Hot Wallet that 
will be used for claims. Coins will be sent to your hot wallet. Then you can transfer them to your cold wallet as usual.

Start your online hot wallet. You you don't have it, please create a new wallet for claiming.

Select HODL page at hot wallet. HODL server said that this wallet is not registered and it is expected. You will claim for another wallet.
Press "Claims" button to continue.  

![](hodl_claims_images/Hodl_not_registered.png)

Uncheck the "I am claiming MWC for this wallet". Now you need to obtain public key hash from you Cold Wallet.

![](hodl_claims_images/claim_for_cold1.png)

Start your Cold Wallet and select HODL Page there. Please copy 'Wallet root public key hash' into your Hot wallet at the form above

![](hodl_claims_images/hodl_cold1.png)

When you input the Hash code, please press "Continue"

![](hodl_claims_images/hodl_claim_with_hash.png)

Now you should be able to see registered coins, and available claims for your Cold Wallet. To continue press "Claim MWC" button.

![](hodl_claims_images/claims_page.png)

During claiming process you will be asked to sign the message with your Cold wallet. Please copy "Message to sign with Cold Wallet" to your Cold Wallet side. 

![](hodl_claims_images/hot_sign_massage.png)

At your Cold wallet, HODL Page, input the message to sign and press "Sign" button

![](hodl_claims_images/cold_sign1.png)

Copy "Signature" value to your Hot wallet at the form above.

![](hodl_claims_images/Cold_sign2.png)

Input the signature into the Hot Wallet and press "Continue"

![](hodl_claims_images/hot_signature.png)

You should see the success message.

![](hodl_claims_images/hot_claim_ok.png)

Press OK. You are done. You should see the transaction status "Response accepted"

![](hodl_claims_images/claiming_done.png)

Finalization process will take a long time because it will be done manually offline. When your transaction will be processed,
status will be changed to 'Finalized'.
