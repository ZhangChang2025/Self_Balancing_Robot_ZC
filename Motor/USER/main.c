/**
* @par Copyright (C): 2018-2028, Shenzhen Yahboom Tech
* @file         // main.c
* @author       // lly
* @version      // V1.0
* @date         // 240628
* @brief        // 电机校准实验 - 测试左右电机转速差异并计算补偿系数
* @details      // 分别测试30%、50%、70%、90%占空比下的转速差异
* @par History  // 修改历史记录
*/

#include "AllHeader.h"

/* 校准参数定义 */
#define CALIB_SAMPLE_TIME    500      // 每个占空比测试采样时间(ms)
#define CALIB_STABLE_TIME    300      // 电机稳定运行等待时间(ms)
#define CALIB_DUTY_COUNT     4        // 测试占空比档位数量
#define CALIB_SPEED_COUNT    5        // 每个档位采集速度样本数量

/* 校准状态枚举 */
typedef enum {
    CALIB_STATE_IDLE = 0,          // 空闲/等待按键
    CALIB_STATE_START,             // 开始校准
    CALIB_STATE_FORWARD_STABLE,    // 正向稳定等待
    CALIB_STATE_FORWARD_MEASURE,   // 正向测量中
    CALIB_STATE_BACKWARD_STOP,     // 反向切换停止
    CALIB_STATE_BACKWARD_STABLE,   // 反向稳定等待
    CALIB_STATE_BACKWARD_MEASURE,  // 反向测量中
    CALIB_STATE_NEXT_DUTY,         // 切换到下一个占空比
    CALIB_STATE_DONE,              // 校准完成
    CALIB_STATE_COMPENSATE_TEST    // 补偿效果验证
} CalibState_e;

/* 速度数据结构 */
typedef struct {
    int16_t left_speed;            // 左轮转速(编码器脉冲/采样周期)
    int16_t right_speed;           // 右轮转速(编码器脉冲/采样周期)
    float left_rpm;                // 左轮转速(RPM)
    float right_rpm;               // 右轮转速(RPM)
    float ratio;                   // 左右转速比(L/R)
} SpeedData_t;

/* 校准数据结构 */
typedef struct {
    uint16_t duty_cycle;           // 占空比值(PWM值)
    SpeedData_t forward_data[CALIB_SPEED_COUNT];   // 正向数据
    SpeedData_t backward_data[CALIB_SPEED_COUNT];  // 反向数据
    float avg_forward_ratio;       // 正向平均转速比
    float avg_backward_ratio;      // 反向平均转速比
    float avg_forward_left_rpm;    // 正向左轮平均RPM
    float avg_forward_right_rpm;   // 正向右轮平均RPM
    float avg_backward_left_rpm;   // 反向左轮平均RPM
    float avg_backward_right_rpm;  // 反向右轮平均RPM
} CalibData_t;

/* 全局变量 */
CalibState_e calib_state = CALIB_STATE_IDLE;
uint8_t duty_index = 0;                          // 当前测试占空比索引
uint8_t sample_index = 0;                        // 当前采样索引
uint8_t backward_sample_index = 0;               // 反向采样索引
uint8_t backward_skip_count = 0;                 // 反向跳过计数
uint32_t state_timer = 0;                        // 状态计时器
uint32_t sample_timer = 0;                       // 采样间隔计时器
int16_t Encoder_Left = 0, Encoder_Right = 0;     // 编码器累计值

/* 测试占空比数组 (对应30%, 50%, 70%, 90%) */
/* PWM范围假设为0-7200，可根据实际调整 */
const uint16_t calib_duty_table[CALIB_DUTY_COUNT] = {2160, 3600, 5040, 6480};

/* 校准数据存储 */
CalibData_t calib_data[CALIB_DUTY_COUNT];

/* 补偿系数 (校准完成后使用) */
float g_compensation_left = 1.0f;
float g_compensation_right = 1.0f;
uint8_t calibration_complete = 0;

/* 验证测试步数 */
uint8_t compensate_test_step = 0;

/* 电机参数(用于RPM计算，需根据实际编码器线数调整) */
#define ENCODER_PPR         11      // 编码器每转脉冲数(电机一圈)
#define GEAR_RATIO          30      // 减速比
#define PULSES_PER_REV      (ENCODER_PPR * GEAR_RATIO)  // 轮子每转脉冲数
#define SAMPLE_PERIOD_MS    100     // 采样周期(ms)

