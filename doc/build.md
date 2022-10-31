# BUILD
* 🔥 At present, it has been compiled and passed on Ubuntu 20.04. The system itself is cross-platform, but the initialization script has been improved in Ubuntu 20.04. I tested it under the root user, please switch to the root user.

&emsp;&emsp;Alibaba Cloud's ubuntu 20.04 64-bit and ubuntu-20.04.5-desktop-amd64 under vmware can be compiled normally, AWS has not tried it, and it should not be a big problem. The download address of ubuntu-20.04.5-desktop-amd64 under vmware : https://releases.ubuntu.com/20.04.5/ubuntu-20.04.5-desktop-amd64.iso. The local virtual machine environment also needs to install vpn or something, which is more troublesome, so it is recommended to compile and deploy in the cloud.
&emsp;

* 🔥 git clone
```bash
   git clone git@github.com:byrnexu/betterquant.git
   
   # If using the released version then use the following command to switch, 
   # the functions and features on the master branch are not so stable yet.
   git reset --hard $(git tag --list | tail -n1) 
```
&emsp;

* 🔥 For the first compilation, please initialize the system and install the necessary dependencies. C++ adopts the 17 standard.
```bash
   cd betterquant && bash init-sys.sh
```
&emsp;

* 🔥 build  

&emsp;&emsp;Since parallel compilation is enabled, it may be interrupted due to insufficient system memory during the compilation process. Please modify DEFAULT_PARALLEL_COMPILE_THREAD_NUM=8 in setting.sh from 8 to a smaller value🎃。
```bash
   bash build-all.sh
```
&emsp;&emsp;The system will automatically download and compile third-party libraries. Sometimes when compiling in the vmware virtual machine environment, the download of some third-party libraries on github times out. You can re-build-all.sh after ctrl + c.<br/>
&emsp;

* 🔥 Generate the compressed package required by the strategy.

&emsp;&emsp;run script
```bash
   bash deploy_stgeng.sh
```
&emsp;
* 🔥 The zip generated in the previous step contains the following directories and files:  

| Directory/Filename | Detailed description | Remark |
| ------ | ------ | ------ |
| inc/cxx | Header files required by the cxx strategy |  |
| lib/libbqstgeng-cxx.a | Library files required by the cxx strategy |  |
| bin/bqstgeng.so | Library files required by the python strategy |  |
| bin/stgeng.py | Package required by the python strategy  |  |

The lib/libbqstgeng-cxx.a library file is very large because it contains many symbols. If you think it is too large, you can use strip libbqstgeng-cxx.a to delete the symbols. Of course, if there are no symbols, the strategy crash will cause the problem to be difficult to locate. It is recommended to keep the symbols.
