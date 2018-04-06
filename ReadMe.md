

##inlinehook lib for x86/64

---

###说明：
作者:[choday](https://github.com/choday/hooklib)


使用了distorm反编译器。
封装了两种接口，直接C和C++调用。

添加了完整工程，添加使用demo。

测试了C和Win API的HOOK。

x86位下没啥问题，x64位下MessageBoxA的hook失败。

---

make_jmp_near函数就算jmp的大小。

其中x64位下使用的跳转是

jmp 或者 call模式。

jmp共使用14个字节，0xFF2500000000为6个字节，目标地址为00000000`00000000为8字节。

call模式

0xff1500000002为6个字节，目标地址为00000000`00000000为8字节。

---

###Hook x64 MessageBoxA失败

00000000774912B8  sub         rsp,38h 

00000000774912BC  xor         r11d,r11d 

// 下面这句导致inlinehook失败，make_jmp_near返回7个字节。

00000000774912BF  cmp         dword ptr [gfEMIEnable (774B213Ch)],r11d 

00000000774912C6  je          MessageBoxA+3Eh (774912F6h) 

00000000774912C8  mov         rax,qword ptr gs:[30h] 

00000000774912D1  mov         r10,qword ptr [rax+48h] 

---








