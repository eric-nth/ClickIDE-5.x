# ClickIDE

Thank you for your visit to ClickIDE Project. If you like it, please give us a star~ Thanks!

## Brief Introduction

Click is an light, open-source, convenient C++/Pascal IDE which based on MinGW and FPC.

ClickIDE是一款轻量级的C++/Pascal IDE，可以满足正常C++编程学习的需求。软件优势：占用空间极小（软件本身只有约2MB，附上编译器也只有约500MB）；完全开源（遵循Apache协议）；免安装直接使用；不需要任何依赖，直接运行main文件夹下的exe文件即可，可以直接拷贝此文件到任何一台电脑上直接使用；资源占用很低，任何电脑都可以轻松运行；可以导出带有语法高亮的HTML；可以调整字号。不足：需要配置环境变量（软件里已经有详细的配置方法），无法显示行号。

## Snapshots

![Snapshots while using](https://ericnth.cn/wp-content/uploads/2020/04/clickide%E4%BD%BF%E7%94%A8%E6%88%AA%E5%9B%BE1-1536x753.png)

## Platforms Available

Windows All(x86/x64)

## To Download

Just clone ***Click4.6.exe*** to your computer!

Or: you can visit: [ericnth.cn/clickide](https://ericnth.cn/clickide) to get faster downloads.

## Update Log

<h3>4.8<small>（2005）（预告）</small></h3>
<p>必定实现功能：C++代码自动格式化。 可能实现功能：免除环境变量配置。</p>
<h3>4.6.5<small>（2004.1）</small></h3>
<p>优化了用户体验，添加了“更改字体大小”，更换了两种更容易阅读的字体。</p>
<h3>4.6.0<small>（2004.0）</small></h3>
<p>添加“一键导出带有语法高亮的HTML”功能，使得代码阅读更舒畅，更容易分享。</p>
<h3>4.0<small>（2002）</small></h3>
<p>C++添加“gdb调试”功能，重大bug修复，可以刷新状态栏。</p>
<h3>3.8<small>（2001.1）</small></h3>
<p>界面更加友好，添加“状态栏”功能，编译状态识别，bug修复，添加“Edit”弹出菜单，加入“帮助”页面。</p>
<h3>3.2<small>（2001.0）</small></h3>
<p>重新整理分类目录，开始支持bat批处理，修复了部分崩溃问题，添加“一键当前目录运行cmd”，使用更灵活。</p>
<h3>3.0<small>（1912）</small></h3>
<p>开始支持Pascal语言，bug修复，添加“直接运行”功能。</p>
<h3>2.0<small>（1911）</small></h3>
<p>使用图形界面。</p>

## Languages

ClickIDE only provides Chinese Version. But most of the options and things are written in English.

## More Information

Author: [EricNTH](https://ericnth.cn/)

License: [Apache License, Version 2.0](http://apache.org/licenses/LICENSE-2.0)

Price: Totally Free!

## Wanna know more?

Please visit our official website: [ericnth.cn](https://ericnth.cn/), you can visit our **official product page** to follow us or download  on: [ericnth.cn/clickide](https://ericnth.cn/clickide). You can also join our QQ group chat: 1019034208.

## Face some problems? Get help here!

1. 在您使用该软件进行编译运行前，请确保您已经将您的g++编译器bin目录和fpc编译器的bin\\i386-win32\\目录添加到环境变量PATH。（环境变量设置方法：右击“此电脑”->属性，点击左侧“高级系统设置”，在“高级”标签下单击“环境变量(N)...”，双击“系统变量”中的PATH项进行编辑，在后面添加“XXX\\FPC\\2.2.2\\bin\\i386-win32\\”和“XXX\\MinGW\\bin\\”（将XXX替换为你的安装位置，不要加引号！），然后全部点击“确定”即可。
2. 在您打开一个文件后，可以对它进行任何操作。我们并没有禁止类似打开一个C++文件后用\"Compile Pascal File...\"来进行编译等的操作（尽管这不对），因此您在使用编译/运行这些选项时，请务必确认是否选择了正确的编程语言！
3. 由于作者能力有限以及本软件向C++的偏向性，部分Pascal程序可能无法正确编译/运行，请您谅解。您也可选择使用其他Pascal编译器（只要把它的目录添加到环境变量PATH，并将软件安装时自带的FPC目录从环境变量PATH中移除即可。
4. 由于本软件开发时间较短，因此在使用过程中由以下限制： 1.仅用于Windows操作系统的部分支持Win32API的版本。\n  2.C++文件仅支持.cpp, .c++, .cxx后缀名，Pascal文件仅支持.pp后缀名，C++头文件仅支持.hpp后缀名，批处理文件仅支持.bat, .com, .cmd后缀名，请谅解。如您使用其他的后缀名（字符数量不符），可能导致编译运行失败。
5. 在您想要直接运行/调试一个程序时，必须先保存。并且，如果您想要运行/调试当前您写的程序，请先编译，否则运行/调试的是您上一次编译后生成的程序。
6. 若您的状态条被遮挡，且您想要查看，可以选择Help > Flush StatusBar进行刷新。
7. 本文件夹内Click4.6.exe是可以单独使用的，即：您可以把这个exe文件复制到任意位置（甚至其他Windows电脑）均可使用。但您需要自己设置MinGW和FPC库，在本发布版中已经自带（即MinGW和FPC文件夹），但您仍然可以使用自己的库。只要将它添加到环境变量即可。
8. 其次，若看到类似结尾为_compile_tmp.log的文件，是我们在编译过程中（可能会）生成的临时日志文件，您完全可以直接删除，对ClickIDE和其他软件的运行没有任何影响。
9. 在您编译时，若是C++程序，仅当发生错误/警告时才会发出提示，否则直接编译完成；若是Pascal程序，任何情况下都会发出编译提示，所以请认真留意提示中是否存在例如\"Fatal\"或\"Error\"之类的字眼，如有，则表明编译出错，反之，则表明编译通过。
10. 若有其他困难，问题，意见或者建议，请您一定要及时联系作者邮箱eric_ni2008@163.com，或加QQ群：1019034208，进行咨询或投诉，以便我们今后把ClickIDE做得更加完善！