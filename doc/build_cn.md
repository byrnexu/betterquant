# 编译
* 🔥 目前在Ubuntu 20.04上编译通过，系统本身是跨平台的，只是在Ubuntu 20.04 LTS完善了初始化脚本，我是在 root 用户下测试的，请切换到 root 用户。

&emsp;&emsp;阿里云的 ubuntu 20.04 64 位和 vmware 下的 ubuntu-20.04.5-desktop-amd64 均可正常编译，vmware 下的 ubuntu-20.04.5-desktop-amd64 下载地址: https://releases.ubuntu.com/20.04.5/ubuntu-20.04.5-desktop-amd64.iso 。本地的虚拟机环境还要装 vpn 什么的，比较麻烦，所以还是建议在云端编译部署。
&emsp;

* 🔥 Clone 代码到本地
```bash
   git clone git@github.com:byrnexu/betterquant.git
   
   # 如果使用发布的版本那么使用以下命令切换，master分支上的功能和特性还不那么稳定。
   git reset --hard $(git tag --list | tail -n1) 
```
&emsp;

* 🔥 首次编译请先初始化系统安装必要的依赖，C++采用的是17标准。
```bash
   cd betterquant && bash init-sys.sh
```
&emsp;

* 🔥 编译  

&emsp;&emsp;编译时间较长，如果需要开启并行编译，请将setting.sh中DEFAULT_PARALLEL_COMPILE_THREAD_NUM=1从1修改为更大的数值，当然如果编译线程数过多，极有可能会导致系统内存不足而编译中断，所以保险起见不要修改此参数，整个编译过程大概会持续一小时，C++ 你懂的。🎃。
```bash
   bash build-all.sh
```
&emsp;&emsp;系统会自动下载并编译第三方库。有时候在 vmware 虚拟机环境编译的时候会遇到 github 上一些第三方库下载超时的情况，ctrl + c 后重新 build-all.sh即可。<br/>
&emsp;

* 🔥 生成策略需要的压缩包

&emsp;&emsp;运行脚本
```bash
   bash deploy_stgeng.sh
```
&emsp;
* 🔥 上一步骤生成的压缩包含以下目录和文件：  

| 目录/文件 | 说明 | 备注 |
| ------ | ------ | ------ |
| inc/cxx | 编写cxx策略需要的头文件 |  |
| lib/libbqstgeng-cxx.a | 编写cxx策略需要的库文件 |  |
| bin/bqstgeng.so | 编写python策略需要的库文件 |  |
| bin/stgeng.py | python策略库 |  |

lib/libbqstgeng-cxx.a 库文件非常大，因为包含很多符号，如果觉得太大可以用 strip libbqstgeng-cxx.a 将符号删除，当然没有符号的话策略 crash 会导致问题难以定位，建议保留符号。  
