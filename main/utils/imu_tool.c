#include "imu_tool.h"
#include <math.h>

#define Euler_Kp 6950               //欧拉角补偿p
#define Euler_Ki 10             //欧拉角补偿i
#define Euler_Kd 0.000002f        //欧拉角补偿d

euler_angles u_calc_euler_angles(imu_data data)
{
  euler_angles eulaer;
  float roll, pitch,yaw ;
  float exInt=0, eyInt=0, ezInt=0;
  float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f; /** quaternion of sensor frame relative to auxiliary frame */

  float ax = data.acc.x;
  float ay = data.acc.y;
  float az = data.acc.z;
  float gx = data.gyr.x;
  float gy = data.gyr.y;
  float gz = data.gyr.z;


  float recipNorm;
  float vx, vy, vz;
  float ex, ey, ez;

  float q0q0 = q0 * q0;
  float q0q1 = q0 * q1;
  float q0q2 = q0 * q2;

  float q1q1 = q1 * q1;
  float q1q3 = q1 * q3;

  float q2q2 = q2 * q2;
  float q2q3 = q2 * q3;

  float q3q3 = q3 * q3;

  if (ax * ay * az == 0)
      return eulaer;
  /* 对加速度数据进行归一化处理 */
  recipNorm = 1/sqrt(ax * ax + ay * ay + az * az);
  ax = ax * recipNorm;
  ay = ay * recipNorm;
  az = az * recipNorm;
  /* DCM矩阵旋转 */
  vx = 2 * (q1q3 - q0q2);
  vy = 2 * (q0q1 + q2q3);
  vz = q0q0 - q1q1 - q2q2 + q3q3;
  /* 在机体坐标系下做向量叉积得到补偿数据 */
  ex = ay * vz - az * vy;
  ey = az * vx - ax * vz;
  ez = ax * vy - ay * vx;
  /* 对误差进行PI计算，补偿角速度 */
  exInt = exInt + ex * Euler_Ki;
  eyInt = eyInt + ey * Euler_Ki;
  ezInt = ezInt + ez * Euler_Ki;

  gx = gx + Euler_Kp * ex + exInt;
  gy = gy + Euler_Kp * ey + eyInt;
  gz = gz + Euler_Kp * ez + ezInt;
  /* 按照四元素微分公式进行四元素更新 */
  q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * Euler_Kd;
  q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * Euler_Kd;
  q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * Euler_Kd;
  q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * Euler_Kd;

  recipNorm = 1/sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);

  q0 = q0 * recipNorm;
  q1 = q1 * recipNorm;
  q2 = q2 * recipNorm;
  q3 = q3 * recipNorm;

  roll = atan2f(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3f;
  pitch = asinf(2 * q1 * q3 - 2 * q0 * q2) * 57.3f;
  yaw = -atan2f(2 * q1 * q2 + 2 * q0 * q3, -2 * q2 * q2 - 2 * q3 * q3 + 1) * 57.3f;

  eulaer.pitch = pitch;
  eulaer.roll = roll;
  eulaer.yaw = yaw;

  return eulaer;
}