/* 函数声明 */
void Calib_Reset(void);
void Calib_Process(void);
void Calib_PrintResults(void);
void Calib_TestCompensation(void);
int16_t Read_Speed_Left(void);
int16_t Read_Speed_Right(void);
float Calculate_RPM(int16_t pulses, uint16_t period_ms);
void Set_Pwm_Compensated(int16_t motor_left, int16_t motor_right);
void Print_RealTime_Speed(void);

/******************************************************************************
* 函数名  : main
* 描述    : 主函数 - 电机校准实验入口
* 输入    : 无
* 输出    : 无
******************************************************************************/
int main(void)
{	
    bsp_init();
    
    /* 打印校准实验信息 */
    printf("\r\n");
    printf("========================================\r\n");
    printf("    Motor Calibration Experiment       \r\n");
    printf("========================================\r\n");
    printf("Test Duty Cycles: 30%%, 50%%, 70%%, 90%%\r\n");
    printf("Encoder PPR: %d, Gear Ratio: %d\r\n", ENCODER_PPR, GEAR_RATIO);
    printf("Pulses Per Wheel Rev: %d\r\n", PULSES_PER_REV);
    printf("========================================\r\n");
    printf("Press KEY1 to start calibration...\r\n");
    printf("========================================\r\n\r\n");
    
    while(1)
    {
        /* 编码器累计读取 */
        Encoder_Left += Read_Encoder(MOTOR_ID_ML);
        Encoder_Right += -Read_Encoder(MOTOR_ID_MR);
        
        /* 实时打印编码器累计值和当前转速 */
        Print_RealTime_Speed();
        
        /* 按键检测 */
        if(Key1_State(1))
        {
            printf("\r\n>>> [KEY PRESSED] <<<\r\n");
            
            if(calib_state == CALIB_STATE_IDLE)
            {
                /* 空闲状态，开始校准 */
                Calib_Reset();
                calib_state = CALIB_STATE_START;
                printf("\r\n>>> Starting Motor Calibration...\r\n\r\n");
            }
            else if(calib_state == CALIB_STATE_DONE)
            {
                /* 校准完成，切换到补偿验证 */
                compensate_test_step = 0;
                calib_state = CALIB_STATE_COMPENSATE_TEST;
                printf("\r\n>>> Switching to Compensation Test Mode...\r\n");
                Calib_TestCompensation();
            }
            else if(calib_state == CALIB_STATE_COMPENSATE_TEST)
            {
                /* 验证模式下切换测试步骤 */
                compensate_test_step++;
                Calib_TestCompensation();
            }
        }
        
        /* 校准状态机处理 */
        Calib_Process();
        
        delay_ms(50);
    }
}

/******************************************************************************
* 函数名  : Print_RealTime_Speed
* 描述    : 实时打印编码器累计值和当前电机转速
* 输入    : 无
* 输出    : 无
******************************************************************************/
void Print_RealTime_Speed(void)
{
    static uint32_t last_print_time = 0;
    static int16_t last_left_pulses = 0, last_right_pulses = 0;
    int16_t current_left, current_right;
    int16_t delta_left, delta_right;
    float left_rpm, right_rpm;
    
    /* 每200ms打印一次 */
    if(state_timer - last_print_time >= 200 || last_print_time == 0)
    {
        last_print_time = state_timer;
        
        current_left = Read_Speed_Left();
        current_right = Read_Speed_Right();
        
        /* 计算机器周期内的脉冲差(用于RPM估算) */
        delta_left = current_left - last_left_pulses;
        delta_right = current_right - last_right_pulses;
        
        left_rpm = Calculate_RPM(myabs(delta_left), 200);
        right_rpm = Calculate_RPM(myabs(delta_right), 200);
        
        /* 带符号的转速 */
        if(delta_left < 0) left_rpm = -left_rpm;
        if(delta_right < 0) right_rpm = -right_rpm;
        
        printf("[RT] Enc_L:%d Enc_R:%d | Speed_L:%.1f RPM Speed_R:%.1f RPM\r\n",
               Encoder_Left, Encoder_Right, left_rpm, right_rpm);
        
        last_left_pulses = current_left;
        last_right_pulses = current_right;
    }
}

