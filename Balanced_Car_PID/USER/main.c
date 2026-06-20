/**
* @par Copyright (C): 2018-2028, Shenzhen Yahboom Tech
* @file         // main.c
* @author       // lly
* @version      // V2.0
* @date         // 240628
* @brief        // Balance Car PID Monitor
* @details      // Monitor and print PID data via serial port
*/

#include "AllHeader.h"

/*==============================================================================
 * 全局变量定义 - 原工程中定义在main.c的变量
 *============================================================================*/
/* 传感器数据 */
float Angle_Balance = 0.0f;
float Gyro_Balance = 0.0f;
float Gyro_Turn = 0.0f;
float Acceleration_Z = 0.0f;
int Temperature = 0;

/* 角度获取方式: 1:DMP 2:Kalman 3:Complementary */
uint8_t GET_Angle_Way = 2;

/* 电机输出 */
int Motor_Left = 0;
int Motor_Right = 0;

/* 控制参数 */
float Move_X = 0.0f;
float Move_Z = 0.0f;
int Mid_Angle = 0;

/* 停止标志: 0:运行 1:停止 */
uint8_t Stop_Flag = 1;

/*==============================================================================
 * 外部函数声明
 *============================================================================*/
extern void MPU6050_EXTI_Init(void);
extern int myabs(int a);

/* PID参数 - 定义在 pid_control.c (存储值=实际值*100) */
extern float Balance_Kp;    // 直立环P (实际值=Balance_Kp/100)
extern float Balance_Kd;    // 直立环D (实际值=Balance_Kd/100)
extern float Velocity_Kp;   // 速度环P (实际值=Velocity_Kp/100)
extern float Velocity_Ki;   // 速度环I (实际值=Velocity_Ki/100)
extern float Turn_Kp;       // 转向环P (实际值=Turn_Kp/100)
extern float Turn_Kd;       // 转向环D (实际值=Turn_Kd/100)

/* 速度变量 - 定义在 app_motor.c */
extern float Velocity_Left;
extern float Velocity_Right;

/* 电池电压 - 定义在 app_control.c */
extern float battery;

/*==============================================================================
 * 本文件变量
 *============================================================================*/
uint8_t Debug_Mode = 2;             // 0:关闭 1:简要 2:详细
uint32_t Print_Count = 0;
uint32_t Loop_Count = 0;

/* 统计用变量 */
float Angle_Max = 0.0f;             // 角度最大值
float Angle_Min = 0.0f;             // 角度最小值
float Angle_Sum = 0.0f;             // 角度累加
uint32_t Angle_Sample_Count = 0;    // 角度采样计数

