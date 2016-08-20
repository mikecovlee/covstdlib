Covariant C++ Library(2nd Generation) Guide
-----------------------------------------------
**智锐随变C++实用工具库(第二版)指南**

###1:License(使用协议)
**This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.**
**This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU General Public License for more details.**
*You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.*

**Copyright (C) 2016 Michael Lee(李登淳)**

Email: China-LDC@outlook.com
Github: https://github.com/mikecovlee
Website: http://ldc.atd3.cn

###2:Warning Codes and Error Codes Reference(警告码与错误码对照表)

#####English:
> 
*Note:
I am a Chinese guy.My English is very poor.So I can't complete this reference by myself.
Please contact me if you want to help me translate this guide.Thanks!*

E0001 Covariant C++ Library need your compiler support C++ Programming Launguage.
E0002 Covariant C++ Library need your compiler support C++14 or higher standard.
E0003 Target is not a function object.
E0004 Call member function with null pointer.

#####简体中文:
E0001 Covariant C++ Library需要您的编译器支持C++编程语言。
E0002 Covariant C++ Library需要您的编译器支持C++14或者更高标准。请检查您否忘记了[-std=c++14]编译选项。
E0003 对象不可执行。
E0004 使用空指针调用成员函数。
E0005 使用了未初始化的对象。
E0006 请求的类型与对象类型不同。
E0007 参数列表错误。
E0008 参数类型错误。
E0009 参数数量错误。
E000A 不支持接收任何参数。
E000B 类型匹配失败。

###3:Function List(功能列表)
Covariant Templates Toolbox
Covariant Functions Toolbox
cov::any
cov::list
cov::tuple
cov::timer
cov::switcher
cov::argument_list

----------
#Covariant Studio保留对本指南所有条目的解释权
#版权所有，盗版必究。
#开源不代表可以盗版，请尊重作者的著作权。
#欢迎通过Github向作者提交代码。感谢所有帮助我的人。

----------