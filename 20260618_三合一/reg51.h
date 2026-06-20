/* BYTE Register */ 
sfr P0 = 0x80; //P0口 
sfr P1 = 0x90; //P1口 
sfr P2 = 0xA0; //P2口 
sfr P3 = 0xB0; //P3口 
sfr PSW = 0xD0; //程序状态字，具体位意义见位定义 
sfr ACC = 0xE0; //累加器，程序员最常用的 
sfr B = 0xF0; //寄存器，主要用于乘除 
sfr SP = 0x81; //堆栈指针，初始化为07；先加1后压栈，先出栈再减1， 
sfr DPL = 0x82; 
sfr DPH = 0x83; //数据指针，用途大 
sfr PCON = 0x87; //电源控制 
sfr TCON = 0x88; //Timer/Counter控制 
sfr TMOD = 0x89; //Timer/Counter方式控制 
sfr TL0 = 0x8A; 
sfr TL1 = 0x8B; // 
sfr TH0 = 0x8C; //存着当前的计数值 
sfr TH1 = 0x8D; //我就想不明白，当时设计的时候，为什么不把TH0,TL0放在连续的地址！ 
sfr IE = 0xA8; //好东西，中断控制 
sfr IP = 0xB8; //中断优先级控制，没有设计过要求时间严格的系统，所以至今没有用过 
sfr SCON = 0x98; //哇，熟悉，串口控制寄存器 
sfr SBUF = 0x99; //哇，更熟悉，串口缓冲寄存器

/* BIT Register */ 
/* PSW */ 
sbit CY = 0xD7; //进位或借位，有就是1，没有就是0 
sbit AC = 0xD6; //辅助进借位，（麻烦b） 
sbit F0 = 0xD5; //没有具体用途，可以由用户决定他的意义，所以它就没有意义 
sbit RS1 = 0xD4; 
sbit RS0 = 0xD3; //工作寄存器选择，这个在下面解释 
sbit OV = 0xD2; //over！溢出，有是1，没有是0 
sbit P = 0xD0; //奇偶校验，奇数个1是1
/* TCON */ 
sbit TF1 = 0x8F; //T1的中断请求标志 
sbit TR1 = 0x8E; //Timer 1 running，好记吧~ 
sbit TF0 = 0x8D; // 
sbit TR0 = 0x8C; //Timer 0running
sbit IE1 = 0x8B; //interrupt external 1 外中断请求标志 
sbit IT1 = 0x8A; //interrupt triggle 1 外中断触发方式 
sbit IE0 = 0x89; //interrupt external 0 外中断触发方式
sbit IT0 = 0x88; //interrupt triggle 0 外中断触发方式
/* IE */ 
sbit EA = 0xAF; //Enable all哇，重要，全局中断控制，光着他，哈哈，什么都不用作了，就像放假一样 
sbit ES = 0xAC; //Enable Serial,开串口中断 
sbit ET1 = 0xAB; //Enable Timer/Counter 1 
sbit EX1 = 0xAA; //Enable External 1 
sbit ET0 = 0xA9; //Enable Timer/counter 0 
sbit EX0 = 0xA8; //Enable External 0
/* IP */ 
sbit PS = 0xBC; //串行中断优先级 
sbit PT1 = 0xBB; //T1优先级 
sbit PX1 = 0xBA; //外部中断1优先级 
sbit PT0 = 0xB9; // 
sbit PX0 = 0xB8; //上面两个1换成0
/* P3 */ //控制寄存器！！！！ 
sbit RD = 0xB7; //读 
sbit WR = 0xB6; //写 
sbit T1 = 0xB5; //T/C1 
sbit T0 = 0xB4; //T/C0 
sbit INT1 = 0xB3; //外中断1 
sbit INT0 = 0xB2; //外中断0 
sbit TXD = 0xB1; //串行发送 
sbit RXD = 0xB0; //串行接收
/* SCON */ 
sbit SM0 = 0x9F; // 
sbit SM1 = 0x9E; //串口工作方式 
sbit SM2 = 0x9D; //多机通信控制位
sbit REN = 0x9C; //串行接收允许 
sbit TB8 = 0x9B; //收到的第九位 
sbit RB8 = 0x9A; //要发的第九位 
sbit TI = 0x99; //哇，熟悉吧，发送完成中断标志 
sbit RI = 0x98; //接收完成中断标志