/******************************************************************************
* 函数名  : main
* 描述    : 主函数 - 平衡小车串级PID监控
******************************************************************************/
int main(void)
{	
    Mid_Angle = 0;
    
    bsp_init();
    MPU6050_EXTI_Init();
    
    /*==========================================================================
     * 系统启动信息
     *==========================================================================*/
    printf("\r\n");
    printf("+==============================================+\r\n");
    printf("|                                              |\r\n");
    printf("|       Balance Car - PID Monitor System       |\r\n");
    printf("|       Cascade: Balance + Velocity + Turn     |\r\n");
    printf("|                                              |\r\n");
    printf("+==============================================+\r\n");
    printf("|                                              |\r\n");
    printf("|  Angle Algorithm: %d                          |\r\n", GET_Angle_Way);
    printf("|    (1: DMP  2: Kalman  3: Complementary)     |\r\n");
    printf("|                                              |\r\n");
    printf("|  Debug Mode: %d                                |\r\n", Debug_Mode);
    printf("|    (0: Off  1: Brief  2: Detailed)           |\r\n");
    printf("|                                              |\r\n");
    printf("+----------------------------------------------+\r\n");
    printf("|                                              |\r\n");
    printf("|         >>> PID Parameters Table <<<         |\r\n");
    printf("|                                              |\r\n");
    printf("|  +--------+----------+----------+----------+ |\r\n");
    printf("|  |  Loop  |    P     |    I     |    D     | |\r\n");
    printf("|  +--------+----------+----------+----------+ |\r\n");
    printf("|  | Balance| %8.2f |    ---   | %8.2f | |\r\n", 
           Balance_Kp/100.0f, Balance_Kd/100.0f);
    printf("|  | Speed  | %8.2f | %8.2f |    ---   | |\r\n", 
           Velocity_Kp/100.0f, Velocity_Ki/100.0f);
    printf("|  | Turn   | %8.2f |    ---   | %8.2f | |\r\n", 
           Turn_Kp/100.0f, Turn_Kd/100.0f);
    printf("|  +--------+----------+----------+----------+ |\r\n");
    printf("|                                              |\r\n");
    printf("|  Note: Values shown are actual coefficients  |\r\n");
    printf("|        (stored as value*100 in code)         |\r\n");
    printf("|                                              |\r\n");
    printf("+==============================================+\r\n");
    printf("\r\n");
    printf("  >>> Press KEY1 to START balance control <<<  \r\n");
    printf("\r\n");
    
    OLED_Draw_Line("put down key start!", 1, true, true);
    
    /* 等待按键启动 */
    while(!Key1_State(1));
    Stop_Flag = 0;
    
    printf("\r\n");
    printf("  +==========================================+\r\n");
    printf("  |     >>> BALANCE CONTROL START <<<        |\r\n");
    printf("  +==========================================+\r\n");
    printf("\r\n");
    
    OLED_Draw_Line("start control!", 1, true, true);
    
    /*==========================================================================
     * 主循环
     *==========================================================================*/
    while(1)
    {
        char showbuf[20];
        
        /* OLED显示角度 */
        sprintf(showbuf, "angle = %.2f  ", Angle_Balance);
        OLED_Draw_Line(showbuf, 3, false, true);
        
        /* 统计数据收集 */
        if(Angle_Balance > Angle_Max) Angle_Max = Angle_Balance;
        if(Angle_Balance < Angle_Min) Angle_Min = Angle_Balance;
        Angle_Sum += Angle_Balance;
        Angle_Sample_Count++;
        
        /* 串口打印 */
        if(Debug_Mode > 0)
        {
            Loop_Count++;
            
            /* 每200ms打印一次 (40 * 5ms) */
            if(Loop_Count % 40 == 0)
            {
                Print_Count++;
                
                if(Debug_Mode == 1)
                {
                    /*==========================================================
                     * 简要模式：单行输出
                     * 格式: [计数] 角度 角速度 转向角速度 | 电机L/R | 速度L/R
                     *==========================================================*/
                    printf("[%5lu] Ang:%7.2f GY:%7.2f GZ:%7.2f | "
                           "ML:%5d MR:%5d | "
                           "VL:%6.1f VR:%6.1f | "
                           "Bal(P:%.1f D:%.2f) Vel(P:%.1f I:%.2f) Turn(P:%.1f D:%.1f)\r\n",
                           Print_Count,
                           Angle_Balance, Gyro_Balance, Gyro_Turn,
                           Motor_Left, Motor_Right,
                           Velocity_Left, Velocity_Right,
                           Balance_Kp/100.0f, Balance_Kd/100.0f,
                           Velocity_Kp/100.0f, Velocity_Ki/100.0f,
                           Turn_Kp/100.0f, Turn_Kd/100.0f);
                }
                else
                {
                    /*==========================================================
                     * 详细模式：多行结构化输出
                     *==========================================================*/
                    float avg_angle = Angle_Sum / Angle_Sample_Count;
                    
                    printf("\r\n");
                    printf("+==================== PID MONITOR [%05lu] ====================+\r\n", 
                           Print_Count);
                    printf("|  Time: %6lu ms  |  Sample Period: 200ms                    |\r\n", 
                           Print_Count * 200);
                    printf("+-------------------------------------------------------------+\r\n");
                    
                    /*---------------------------------------------------------
                     * 第一部分：传感器原始数据
                     *---------------------------------------------------------*/
                    printf("|                                                             |\r\n");
                    printf("|  >>> SENSOR DATA <<<                                        |\r\n");
                    printf("|                                                             |\r\n");
                    printf("|  Angle (Pitch):    %10.2f deg                              |\r\n", 
                           Angle_Balance);
                    printf("|  Gyro_Balance (Y): %10.2f deg/s                            |\r\n", 
                           Gyro_Balance);
                    printf("|  Gyro_Turn (Z):    %10.2f deg/s                            |\r\n", 
                           Gyro_Turn);
                    printf("|  Accel_Z:          %10.2f                                  |\r\n", 
                           Acceleration_Z);
                    printf("|  Temperature:      %10d C                                  |\r\n", 
                           Temperature);
                    printf("|  Angle Stats: Max=%.2f  Min=%.2f  Avg=%.2f deg            |\r\n", 
                           Angle_Max, Angle_Min, avg_angle);
                    printf("|                                                             |\r\n");
                    
                    /*---------------------------------------------------------
                     * 第二部分：编码器与速度数据
                     *---------------------------------------------------------*/
                    printf("|  >>> VELOCITY DATA <<<                                      |\r\n");
                    printf("|                                                             |\r\n");
                    printf("|  Velocity_Left:    %10.2f mm/s                             |\r\n", 
                           Velocity_Left);
                    printf("|  Velocity_Right:   %10.2f mm/s                             |\r\n", 
                           Velocity_Right);
                    printf("|  Velocity_Avg:     %10.2f mm/s                             |\r\n", 
                           (Velocity_Left + Velocity_Right) / 2.0f);
                    printf("|  Velocity_Diff:    %10.2f mm/s  (L-R)                      |\r\n", 
                           Velocity_Left - Velocity_Right);
                    printf("|                                                             |\r\n");
                    
                    /*---------------------------------------------------------
                     * 第三部分：PID控制器参数与输出（核心新增）
                     *---------------------------------------------------------*/
                    printf("|  >>> PID CONTROLLERS <<<                                    |\r\n");
                    printf("|                                                             |\r\n");
                    
                    /* 直立环 PD */
                    printf("|  [1] Balance Loop (PD) - Keep Upright                      |\r\n");
                    printf("|      +------------------+------------------+               |\r\n");
                    printf("|      | Parameter |  Raw Value  | Actual Value |            |\r\n");
                    printf("|      +-----------+-------------+--------------+            |\r\n");
                    printf("|      |     P     | %8.0f   | %11.2f   |            |\r\n", 
                           Balance_Kp, Balance_Kp/100.0f);
                    printf("|      |     D     | %8.0f   | %11.2f   |            |\r\n", 
                           Balance_Kd, Balance_Kd/100.0f);
                    printf("|      +-----------+-------------+--------------+            |\r\n");
                    printf("|      Target Angle: %.2f deg                                |\r\n", 0.0f);
                    printf("|      Current Error: %.4f deg                               |\r\n", 
                           Angle_Balance - 0.0f);
                    printf("|                                                             |\r\n");
                    
                    /* 速度环 PI */
                    printf("|  [2] Velocity Loop (PI) - Control Speed                    |\r\n");
                    printf("|      +------------------+------------------+               |\r\n");
                    printf("|      | Parameter |  Raw Value  | Actual Value |            |\r\n");
                    printf("|      +-----------+-------------+--------------+            |\r\n");
                    printf("|      |     P     | %8.0f   | %11.2f   |            |\r\n", 
                           Velocity_Kp, Velocity_Kp/100.0f);
                    printf("|      |     I     | %8.0f   | %11.3f   |            |\r\n", 
                           Velocity_Ki, Velocity_Ki/100.0f);
                    printf("|      +-----------+-------------+--------------+            |\r\n");
                    printf("|      Target Speed: %.2f mm/s                               |\r\n", 0.0f);
                    printf("|      Current Error: %.4f mm/s                              |\r\n", 
                           (Velocity_Left + Velocity_Right) / 2.0f);
                    printf("|                                                             |\r\n");
                    
                    /* 转向环 PD */
                    printf("|  [3] Turn Loop (PD) - Control Steering                     |\r\n");
                    printf("|      +------------------+------------------+               |\r\n");
                    printf("|      | Parameter |  Raw Value  | Actual Value |            |\r\n");
                    printf("|      +-----------+-------------+--------------+            |\r\n");
                    printf("|      |     P     | %8.0f   | %11.2f   |            |\r\n", 
                           Turn_Kp, Turn_Kp/100.0f);
                    printf("|      |     D     | %8.0f   | %11.2f   |            |\r\n", 
                           Turn_Kd, Turn_Kd/100.0f);
                    printf("|      +-----------+-------------+--------------+            |\r\n");
                    printf("|      Target Gyro_Z: %.2f deg/s                             |\r\n", 0.0f);
                    printf("|      Current Error: %.4f deg/s                             |\r\n", Gyro_Turn);
                    printf("|                                                             |\r\n");
                    
                    /* PID参数汇总表 */
                    printf("|  >>> PID SUMMARY TABLE <<<                                  |\r\n");
                    printf("|                                                             |\r\n");
                    printf("|  +----------+----------+----------+----------+             |\r\n");
                    printf("|  |   Loop   |    P     |    I     |    D     |             |\r\n");
                    printf("|  +----------+----------+----------+----------+             |\r\n");
                    printf("|  | Balance  | %8.2f |   ----   | %8.2f |             |\r\n", 
                           Balance_Kp/100.0f, Balance_Kd/100.0f);
                    printf("|  | Velocity | %8.2f | %8.3f |   ----   |             |\r\n", 
                           Velocity_Kp/100.0f, Velocity_Ki/100.0f);
                    printf("|  | Turn     | %8.2f |   ----   | %8.2f |             |\r\n", 
                           Turn_Kp/100.0f, Turn_Kd/100.0f);
                    printf("|  +----------+----------+----------+----------+             |\r\n");
                    printf("|                                                             |\r\n");
                    
                    /*---------------------------------------------------------
                     * 第四部分：电机输出
                     *---------------------------------------------------------*/
                    printf("|  >>> MOTOR OUTPUT <<<                                       |\r\n");
                    printf("|                                                             |\r\n");
                    printf("|  Motor_Left:       %10d  (%s)                              |\r\n", 
                           Motor_Left, Motor_Left > 0 ? "Forward" : 
                           Motor_Left < 0 ? "Backward" : "Stop");
                    printf("|  Motor_Right:      %10d  (%s)                              |\r\n", 
                           Motor_Right, Motor_Right > 0 ? "Forward" : 
                           Motor_Right < 0 ? "Backward" : "Stop");
                    printf("|  PWM_Left_Abs:     %10d                                    |\r\n", 
                           myabs(Motor_Left));
                    printf("|  PWM_Right_Abs:    %10d                                    |\r\n", 
                           myabs(Motor_Right));
                    printf("|  PWM_Diff:         %10d  (L-R)                             |\r\n", 
                           Motor_Left - Motor_Right);
                    printf("|                                                             |\r\n");
                    
                    /*---------------------------------------------------------
                     * 第五部分：系统状态
                     *---------------------------------------------------------*/
                    printf("|  >>> SYSTEM STATUS <<<                                      |\r\n");
                    printf("|                                                             |\r\n");
                    printf("|  Stop_Flag:        %10d  (%s)                              |\r\n", 
                           Stop_Flag, Stop_Flag ? "STOPPED" : "RUNNING");
                    printf("|  Mid_Angle:        %10d                                    |\r\n", Mid_Angle);
                    printf("|  Move_X:           %10.1f  (Forward/Backward)              |\r\n", Move_X);
                    printf("|  Move_Z:           %10.1f  (Turn)                          |\r\n", Move_Z);
                    printf("|  Battery:          %10.2f V                                |\r\n", battery);
                    printf("|  Angle_Algorithm:  %10d                                    |\r\n", GET_Angle_Way);
                    printf("|                                                             |\r\n");
                    printf("+=============================================================+\r\n");
                }
                
                /*==============================================================
                 * 每10秒输出一次统计汇总
                 *==============================================================*/
                if(Print_Count % 50 == 0)
                {
                    float avg_angle = Angle_Sum / Angle_Sample_Count;
                    
                    printf("\r\n");
                    printf("+=================== 10-SECOND SUMMARY =====================+\r\n");
                    printf("|  Elapsed: %lu ms  |  Samples: %lu                          |\r\n", 
                           Print_Count * 200, Print_Count);
                    printf("+-----------------------------------------------------------+\r\n");
                    printf("|  Angle:     Avg=%.2f  Max=%.2f  Min=%.2f deg             |\r\n", 
                           avg_angle, Angle_Max, Angle_Min);
                    printf("|  Gyro_Y:    %.2f deg/s                                    |\r\n", Gyro_Balance);
                    printf("|  Gyro_Z:    %.2f deg/s                                    |\r\n", Gyro_Turn);
                    printf("|  Vel_Avg:   %.2f mm/s                                     |\r\n", 
                           (Velocity_Left + Velocity_Right) / 2.0f);
                    printf("|  Motor:     L=%d  R=%d                                |\r\n", 
                           Motor_Left, Motor_Right);
                    printf("+-----------------------------------------------------------+\r\n");
                    printf("|  PID Parameters (Actual Values):                          |\r\n");
                    printf("|    Balance:  P=%.2f  D=%.2f                              |\r\n", 
                           Balance_Kp/100.0f, Balance_Kd/100.0f);
                    printf("|    Velocity: P=%.2f  I=%.3f                              |\r\n", 
                           Velocity_Kp/100.0f, Velocity_Ki/100.0f);
                    printf("|    Turn:     P=%.2f  D=%.2f                              |\r\n", 
                           Turn_Kp/100.0f, Turn_Kd/100.0f);
                    printf("+-----------------------------------------------------------+\r\n");
                    printf("|  PID Tuning Suggestions:                                  |\r\n");
                    printf("|    Oscillation  -> Decrease Balance_Kp or Balance_Kd      |\r\n");
                    printf("|    Slow Drift   -> Increase Velocity_Ki                   |\r\n");
                    printf("|    Wobbling     -> Decrease Balance_Kp                    |\r\n");
                    printf("|    Hard to Turn -> Increase Turn_Kp                       |\r\n");
                    printf("|    Shaking      -> Increase Balance_Kd                    |\r\n");
                    printf("+===========================================================+\r\n\r\n");
                    
                    /* 重置统计数据 */
                    Angle_Max = Angle_Balance;
                    Angle_Min = Angle_Balance;
                    Angle_Sum = 0.0f;
                    Angle_Sample_Count = 0;
                }
            }
        }
        
        delay_ms(5);
    }
}