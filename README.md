# 1. duilib2
关于`duilib`的介绍可以访问：[https://github.com/duilib/duilib](https://github.com/duilib/duilib)

该项目基于[DuiLib_Ultimate](https://github.com/qdtroy/DuiLib_Ultimate)修改，对其进行了BUG修复、功能增强。

# 2. 如何编译
使用CMake生成相应的工程然后编译。

如果需要编译Cef控件，需要打开`UILIB_WITH_CEF`选项，并设置`CEF_BINARY_VER`参数。

`CEF_BINARY_VER`默认值位`1.0`，CMake脚本会默认下载[winsoft666 cef_binary](https://github.com/winsoft666/cef_binary)项目的v1.0版本到`cef_binary-1.0`目录，也可以从其他位置下载其他版本的cef，放置到此目录。

# 3. 属性
属性文档见：`src\doc\Attributes.xml`

# 4. BUG修复及新增功能

- 修复：在高DPI模式下, CLabelUI(含派生于它的CButtonUI控件)的size不正确的BUG。
- 修复：Edit控件在没有指定高度时，宽度无法适应DPI的BUG。
- 修复：List控件在拖动鼠标改变表头宽度时，表头宽度成倍数级增长的BUG。
- 修复：List控件在当用户指定了表头高度时，没有将高度进行DPI缩放的BUG。
- 修复：在未指定字体，使用默认字体时，文字被进行2次DPI Scale的BUG。
- 修复：Menu控件在高DPI下Menu窗体Size计算错误的BUG。
- 修复：Windows的maxinfo属性不支持DPI缩放的BUG。
- 修复：Caption 属性不能适应DPI缩放的BUG。
- 优化：不同DPI图片选择机制，在对应的DPI图片(***@200.png)不存在时，使用1倍原图缩放。
- 优化：动态设置非资源图片，支持DPI图片选择机制。
- 新增：ListHeader新增sepwidthadaptdpiscale属性，用于指定分隔符宽是否适应DPI缩放。
- 新增：bkimage等属性新增adaptdpiscale子属性，用于指定该图片是否适应DPI缩放。
- 新增：Window新增dpi属性，支持dpi数字、system等2种类型取值。
- 修复：绘制矩形边框时，右边框和下边框有1px的间距误差的BUG。
- 修复：压缩包内文件不支持中文的BUG。
- 修复：Groupbox的上边框不能位于文字中间的BUG。
- 新增：新增Windows size的取值max，支持在启动时即最大化。
- 新增：发送任务（含lamda表达式）到UI线程的功能，见`Utils\Task.h`。
- 新增：CEF控件，使用CEF离线渲染模式，支持背景透明，异形控件，Debug/Release均可完美退出无cef异常抛出。
- 优化：CEF支持GDI绘制和OpenGL绘制，提升客户端兼容性。
- 优化：任务栏托盘，提供任务栏重启重新创建托盘图标的功能。
- 修复：flash控件显示异常，及无法与C++交互的问题。
- 新增：CMake支持。
- 新增：支持通过Vcpkg安装。

