mysql-connector-c-6.1.10-src 可以支持vs2015，不需要再去下载 mysql 源码了

####
mysql-connector-c-6.1.6-src 不支持vs2015，所以必须使用 mysql-5.7.10 去生成libmysql

生成 Unicode 版本：
Just add this line in your top CMakeLists.txt file: 
add_definitions(-DUNICODE -D_UNICODE)

mysql-5.7.10 尚不支持 UNICODE，但可以支持 64 bit.