# Installing the MWC Qt-Wallet on macOS

## 1. Download the correct archive for your Mac

Go to the release page  
👉 [https://github.com/mwcproject/mwc-qt-wallet/releases](https://github.com/mwcproject/mwc-qt-wallet/releases)

| CPU | Archive to download                       |
|---|-------------------------------------------|
| **Apple Silicon (M1 / M2 / M3 …)** | `mwc-qt-wallet_X.X.X-macos-arm.tar.gz` |
| **Intel** | `mwc-qt-wallet_X.X.X-macos-x64.tar.gz`   |

Each archive has a companion file ending in `.sha256sum`—download that too.

---

## 2. Verify the checksum (optional but recommended)

Open **Terminal** and `cd` to the folder where you downloaded the files (usually `~/Downloads`).

```bash
shasum -a 256 -c mwc-qt-wallet_*-macos-*.tar.gz.sha256sum
```
If the file is valid you’ll see:
```
mwc-qt-wallet_1.2.14-macos-*.tar.gz: OK
```
Any other message means the download is corrupted - delete and re-download.

## 3. Unpack the archive and run installer

In Terminal:

```bash
tar -xzf mwc-qt-wallet_*-macos-*.tar.gz
mwc-qt-wallet/install.sh
```

You should see the app window.

![wnd](installation/mwc-qt-wallet-wnd.png)

Please note, mwc-qt wallet is not copied at your applicaiton yet. 

You can run the mwc-qt-wallt app by clickin on it.

## 5. Move the app to Applications

Drag the mwc-qt-wallet icon onto the Applications folder.

![wnd](installation/drug_icon_image.png)

## 6. run MWC QT Wallet  

MWC QT Wallet is ready. Go to your Applications and run mwc-qt-wallet.