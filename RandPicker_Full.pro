TEMPLATE = subdirs

# 定义子项目
SUBDIRS += \
    RPQt.pro \
    ExtensionServer/RPExtensionServer.pro

# 设置构建目录，让所有目标文件输出到同一 bin 文件夹
CONFIG += ordered