/******************************************************************************
* 函数名  : Calculate_RPM
* 描述    : 根据脉冲数计算转速(RPM)
* 输入    : pulses - 采样周期内的脉冲数
*          period_ms - 采样周期(ms)
* 输出    : 转速(RPM)
******************************************************************************/
float Calculate_RPM(int16_t pulses, uint16_t period_ms)
{
    float revolutions;
    float minutes;
    
    if(pulses == 0 || period_ms == 0)
        return 0.0f;
    
    /* 脉冲数 / 每转脉冲数 = 转数 */
    revolutions = (float)pulses / (float)PULSES_PER_REV;
    
    /* 采样周期转换为分钟 */
    minutes = (float)period_ms / 60000.0f;
    
    /* RPM = 转数 / 分钟数 */
    return revolutions / minutes;
}

/******************************************************************************
* 函数名  : Calib_Process
* 描述    : 校准状态机处理函数
* 输入    : 无
* 输出    : 无
******************************************************************************/
void Calib_Process(void)
{
    int16_t raw_left, raw_right;
    float left_rpm, right_rpm;
    
    switch(calib_state)
    {
        case CALIB_STATE_IDLE:
        case CALIB_STATE_DONE:
            /* 空闲/完成状态，更新计时器用于实时打印 */
            state_timer += 50;
            break;
            
        case CALIB_STATE_COMPENSATE_TEST:
            /* 补偿验证模式 */
            state_timer += 50;
            break;
            
        case CALIB_STATE_START:
            /* 初始化当前占空比测试 */
            printf("========================================\r\n");
            printf("Testing Duty: %d/7200 (%.0f%%)\r\n",
                   calib_duty_table[duty_index],
                   (float)calib_duty_table[duty_index] / 72.0f);
            printf("Direction: FORWARD\r\n");
            printf("Stabilizing motor for %dms...\r\n", CALIB_STABLE_TIME);
            
            Set_Pwm(calib_duty_table[duty_index], calib_duty_table[duty_index]);
            state_timer = 0;
            sample_index = 0;
            calib_state = CALIB_STATE_FORWARD_STABLE;
            break;
            
        case CALIB_STATE_FORWARD_STABLE:
            /* 等待电机稳定 */
            state_timer += 50;
            if(state_timer >= CALIB_STABLE_TIME)
            {
                state_timer = 0;
                sample_timer = 0;
                sample_index = 0;
                printf("Motor stabilized. Start forward sampling...\r\n");
                printf("Sample period: %dms, Total samples: %d\r\n\r\n",
                       CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT, CALIB_SPEED_COUNT);
                calib_state = CALIB_STATE_FORWARD_MEASURE;
            }
            break;
            
        case CALIB_STATE_FORWARD_MEASURE:
            /* 正向速度采样 */
            state_timer += 50;
            sample_timer += 50;
            
            if(sample_timer >= (CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT))
            {
                sample_timer = 0;
                
                /* 读取当前速度 */
                raw_left = Read_Speed_Left();
                raw_right = Read_Speed_Right();
                
                calib_data[duty_index].forward_data[sample_index].left_speed = raw_left;
                calib_data[duty_index].forward_data[sample_index].right_speed = raw_right;
                
                /* 计算RPM */
                left_rpm = Calculate_RPM(myabs(raw_left), CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT);
                right_rpm = Calculate_RPM(myabs(raw_right), CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT);
                
                calib_data[duty_index].forward_data[sample_index].left_rpm = left_rpm;
                calib_data[duty_index].forward_data[sample_index].right_rpm = right_rpm;
                
                /* 计算转速比 */
                if(raw_right != 0)
                {
                    calib_data[duty_index].forward_data[sample_index].ratio =
                        (float)raw_left / (float)raw_right;
                }
                else
                {
                    calib_data[duty_index].forward_data[sample_index].ratio = 1.0f;
                }
                
                printf("  [FWD Sample %d/%d] L_Pulses:%d R_Pulses:%d | "
                       "L_RPM:%.1f R_RPM:%.1f | Ratio(L/R):%.4f\r\n",
                       sample_index + 1, CALIB_SPEED_COUNT,
                       raw_left, raw_right,
                       left_rpm, right_rpm,
                       calib_data[duty_index].forward_data[sample_index].ratio);
                
                sample_index++;
                if(sample_index >= CALIB_SPEED_COUNT)
                {
                    /* 正向测量完成 */
                    printf("\r\n>>> Forward measurement complete!\r\n");
                    printf(">>> Stopping motor briefly...\r\n\r\n");
                    Set_Pwm(0, 0);
                    state_timer = 0;
                    calib_state = CALIB_STATE_BACKWARD_STOP;
                }
            }
            break;
            
        case CALIB_STATE_BACKWARD_STOP:
            /* 短暂停止后开始反向测试 */
            state_timer += 50;
            if(state_timer >= 500)
            {
                state_timer = 0;
                backward_sample_index = 0;
                backward_skip_count = 0;
                sample_timer = 0;
                
                printf("Direction: BACKWARD\r\n");
                printf("Stabilizing motor for %dms...\r\n", CALIB_STABLE_TIME);
                
                Set_Pwm(-calib_duty_table[duty_index], -calib_duty_table[duty_index]);
                calib_state = CALIB_STATE_BACKWARD_STABLE;
            }
            break;
            
        case CALIB_STATE_BACKWARD_STABLE:
            /* 反向稳定等待 */
            state_timer += 50;
            if(state_timer >= CALIB_STABLE_TIME)
            {
                state_timer = 0;
                sample_timer = 0;
                printf("Motor stabilized. Start backward sampling...\r\n");
                printf("Sample period: %dms, Total samples: %d\r\n\r\n",
                       CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT, CALIB_SPEED_COUNT);
                calib_state = CALIB_STATE_BACKWARD_MEASURE;
            }
            break;
            
        case CALIB_STATE_BACKWARD_MEASURE:
            /* 反向速度采样 */
            state_timer += 50;
            sample_timer += 50;
            
            if(sample_timer >= (CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT))
            {
                sample_timer = 0;
                backward_skip_count++;
                
                /* 跳过前3个不稳定样本 */
                if(backward_skip_count > 3 && backward_sample_index < CALIB_SPEED_COUNT)
                {
                    raw_left = myabs(Read_Speed_Left());
                    raw_right = myabs(Read_Speed_Right());
                    
                    calib_data[duty_index].backward_data[backward_sample_index].left_speed = raw_left;
                    calib_data[duty_index].backward_data[backward_sample_index].right_speed = raw_right;
                    
                    /* 计算RPM */
                    left_rpm = Calculate_RPM(raw_left, CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT);
                    right_rpm = Calculate_RPM(raw_right, CALIB_SAMPLE_TIME / CALIB_SPEED_COUNT);
                    
                    calib_data[duty_index].backward_data[backward_sample_index].left_rpm = left_rpm;
                    calib_data[duty_index].backward_data[backward_sample_index].right_rpm = right_rpm;
                    
                    /* 计算转速比 */
                    if(raw_right != 0)
                    {
                        calib_data[duty_index].backward_data[backward_sample_index].ratio =
                            (float)raw_left / (float)raw_right;
                    }
                    else
                    {
                        calib_data[duty_index].backward_data[backward_sample_index].ratio = 1.0f;
                    }
                    
                    printf("  [BWD Sample %d/%d] L_Pulses:%d R_Pulses:%d | "
                           "L_RPM:%.1f R_RPM:%.1f | Ratio(L/R):%.4f\r\n",
                           backward_sample_index + 1, CALIB_SPEED_COUNT,
                           raw_left, raw_right,
                           left_rpm, right_rpm,
                           calib_data[duty_index].backward_data[backward_sample_index].ratio);
                    
                    backward_sample_index++;
                }
                
                if(backward_sample_index >= CALIB_SPEED_COUNT)
                {
                    /* 反向测量完成 */
                    printf("\r\n>>> Backward measurement complete!\r\n");
                    Set_Pwm(0, 0);
                    state_timer = 0;
                    calib_state = CALIB_STATE_NEXT_DUTY;
                }
            }
            break;
            
        case CALIB_STATE_NEXT_DUTY:
            /* 切换到下一个占空比 */
            state_timer += 50;
            if(state_timer >= 800)
            {
                state_timer = 0;
                duty_index++;
                
                if(duty_index < CALIB_DUTY_COUNT)
                {
                    printf("\r\n");
                    printf(">>> Switching to next duty cycle... <<<\r\n\r\n");
                    calib_state = CALIB_STATE_START;
                }
                else
                {
                    /* 所有占空比测试完成 */
                    calib_state = CALIB_STATE_DONE;
                    calibration_complete = 1;
                    
                    printf("\r\n");
                    printf("========================================\r\n");
                    printf("      CALIBRATION COMPLETE!             \r\n");
                    printf("========================================\r\n");
                    
                    Calib_PrintResults();
                    
                    printf("\r\n");
                    printf("Press KEY1 to enter Compensation Test Mode...\r\n");
                    printf("========================================\r\n\r\n");
                }
            }
            break;
            
        default:
            break;
    }
}

