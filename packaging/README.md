# keytop packaging

源码安装由根目录 Makefile 提供稳定接口，底层仍使用 CMake 构建。`DESTDIR` 只影响
暂存安装路径，不会写入用户配置、启动服务或设置 capability。
