To build the MWC QT Wallet we can use some scripts of the mwcbuildpipeline Repository in order to automate some steps of our build. <br>
> Please note that this process is tested on Ubuntu 18.04, other Distributions may vary and need some manual troubleshooting.<br>
> On Ubuntu 16.04 you might need to install SSL, see https://github.com/mwcproject/mwc-wallet/blob/master/doc/installing_openssl.md<br>

## Build process

1.) Clone the build pipeline

```git clone https://github.com/mwcproject/mwcbuildpipeline.git```

2.) Run the Setup script:

```cd mwcbuildpipeline```<br>
```sudo sh setup_linux.sh```<br>
```sudo apt-get install libncurses5-dev libncursesw5-dev```<br>

3.) Configure the build script to build the wanted version, in this example 1.0.22 

```sudo nano build-qt-wallet-linux.sh```

Add the following line above "compiling for CPU: $CPU_CORE"
```export CPU_CORE ="native"```

(this tells the compiler to compile for your native CPU) 


Now we need to select what to version to compile, as by default we always compile the current version of the branch
_Please note_ those version numbers will change, we use QT V1.0.22 here as an example, as a Rule of thumb you can use the latest Tag numbers on Github for the repective Repo's

3.1) create a file called mwc-qt-wallet.version containing
```1.0.22```<br>
3.2) create a file called mwc-node.version containing
```3.2.3```<br>
3.3) create a file called mwc713.version containing
```3.2.2```<br>


4.) Now we can build the binaries <br>
```sudo sh build-qt-wallet-linux.sh```

5.) Link the platforms folder of the QT subfolder. _Make sure the Version number is correct as this might change over time_.

```ln -s ../Qt/5.9.9/gcc_64/plugins/platforms platforms```

6.) Finally let's copy all binaries/Ressources required by the QT into the QT directory<br>
```cp mwc713/target/release/mwc713 ./mwc-qt-wallet/```<br>
```cp mwc713/target/release/mwczip ./mwc-qt-wallet/```<br>
```cp mwc-node/target/release/mwc ./mwc-qt-wallet/```<br>
```cp resources/tor.linux ./mwc-qt-wallet/tor```<br>


# Running the binary

To start the QT use the following command in the mwcbuildpipeline directory

```./mwc-qt-wallet/mwc-qt-wallet```



If there are any Errors during the Build process on other Distributions of a Unix based System you may need to Troubleshoot them on your own as they may have other packages and libraries preinstalled.