/******************************************************************************
* 函数名  : Calib_PrintResults
* 描述    : 打印校准结果并计算补偿系数
* 输入    : 无
* 输出    : 无
******************************************************************************/
void Calib_PrintResults(void)
{
    float total_forward_ratio = 0;
    float total_backward_ratio = 0;
    float total_forward_left_rpm = 0, total_forward_right_rpm = 0;
    float total_backward_left_rpm = 0, total_backward_right_rpm = 0;
    uint8_t i, j;
    
    printf("\r\n");
    printf("================== CALIBRATION DATA ==================\r\n");
    printf("Duty  | Dir |  L_RPM   |  R_RPM   | Diff(RPM) | Ratio\r\n");
    printf("------------------------------------------------------\r\n");
    
    for(i = 0; i < CALIB_DUTY_COUNT; i++)
    {
        /* 计算正向平均数据 */
        float forward_ratio_sum = 0;
        float forward_left_rpm_sum = 0, forward_right_rpm_sum = 0;
        
        for(j = 0; j < CALIB_SPEED_COUNT; j++)
        {
            forward_ratio_sum += calib_data[i].forward_data[j].ratio;
            forward_left_rpm_sum += calib_data[i].forward_data[j].left_rpm;
            forward_right_rpm_sum += calib_data[i].forward_data[j].right_rpm;
        }
        
        calib_data[i].avg_forward_ratio = forward_ratio_sum / CALIB_SPEED_COUNT;
        calib_data[i].avg_forward_left_rpm = forward_left_rpm_sum / CALIB_SPEED_COUNT;
        calib_data[i].avg_forward_right_rpm = forward_right_rpm_sum / CALIB_SPEED_COUNT;
        
        /* 计算反向平均数据 */
        float backward_ratio_sum = 0;
        float backward_left_rpm_sum = 0, backward_right_rpm_sum = 0;
        
        for(j = 0; j < CALIB_SPEED_COUNT; j++)
        {
            backward_ratio_sum += calib_data[i].backward_data[j].ratio;
            backward_left_rpm_sum += calib_data[i].backward_data[j].left_rpm;
            backward_right_rpm_sum += calib_data[i].backward_data[j].right_rpm;
        }
        
        calib_data[i].avg_backward_ratio = backward_ratio_sum / CALIB_SPEED_COUNT;
        calib_data[i].avg_backward_left_rpm = backward_left_rpm_sum / CALIB_SPEED_COUNT;
        calib_data[i].avg_backward_right_rpm = backward_right_rpm_sum / CALIB_SPEED_COUNT;
        
        /* 打印正向结果 */
        printf("%.0f%%  | FWD | %8.1f | %8.1f | %8.1f | %.4f\r\n",
               (float)calib_duty_table[i] / 72.0f,
               calib_data[i].avg_forward_left_rpm,
               calib_data[i].avg_forward_right_rpm,
               calib_data[i].avg_forward_left_rpm - calib_data[i].avg_forward_right_rpm,
               calib_data[i].avg_forward_ratio);
        
        /* 打印反向结果 */
        printf("      | BWD | %8.1f | %8.1f | %8.1f | %.4f\r\n",
               calib_data[i].avg_backward_left_rpm,
               calib_data[i].avg_backward_right_rpm,
               calib_data[i].avg_backward_left_rpm - calib_data[i].avg_backward_right_rpm,
               calib_data[i].avg_backward_ratio);
        
        if(i < CALIB_DUTY_COUNT - 1)
            printf("------------------------------------------------------\r\n");
        
        total_forward_ratio += calib_data[i].avg_forward_ratio;
        total_backward_ratio += calib_data[i].avg_backward_ratio;
        total_forward_left_rpm += calib_data[i].avg_forward_left_rpm;
        total_forward_right_rpm += calib_data[i].avg_forward_right_rpm;
        total_backward_left_rpm += calib_data[i].avg_backward_left_rpm;
        total_backward_right_rpm += calib_data[i].avg_backward_right_rpm;
    }
    
    printf("======================================================\r\n");
    
    /* 计算综合平均值 */
    float avg_forward_ratio = total_forward_ratio / CALIB_DUTY_COUNT;
    float avg_backward_ratio = total_backward_ratio / CALIB_DUTY_COUNT;
    float overall_ratio = (avg_forward_ratio + avg_backward_ratio) / 2.0f;
    
    float avg_fwd_l_rpm = total_forward_left_rpm / CALIB_DUTY_COUNT;
    float avg_fwd_r_rpm = total_forward_right_rpm / CALIB_DUTY_COUNT;
    float avg_bwd_l_rpm = total_backward_left_rpm / CALIB_DUTY_COUNT;
    float avg_bwd_r_rpm = total_backward_right_rpm / CALIB_DUTY_COUNT;
    
    printf("\r\n================== SUMMARY ==================\r\n");
    printf("Overall Forward Ratio (L/R):  %.4f\r\n", avg_forward_ratio);
    printf("Overall Backward Ratio (L/R): %.4f\r\n", avg_backward_ratio);
    printf("Overall Average Ratio (L/R):  %.4f\r\n", overall_ratio);
    printf("\r\n");
    printf("Avg Forward  Left RPM:  %.1f\r\n", avg_fwd_l_rpm);
    printf("Avg Forward  Right RPM: %.1f\r\n", avg_fwd_r_rpm);
    printf("Avg Backward Left RPM:  %.1f\r\n", avg_bwd_l_rpm);
    printf("Avg Backward Right RPM: %.1f\r\n", avg_bwd_r_rpm);
    printf("=============================================\r\n");
    
    /* 计算补偿系数 */
    if(overall_ratio >= 1.0f)
    {
        /* 左轮快于右轮，降低左轮PWM */
        g_compensation_left = 1.0f / overall_ratio;
        g_compensation_right = 1.0f;
        printf("\r\n>>> Left motor is FASTER (%.1f%%), reducing left PWM\r\n",
               (overall_ratio - 1.0f) * 100.0f);
    }
    else
    {
        /* 右轮快于左轮，降低右轮PWM */
        g_compensation_left = 1.0f;
        g_compensation_right = overall_ratio;
        printf("\r\n>>> Right motor is FASTER (%.1f%%), reducing right PWM\r\n",
               (1.0f - overall_ratio) * 100.0f);
    }
    
    printf("\r\n========== COMPENSATION COEFFICIENTS ==========\r\n");
    printf("Left Motor Compensation:  %.4f\r\n", g_compensation_left);
    printf("Right Motor Compensation: %.4f\r\n", g_compensation_right);
    printf("Compensated PWM Formula:\r\n");
    printf("  Left_PWM  = Original_PWM * %.4f\r\n", g_compensation_left);
    printf("  Right_PWM = Original_PWM * %.4f\r\n", g_compensation_right);
    printf("==============================================\r\n");
}

