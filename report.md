## Tomasulo 实验报告

> 2017011362 计 75 班 赵成钢

### 编译说明

本项目用 CMake 构建，请用类似下面的指令进行编译和运行：

```bash
mkdir build
cd build
cmake ..
./Tomasulo
```

其中本项目依赖了 boost 库（和算法无关，一些 C++ 特性），如果 boost 库的位置不在环境中，请自行修改 `CMakeLists.txt`。

### 输出文件说明

在本实现中和 `Example.pdf` 提供的过程不完全一致（释放保留站之后不会有一个空档再执行，后文会详细说明），故输出结果可能和助教的预期并不相同。

输出的内容见 `Log` 目录中的文件，其中完整记录了各条指令的发射周期、执行完成周期和写回结果周期，包括全部的测例（含扩展测例）。

### 查询任意 Cycle 状态的说明

因为输出量过大，在实现中已经注释掉了输出每个周期的内容，如果要查询，可以直接取消 `runner.hpp` 文件中的 `processor.print();`，这样会在每个周期之后进行注释的打印。如果需要查询某个特定周期的状态，则在对应的周期调用 `Processor::print()` 函数即可。

### 实现设计

最后，我完成了带有简单 JUMP 功能的 Tomasulo 实现，可以运行全部的测例。

#### 整体流程

在不展开讨论 Tomasulo 的细节时，整体的流程如下：

- 读入文件
- 翻译为具体的指令
- 让 Verifier 执行指令
- 让 Tomasulo 执行指令
- 比较和验证结果

其中，我实现了一个验证器，具体细节请参见 `verifier.hpp`，最后的寄存器的计算结果会和 Tomasulo 求解器进行比较进行简单验证。

#### Tomasulo 实现

Tomasulo 实现在 `processor.hpp` 文件中，其中包含了：

- FunctionalUnit
  - 即功能单元 FU
  - 包含剩余执行周期、写入的目的寄存器和计算出来的值
- ReservationStation
  - 即保留站 RS（并不对 Load Buffer 和运算器 RS 作区分）
  - 包含 `busy`、`vj`、`vk`、`addr`、`qj` 和 `qk` 等经典概念
  - 包含执行的 FU 和正在执行的指令
- Register
  - 寄存器
  - 包含即将写入的 RS（最新的）
- ToWriteBack
  - 传给下一个周期进行写入
  - 包含目的寄存器和值
- Processor
  - 模拟一个 Tomasulo 控制的 CPU
  - 每次给一个指令的地址，决定如何变化地址，新的 PC 会在下一个周期执行

其中，Tomasulo **每个周期**的核心流程如下（即如何用软件模拟 CDB 等行为）：

- 写入上一个周期的 WriteBack
- 模拟一个 CDB
  - 在后面记录这个周期执行完成的值
- 遍历 RS
  - 如果 RS 在忙，而且没有分配 FU，就放在一个准备分配 FU  的数组中
  - 如果执行完了，把要写入的值写入到 CDB 中，然后清空 RS
  - 把要写入的值也记录到 WriteBack 中，下一个周期进行写入
- 尝试 Issue 当前 PC 的指令（如果当前不在跳转指令的逻辑处理时间内）
  - 如果能找到一个不忙的 RS
    - Issue 指令
    - 看看 CDB 是不是依赖的寄存器刚刚写入或者是直接用寄存器的值（选择最新的）
    - 如果是 JUMP 指令，需要让后面的指令停下来，用一个布尔变量来记录
    - 设置目的寄存器要等着这个 RS
    - 如果有了 RS，放在准备分配 FU 的数组（上面提到过了）中
- 遍历每个准备分配 FU 的 RS
  - 更新 RS 需要的值是不是在 CDB 中刚刚执行完
  - 如果有 FU 空着
    - 放到 FU 中执行

也正如最开始提到的，因为我在每个周期开始的时候会把执行完的 RS 全部清空，而后面 Issue 寻找 RS 时会找到该 RS，然后一并在后面（同一个周期）的“遍历每个准备分配 FU 的 RS”过程中进行 FU 的寻找，所以这样不会造成 RS 被浪费了一个时钟周期（这个是 `Example.pdf` 的做法）。而我们分析一下，这完全是可以让硬件做到的，我们完全可以根据上个时钟周期来推断出该时钟周期是否可以被执行完成然后是否会被当前在 Issue 的变量占用，从而在一个时钟周期完成一系列的操作，下个周期开始前之间给对应 FU 信号开始执行，并不需要所谓“就绪”这个过程，个人认为本实现这样的做法更为合理，而且不会浪费掉这样产生的时钟周期，对应的 CPU 的速度也会更快。

#### 和记分牌算法的差异 - 分布式

不在集中记录每个 FU 和 RS 的使用情况，而是分布到每个 RS 和 FU 以及寄存器中，在代码中体现为各个结构体的相互引用，和下面的定义：

```c++
Register registers[NUM_REGISTERS]{};

ReservationStation add_sub_rs[NUM_ADD_SUB_RS]{};
ReservationStation mul_div_rs[NUM_MUL_DIV_RS]{};
ReservationStation load_rs[NUM_LOAD_RS]{};

FunctionalUnit add_sub_fu[NUM_ADD_SUB_FU]{};
FunctionalUnit mul_div_fu[NUM_MUL_DIV_FU]{};
FunctionalUnit load_fu[NUM_LOAD_FU]{};
```

可以看到每个信息被分布到每个 RS 中，而没有整个的类似记分牌的管理。

#### 和记分牌算法的差异 - 重命名技术

每个指令会用 RS 来管理，而 RS 依赖的值只能是已经算好的值（`vj` 和  `vk`），或者引用另一个 RS（`qj` 和 `qk`），而不是引用另一个寄存器，而且在寄存器中也会有 RS 的引用，这个要点就是重命名技术，在代码中，体现为：

```c++
// Reservation station for both FU and Loader
struct ReservationStation {
    bool busy;
    int vj, vk, addr;
    FunctionalUnit *fu;
    Instruction *instruction;
    ReservationStation *qj, *qk; // 这里是另外两个 RS 的指针，也就意味着重命名
}
```

这对应 Tomasulo 的核心思想：

- 操作数一旦就绪就立即执行，尽可能去减少 RAW 冲突
- RAW 则通过把寄存器重命名指向 RS 的 `qj` 和 `qk` 来完成消除
- WAW 则通过每个寄存器记录最新的 RS 引用来完成消除

具体如何利用这些变量请参见 `Processor::tick` 函数。

### 性能

最后，给出 Performance 类和 Gcd.nel 的执行时间（本地环境为 macOS 10.15，Intel Core i7 7700HQ，Apple clang 11.0）：

| 测例         | 时间        |
| ------------ | ----------- |
| Gcd.nel      | 12277.731ms |
| Mul.nel      | 1.275ms     |
| Big_test.nel | 428.352ms   |

个人认为应该把 Gcd.nel 放到 Performance 中，该测例执行了整整 2000 万次 Issue 的操作。

### 收获

本次学习 Tomasulo 收获非常大，感受到了体系结构算法的设计精妙之处，就像建筑一样既有美感，也有人类设计的智慧的体现，着实令人着迷。

