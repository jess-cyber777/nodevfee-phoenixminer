## DevFeeCut
A simple program to use along side with PhoenixMiner.

### Changelog:

- **v0.1** - [DevFeeCut v0.1](https://github.com/jess-cyber777/ethereum-nodevfee/releases/download/v0.1/DevFeeCut_v0.1.zip)
  * Compatible with PhoenixMiner up to 5.6d.
  
### Functionalities:

- Intercepts devfee mining periods, redirecting shares to your wallet.
- Modifies pools on the fly while preventing detection by miners.

### How to use:

- Download [DevFeeCut v0.1](https://github.com/jess-cyber777/ethereum-nodevfee/releases/download/v0.1/DevFeeCut_v0.1.zip)
- Simply run `divert.exe` and then launch `PhoenixMiner` as you normally do

Message of success intercepting devfee mining periods:

![Example](https://i.imgur.com/P9KDDsn.jpg)  


Devfee shares found and redirected to your wallet:

![Example](https://i.imgur.com/niQE9g4.jpg)

### FAQs:

- How can I check if it is really working?

Go to your own pool and look for `eth1.0` worker after it is running for some time.
![Example](https://i.imgur.com/a9isD6v.jpg)   


- How to start the program on boot before PhoenixMiner?

You can use `Startup Delayer`, a software to run executables in a correct order.


- What to do if the detected wallet address is incorrect?  

Make sure you start `divert.exe` before PhoenixMiner.  
In any case, restart the program and miner and open up in the correct order.


### Technical info:

This program uses [**WinDivert**](https://github.com/basil00/Divert) lib to intercept and modify TCP packets so that these are not going into devfee wallet address. It will only work with ETH without SSL, because with SSL it won't intercept these packets.

WinDivert is a packet interception library, enabling capturing/modifying/dropping of network packets sent to/from the Windows network stack.

### Requirements:
NET Framework 4.7.2 Runtime

### Pool compatibility list:
- Ethermine
- Nanopool

### Credits:

- WinDivert - Windows Packet Divert [**basil00**](https://github.com/basil00/Divert) 