/******************************************************************************
* 函数名  : Calib_TestCompensation
* 描述    : 补偿效果验证测试
* 输入    : 无
* 输出    : 无
******************************************************************************/
void Calib_TestCompensation(void)
{
    int16_t pwm_base = 3600;  // 50%占空比用于验证
    
    /* 循环切换: 停止 -> 无补偿前进 -> 停止 -> 补偿前进 -> 停止 */
    if(compensate_test_step >= 5)
        compensate_test_step = 0;
    
    printf("\r\n========== COMPENSATION TEST ==========\r\n");
    
    switch(compensate_test_step)
    {
        case 0:
            Set_Pwm(0, 0);
            printf("Step %d: MOTOR STOP\r\n", compensate_test_step);
            printf("Encoder Reset: L=%d, R=%d\r\n", Encoder_Left, Encoder_Right);
            break;
            
        case 1:
            /* 无补偿前进 */
            Set_Pwm(pwm_base, pwm_base);
            printf("Step %d: FORWARD (No Compensation)\r\n", compensate_test_step);
            printf("  Left_PWM = %d, Right_PWM = %d\r\n", pwm_base, pwm_base);
            printf("  Observe: Car may deviate from straight line\r\n");
            break;
            
        case 2:
            Set_Pwm(0, 0);
            printf("Step %d: MOTOR STOP\r\n", compensate_test_step);
            printf("Encoder after no-comp test: L=%d, R=%d\r\n",
                   Encoder_Left, Encoder_Right);
            break;
            
        case 3:
            /* 补偿后前进 */
            Set_Pwm_Compensated(pwm_base, pwm_base);
            printf("Step %d: FORWARD (With Compensation)\r\n", compensate_test_step);
            printf("  Left_PWM = %d, Right_PWM = %d\r\n",
                   (int16_t)(pwm_base * g_compensation_left),
                   (int16_t)(pwm_base * g_compensation_right));
            printf("  Observe: Car should go straight!\r\n");
            break;
            
        case 4:
            Set_Pwm(0, 0);
            printf("Step %d: MOTOR STOP\r\n", compensate_test_step);
            printf("Encoder after comp test: L=%d, R=%d\r\n",
                   Encoder_Left, Encoder_Right);
            printf("\r\nPress KEY1 to cycle through test steps...\r\n");
            break;
    }
    
    printf("========================================\r\n\r\n");
}

