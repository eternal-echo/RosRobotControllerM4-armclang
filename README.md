# RosRobotControllerM4
## 环境
安装MDK，使用keygen破解，如果是新版keil不自带armcc5，需要检查，没有需要自行去官网下载，建议照着资料的安装。

## 使用
打开`RosRobotControllerM4\MDK-ARM\RosRobotControllerM4.uvprojx`路径下的工程文件，编译并下载到STM32F407ZGT6开发板上。

## 硬件外设
- 串口：huart3，用于与ROS通信。注意，波特率为`1000000`。
- 日志串口：huart1，用于输出日志。波特率为`115200`。

## 配置项
- `Hiwonder\Misc\syscall.c`：修改`stdout_putchar`函数重定向到串口1，用于输出日志。
- `Hiwonder\System\global_conf.h`：配置是否使用IMU

## 工程结构
- System：
    - app.c：最上层的应用代码，这里包含了差速小车的电机控制逻辑，后续需要修改
    - global_conf.h：配置log等
    - [ ] packet_handle.c在例程里，待添加。
- Misc: 小车用到的通用库，包含日志、PID、包格式等。
    - packet.h：与上位机的通信协议处理。
        - 依赖的硬件接口：`send_packet`，该系统的硬件实现为向`packet_tx_queueHandle`队列发送数据，然后创建的任务`packet_tx_task_entry`会持续读取该队列的数据，统一发送到串口。
            - Tips：采用消息队列是为了避免系统的多个任务同时直接访问串口传输数据，导致乱码甚至hardfault。
        - 对外提供的接口：
            - `packet_transmit`：发送数据到串口，其他任务（如传感器任务）可以调用该接口发送数据到ROS对应的串口。想要知道发送了哪些数据，搜索这个函数即可。
            - `packet_recv`：接收数据，packet_rx_task_entry任务里持续调用该接口解析数据。在调用之前，已经通过串口DMA中断回调packet_dma_receive_event_callback读取了数据到缓冲区。
## 任务总览

### imu任务
`void imu_task_entry(void *argument)`：IMU任务，负责读取IMU数据，然后通过`packet_transmit`发送到ROS。
- 里面有日志，可以观察IMU数据是否正常。

### ROS串口通信任务
- `packet_tx_task_entry`：将数据通过huart3发送到ROS，在`Hiwonder\Portings\packet_porting.c`中实现
    - `packet_tx_idleHandle`信号量：用于检测串口DMA的硬件发送操作是否结束
    - `packet_tx_queueHandle`：获取串口数据的队列