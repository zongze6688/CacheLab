# Lab4 · Cache Lab

**截止时间：2026 年 12 月 11 日 23:59。Part A、Part B、Part C 和 Bonus 均为必做。**

## 提交内容与方式

1. 从课程的 [CacheLab 学生模板](https://github.com/ICS-26Fall-FDU/CacheLab) 点击 **Use this template → Create a new repository**，在自己的 GitHub 账号下创建并克隆个人仓库。
2. 截止前，将 `csim.c`、`trans.c`、`mul.c`、`solver.c` 和一份 **PDF 实验报告**放在仓库根目录，提交并推送到默认分支。
3. 截止前，在 eLearning 课程的「作业」页面提交个人仓库链接。打开链接，检查默认分支上的源码是否为最新版、PDF 是否可以打开。
4. 自截止时间起，个人仓库须保持 **Public** 至少 24 小时，供助教收取。

实验报告须包含四个部分的实现与分析，以及代码运行效果展示。报告单独计分。

## 源码要求

| 部分 | 文件与入口 | 要求 | 分值 |
| --- | --- | --- | ---: |
| Part A | `csim.c` 的 `access_cache()` | 模拟 LRU 缓存，统计 hits、misses 和 evictions | 35 |
| Part B | `trans.c` 的 `transpose_submit()` | 将 `A[N][M]` 转置到 `B[M][N]`，尽量减少 miss | 40 |
| Part C | `mul.c` 的 `mul_submit()` | 计算 `C = A × B`，尽量减少 miss | 25 |
| Bonus | `solver.c` 的 `solve()` | 通过读写探测缓存，报告 `b`、`s`、`E`、`policy` 和 `write_allocate` | 10 |

四部分均须无编译警告。禁止针对测试或评分数据硬编码。

## 提交前自测

在 x86_64 Linux 环境中使用 `gcc`、`make`、`python3` 和 `valgrind`。将命令中的“你的学号”替换为实际学号。

```bash
make clean && make
python3 driver.py -i 你的学号
```

总评测覆盖四个部分，自动评测满分 110 分。完整实验规格见 [README.md](README.md)。