/******************************************************************************
* 函数名  : Set_Pwm_Compensated
* 描述    : 带补偿的PWM设置函数
* 输入    : motor_left  - 左轮目标PWM
*          motor_right - 右轮目标PWM
* 输出    : 无
******************************************************************************/
void Set_Pwm_Compensated(int16_t motor_left, int16_t motor_right)
{
    if(calibration_complete)
    {
        motor_left = (int16_t)((float)motor_left * g_compensation_left);
        motor_right = (int16_t)((float)motor_right * g_compensation_right);
    }
    Set_Pwm(motor_left, motor_right);
}

/******************************************************************************
* 函数名  : Read_Speed_Left
* 描述    : 读取左轮瞬时速度
* 输入    : 无
* 输出    : 左轮编码器读数(带符号)
******************************************************************************/
int16_t Read_Speed_Left(void)
{
    return (int16_t)Read_Encoder(MOTOR_ID_ML);
}

/******************************************************************************
* 函数名  : Read_Speed_Right
* 描述    : 读取右轮瞬时速度(已取反，正值表示前进)
* 输入    : 无
* 输出    : 右轮编码器读数(带符号)
******************************************************************************/
int16_t Read_Speed_Right(void)
{
    return (int16_t)(-Read_Encoder(MOTOR_ID_MR));
}

