# cpuRender

仿照[tiny3D](https://github.com/sdlwlxf1/tinyEngine)写一个软光栅渲染器。

## 现阶段

大体上抄的Games101的框架。
- 扫描线画三角形
- 背面剔除
- 矩阵运算用的eigen
- 最简单的裁减（只要在外面就裁），准备接下来抄[tiny3D](https://github.com/sdlwlxf1/tinyEngine)的裁剪

### Todo

- 三角形裁剪
- 画线段
- （可能的）上采样
- 边缘判断
- 其余（纹理，光照，shader，等等放到下次写gpu渲染器的时候写）