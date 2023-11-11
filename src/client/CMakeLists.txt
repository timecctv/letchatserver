#定义了一个CLIENT变量，包含当前目录下所有文件
aux_source_directory(. CLIENT)
#指定生成可执行文件的名字
add_executable(chatclient ${CLIENT})

#指定生成可执行文件是需要连接的库文件
target_link_libraries(chatclient pthread)