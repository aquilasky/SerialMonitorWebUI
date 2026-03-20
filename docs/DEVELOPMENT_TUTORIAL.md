# SerialMonitor 开发环境配置教程

本文档详细说明如何配置 SerialMonitor 前端开发环境。

## 前置要求

### 必需软件

| 软件 | 版本要求 | 下载地址 |
|------|----------|----------|
| Node.js | 18.x 或更高 | https://nodejs.org/ |
| npm | 9.x 或更高 | 随 Node.js 一起安装 |

### 验证安装

打开 PowerShell 终端，运行以下命令验证：

```powershell
node --version
npm --version
```

应该看到类似输出：
```
v18.17.0
9.6.7
```

## 环境配置步骤

### 1. 安装 Node.js

1. 访问 https://nodejs.org/
2. 下载 LTS 版本（推荐 v18.x 或 v20.x）
3. 运行安装程序
4. **重要**: 勾选 "Add to PATH" 选项
5. 完成安装后重启终端

### 2. 配置 npm 镜像（可选，中国用户推荐）

由于网络原因，建议配置国内镜像加速下载：

```powershell
# 设置淘宝镜像
npm config set registry https://registry.npmmirror.com

# 验证配置
npm config get registry
# 应该显示: https://registry.npmmirror.com
```

### 3. 克隆项目

```powershell
cd c:\Users\74659\Documents\trae_projects
git clone <repository-url> SerialMonitor
cd SerialMonitor
```

### 4. 安装前端依赖

```powershell
cd frontend
npm install
```

这将安装项目所需的所有依赖，包括：
- Vue 3
- Vite
- Element Plus
- Pinia
- 等等

### 5. 启动开发服务器

```powershell
cd frontend
npm run dev
```

开发服务器启动后，你将看到类似输出：

```
  VITE v5.0.0  ready in 1234 ms

  ➜  Local:   http://localhost:5173/
  ➜  Network: http://192.168.x.x:5173/
```

### 6. 访问预览

在浏览器中打开 http://localhost:5173/ 即可预览前端页面。

注意：开发模式下前端独立运行，需要后端 C++ 服务提供 API 支持。

## 开发命令参考

```powershell
# 安装依赖
npm install

# 开发模式运行
npm run dev

# 类型检查
npm run type-check

# 代码检查
npm run lint

# 构建生产版本
npm run build

# 预览生产构建
npm run preview
```

## 目录结构说明

```
frontend/
├── public/              # 静态资源（会被直接复制到输出）
├── src/
│   ├── assets/          # 资源文件（图片、字体等）
│   ├── components/      # Vue 组件
│   │   ├── Connection/  # 连接相关组件
│   │   ├── Display/     # 数据显示组件
│   │   ├── Send/        # 发送组件
│   │   └── Settings/    # 设置组件
│   ├── stores/          # Pinia 状态管理
│   ├── styles/          # 全局样式
│   ├── types/           # TypeScript 类型定义
│   ├── views/           # 页面视图
│   ├── App.vue          # 根组件
│   └── main.ts          # 入口文件
├── index.html           # HTML 入口
├── package.json         # 项目配置
├── tsconfig.json        # TypeScript 配置
├── vite.config.ts       # Vite 配置
└── README.md            # 前端说明
```

## 常见问题

### Q: npm install 失败怎么办？

A: 可以尝试以下解决方案：

1. 清理 npm 缓存：
```powershell
npm cache clean --force
```

2. 删除 node_modules 和 package-lock.json 后重试：
```powershell
rm -Recurse -Force node_modules
rm -Force package-lock.json
npm install
```

3. 使用淘宝镜像：
```powershell
npm config set registry https://registry.npmmirror.com
npm install
```

### Q: 端口被占用怎么办？

A: 修改 `vite.config.ts` 中的端口配置：

```typescript
export default defineConfig({
  server: {
    port: 3000  // 改为其他端口
  }
})
```

### Q: 如何查看完整的类型错误？

```powershell
npm run type-check
```

## 下一步

配置完成后，你可以：

1. 修改前端代码并实时预览
2. 了解后端 API 结构
3. 开始参与功能开发

如需了解项目整体架构，请参阅 [SPEC.md](./SPEC.md)。
