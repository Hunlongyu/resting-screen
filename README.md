## 息屏助手

<p align="center">
	<img src="https://forthebadge.com/images/badges/built-with-love.svg">
<p>

<p align="center">
<img alt="GitHub All Releases" src="https://img.shields.io/github/downloads/Hunlongyu/resting-screen/total?style=for-the-badge">
<img alt="GitHub release (latest by date including pre-releases)" src="https://img.shields.io/github/v/release/Hunlongyu/resting-screen?include_prereleases&style=for-the-badge">
<img alt="GitHub" src="https://img.shields.io/github/license/Hunlongyu/resting-screen?style=for-the-badge">
<p>

### ✨ 特性
1. 全局快捷键 ALT + L
2. 支持锁屏 + 息屏
3. 支持开机自启
4. 无界面，托盘图标
5. 软件：78KB， 安装包：1.5MB，占内存：6.5MB
6. 支持 x86，x64 系统


### 👀 说明

* 息屏：让显示器进入待机模式节省电力，不影响后台进程。唤醒时继续之前操作，无需密码（除非已设定）。
* 锁屏：显示登录界面，需密码才能访问系统，确保无授权用户无法使用，适用于临时离开电脑。后台任务继续运行。

总的来说，息屏主要用于节省电力，而锁屏主要用于提升账户安全。

### ⚙️ 软件配置
托盘图标的右键菜单项里修改

![](https://s2.loli.net/2024/03/16/RwbpE7cVAqxIvhW.png)

### ⌚ 更新日志
v1.4.1 20240321
1. 将 vs 项目管理，改成 cmake
2. 优化目录结构

v1.4.0 20240316
1. 移除界面
2. 将配置项放置到右键菜单里
3. 更新依赖库
4. 修改托盘图标的提示文字

v1.3.1 20240309
1. 增加开机自启功能
2. 托盘菜单添加开机自启菜单项
3. C++14 切到 C++17
4. 安装时默认勾选创建桌面图标

v1.3.0 20240309
1. 将配置文件移到用户数据目录
2. 托盘图标新增设置菜单，点击后直接打开配置文件
3. 优化 x64 和 x86 安装路径
4. 优化版本号，统一管理
5. 优化 inno 构建脚本

v1.2.0 20240308
1. 增加息屏时是否锁屏的配置项
2. 增加 x86 32位操作系统的安装包
3. 增加脚本，一次可以编译2个安装包

v1.1.0 20240306
1. 增加配置文件
2. 启动软件则立即息屏
3. 启动软件不显示窗口
4. 新增全局快捷键 ALT + L
5. 增加单实例检测

v1.0.0 20240228
1. 双击息屏
2. 记录上次启动位置
3. 窗口置顶
4. 窗口拖拽

### 😃 唠唠叨叨
我是一个很注重午休的人，中午大厅关灯，屏幕锁屏后，却依旧亮着，还需要手动关闭显示器电源，午休结束还需要再次开启显示器电源，神烦。

现在可以快速息屏，午休结束动一下鼠标就能点亮屏幕，进入电脑桌面，美滋滋~~