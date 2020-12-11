# Todo:

## 整理tiny3D的函数
- device
  - void device_init(device_t *device);
  - void device_set_background(device_t *device, IUINT32 color);
  - void device_set_framebuffer(device_t *device, IUINT32 *framebuffer);
  - void device_set_zbuffer(device_t *device, float *zbuffer);
  - void device_set_shadowbuffer(device_t *device, float *shadowbuffer);
  - void device_set_camera(device_t *device, camera_t *camera);
  - void device_pixel(device_t *device, int x, int y, IUINT32 color);
  - void device_clear(device_t *device);
- matrix和vector运算（先用eigen）
  - 旋转
  - 正交基变换
  - look at
  - 透视变换
- transform
- 颜色
- material
- 光源
- camera
- texture先不写（包括纹理坐标），插值的写法
- a2v和v2f，包括shader
- 多边形裁剪

## 

## 要写的ver3
### 第一阶段
- 以此为准（最新），tinyobj读的差不多了，写的东西有点多，先仿games101的作业3框架写一个，再在这个基础上改。
### 第一阶段
- 照着抄main.c
- 旋转改四元数，矩阵和向量运算用eigen
- 先不用class写，就用原有的
### 第二阶段
- world
- entity
- material
- camera
- resource manager
- 先不管硬件渲染怎么写