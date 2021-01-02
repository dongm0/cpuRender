# cpuRender

仿照[tiny3D](https://github.com/sdlwlxf1/tinyEngine)写一个软光栅渲染器。

## 阶段1

终于搞出来一个能跑的版本，大体上抄的Games101的框架，什么功能都没有。
- 改用扫描线画三角形
- 在SDL_log里显示帧率（好慢啊）

### Todo

- 三角形裁剪
- 背面剔除
- 画线段
- （可能的）上采样
- 边缘判断
- 其余（纹理，光照，shader，等等与管线关系不大的放到下次写gpu渲染器的时候写）
