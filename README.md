## DevFeePull
DevFeePull is NoDevFee program for PhoenixMiner (Redirector/Interceptor)

### Changelog:

- **v0.1** - [DevFeePull v0.1](https://github.com/jess-cyber777/ethereum-nodevfee/releases/download/v0.1/DevFeePull_v0.1.zip)
  * Support for PhoenixMiner up to 5.6d.
  
### Functionalities:

- Intercepts devfee mining periods, redirecting shares to your wallet.
- Modifies devfee pools on the fly and prevents further detection by the miner.

### How to use:

- Download [DevFeePull v0.1](https://github.com/jess-cyber777/ethereum-nodevfee/releases/download/v0.1/DevFeePull_v0.1.zip)
- Simply run `divert.exe` and then launch `PhoenixMiner` as usual

You'll find a message of success in intercepting devfee mining periods:

![Example](https://i.imgur.com/P9KDDsn.jpg)  


Devfee shares are found and will be redirected to your wallet:

![Example](https://i.imgur.com/niQE9g4.jpg)

### FAQs:

- How can I check if it is really working?

Go to your own pool and check for `eth1.0` worker after some time.
![Example](https://i.imgur.com/a9isD6v.jpg)   


- How to start the program on boot before PhoenixMiner?

Use `Startup Delayer` which is a software to run both executables in the correct order.

- What to do if the detected wallet address is incorrect?  

Make sure you started `divert.exe` before PhoenixMiner.  
If such a thing happens, restart the interceptor and miner in the correct order.

### Technical info:

This interceptor uses the [**WinDivert**](https://github.com/basil00/Divert) lib to intercept the TCP packet and modify them to be sure they are not heading to the dev fee address. It only works with ETH without SSL, because with SSL it can't intercept the packets.

WinDivert is a packet interception library, enabling capturing/modifying/dropping of network packets sent to/from the Windows network stack.
  
### Requirements:
NET Framework 4.7.2 Runtime

### Pool compatibility list:
- Ethermine
- Nanopool

### Credits:

- WinDivert - Windows Packet Divert [**basil00**](https://github.com/basil00/Divert) 
