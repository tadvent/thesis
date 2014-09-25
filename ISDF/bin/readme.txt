isdf.exe 是 ISDF 项目的生成文件
i.txt 是输入数据，格式按照 InputDataFormat.txt 中的说明

isdf.exe 根据 i.txt 计算出调度结果，产生 o.txt 和 data.txt 文件
在 o.txt 中可以看到计算各个步骤的输出结果以及最终的调度结果

为了方便演示，用 python 完成的调度结果的图形化功能。
采用 python3 + matploylib 库，根据 data.txt 的输入文件得到调度图

