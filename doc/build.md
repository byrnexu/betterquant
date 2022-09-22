# 编译
* 🔥 目前在Ubuntu 20.04.3 LTS上编译通过，系统本身是跨平台的，只是在Ubuntu 20.04.3 LTS完善了初始化脚本

* 🔥 Clone 代码到本地
```bash
   git clone git@github.com:byrnexu/betterquant.git
```

* 🔥 首次编译请先初始化系统安装必要的依赖，C++采用的是17标准。
```bash
   cd betterquant && bash init-sys.sh
```

* 🔥 编译
```bash
   bash build-all.sh
```
&emsp;&emsp;系统会自动下载并编译第三方库。<br/>
&emsp;&emsp;由于开启了并行编译，因此可能编译过程系统内存不足导致中断，请将setting.sh中DEFAULT_PARALLEL_COMPILE_THREAD_NUM=8从8修改为更小的数值🎃。
