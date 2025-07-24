# Installing the MWC Qt-Wallet on macOS

## 1. Download the correct archive for your Mac

Go to the release page  
👉 [https://github.com/mwcproject/mwc-qt-wallet/releases](https://github.com/mwcproject/mwc-qt-wallet/releases)

| CPU | Archive to download                       |
|---|-------------------------------------------|
| **Apple Silicon (M1 / M2 / M3 …)** | `mwc-qt-wallet_X.X.X-macos-arm.tar.gz` |
| **Intel** | `mwc-qt-wallet_X.X.X-macos-x64.tar.gz`   |

Each archive has a companion file ending in `.sha256sum` - download that too.

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

## 6. Run MWC QT Wallet  

MWC QT Wallet is ready. Go to your Applications and run mwc-qt-wallet.

# Installing the MWC Qt-Wallet on Windows

Go to the release page  
👉 [https://github.com/mwcproject/mwc-qt-wallet/releases](https://github.com/mwcproject/mwc-qt-wallet/releases)

| Platform                                      | Archive to download                                |
|-----------------------------------------------|----------------------------------------------------|
| **Intel/AMD x64 CPU**                         | `mwc-qt-wallet_X.X.XX-win64-setup-standardcpu.exe` |
| **Low end Intel/AMD x64 CPU (Atom, Celeron)** | `mwc-qt-wallet_X.X.XX-win64-setup-basiccpu.exe`    |
| **32 bit Windows**                            | `mwc-qt-wallet_X.X.XX-win32-setup.exe`           |

Each archive has a companion file ending in `.sha256sum` - download that too.

### 2. Verify File Integrity (SHA256)

To ensure the installer is not corrupted or tampered with:

1. Open **PowerShell**
2. Navigate to the download folder:

```powershell
cd .\Downloads\
Get-FileHash mwc-qt-wallet_*-win*-setup*.exe -Algorithm SHA256
```

You will see output similar ti this (your hash might be different)

```powershell
Algorithm       Hash                                                                   Path
---------       ----                                                                   ----
SHA256          D705CA53064E1AD9FF4AA646E3CED39DA133FE561FC362C9C5741EC6751484C7       C:\Users\XXXXX\Downloads\mwc-qt-...
```

Print content of companion sha256sum file:
```powershell
Get-Content mwc-qt-wallet_*-win*-setup*.exe.sha256sum
```

You should see the content with expected value of the hash:

```powershell
d705ca53064e1ad9ff4aa646e3ced39da133fe561fc362c9c5741ec6751484c7  mwc-qt-wallet_1.2.14-win32-setup.exe
```

Compare the hash values, if they are different, the download is corrupted - delete the installer and re-download.

### 3. Run installer

If hash verification was successful, from the Powershell run the installer:

```powershell
./mwc-qt-wallet_*-win*-setup*.exe
```

You should see the installer UI:

![wnd](installation/windows_installer.png)

Follow the installer steps.

### Run MWC QT Wallet

- Open the **Start Menu**
- Search for **MWC GUI**
- Launch the app

