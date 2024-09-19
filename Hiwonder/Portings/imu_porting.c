#include "global.h"
#include "log.h"
#include "lwmem_porting.h"
#include "packet_reports.h"
#include "global_conf.h"
#include "imu.h"

#include "driver_mpu6050_basic.h"

IMU_ObjectTypeDef imu;
int imu_report_interval = 50;

static void imu_reset(IMU_ObjectTypeDef *self);
static int imu_get_accel_gyro(IMU_ObjectTypeDef *self, float *xyz, float *gyro);

void imu_init(void)
{
    imu.reset = imu_reset;
    imu.get_accel_gyro = imu_get_accel_gyro;
}

#if ENABLE_IMU
/**
 * @brief  imu task 入口函数
 *
 */

void imu_task_entry(void *argument)
{
    // extern osSemaphoreId_t mpu6050_data_readyHandle;
    int res;
    imu_init();
    imu.reset(&imu);

    PacketReportIMU_Raw_TypeDef report;
    for(;;) {
        // osSemaphoreAcquire(mpu6050_data_readyHandle, osWaitForever);
        // imus[0]->update(imus[0]);
		// imus[0]->get_accel(imus[0], report.array.accel_array);
		// imus[0]->get_gyro(imus[0], report.array.gyro_array);
        res = imu.get_accel_gyro(&imu, report.array.accel_array, report.array.gyro_array);
        if (res != 0)
        {
            printf("get accel gyro failed.\n");
            continue;
        }
        packet_transmit(&packet_controller, PACKET_FUNC_IMU, &report, sizeof(PacketReportIMU_Raw_TypeDef));
        // LOG_DEBUG("IMU: %f %f %f %f %f %f\n", report.array.accel_array[0], report.array.accel_array[1], report.array.accel_array[2], report.array.gyro_array[0], report.array.gyro_array[1], report.array.gyro_array[2]);
        osDelay(imu_report_interval);
    }
}

#endif

static void imu_reset(IMU_ObjectTypeDef *self)
{
    uint8_t res;
    mpu6050_address_t addr = MPU6050_ADDRESS_AD0_LOW;
    res = mpu6050_basic_init(addr);

    if (res != 0)
    {
        /* handle error */
        printf("init failed.\n");
    }

    // return;
}

static int imu_get_accel_gyro(IMU_ObjectTypeDef *self, float *xyz, float *gyro)
{
    uint8_t res;
    /* 读取加速度和陀螺仪数据 */
    res = mpu6050_basic_read(xyz, gyro);
    if (res != 0)
    {
        printf("read failed.\n");
        return -1;
    }
    return 0;

    // // 填充测试数据
    // xyz[0] = 1.0;
    // xyz[1] = 2.0;
    // xyz[2] = 3.0;

    // gyro[0] = 4.0;
    // gyro[1] = 5.0;
    // gyro[2] = 6.0;
    // return 0;
}