/******************************************************************************
* 函数名  : Calib_Reset
* 描述    : 重置校准数据
* 输入    : 无
* 输出    : 无
******************************************************************************/
void Calib_Reset(void)
{
    uint8_t i, j;
    
    duty_index = 0;
    sample_index = 0;
    backward_sample_index = 0;
    backward_skip_count = 0;
    state_timer = 0;
    sample_timer = 0;
    compensate_test_step = 0;
    calibration_complete = 0;
    g_compensation_left = 1.0f;
    g_compensation_right = 1.0f;
    
    for(i = 0; i < CALIB_DUTY_COUNT; i++)
    {
        calib_data[i].duty_cycle = calib_duty_table[i];
        calib_data[i].avg_forward_ratio = 0;
        calib_data[i].avg_backward_ratio = 0;
        calib_data[i].avg_forward_left_rpm = 0;
        calib_data[i].avg_forward_right_rpm = 0;
        calib_data[i].avg_backward_left_rpm = 0;
        calib_data[i].avg_backward_right_rpm = 0;
        
        for(j = 0; j < CALIB_SPEED_COUNT; j++)
        {
            calib_data[i].forward_data[j].left_speed = 0;
            calib_data[i].forward_data[j].right_speed = 0;
            calib_data[i].forward_data[j].left_rpm = 0;
            calib_data[i].forward_data[j].right_rpm = 0;
            calib_data[i].forward_data[j].ratio = 0;
            
            calib_data[i].backward_data[j].left_speed = 0;
            calib_data[i].backward_data[j].right_speed = 0;
            calib_data[i].backward_data[j].left_rpm = 0;
            calib_data[i].backward_data[j].right_rpm = 0;
            calib_data[i].backward_data[j].ratio = 0;
        }
    }
    
    Encoder_Left = 0;
    Encoder_Right = 0;
}