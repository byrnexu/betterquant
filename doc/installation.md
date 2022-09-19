# 安装
* 🔥 先执行第一步编译

* 🔥 制作镜像  

&emsp;&emsp;需改docker/setting.sh中DB_PASSWD和ROOT_DIR_OF_INSTALLATION也就是配置、插件和日志及数据库的挂载目录。
```bash
   cd docker && bash make-images.sh
```

* 🔥 运行  

&emsp;&emsp;运行基本组件 
```bash
   bash run-base.sh
```
&emsp;&emsp;运行交易网关（数据库必须有相应的账户配置并在docker-compose-tdgw.yaml增加相应项）
```bash
   bash run-tdgw.sh
```
