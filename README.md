# MostFrequentURL
由于对于rust异步的理解不到位，没有完成。

## record
day1：使用C++写了一个简单的demo，基本能够实现功能
day2：构思高性能的算法
day2~4：学习rust
day5：了解io_uring和libAio，以及rust异步相关库。
day6：部分fast-readline开发。

## 算法说明
将Url做hash分片到小文件中（不做计数），这样可以保证相同的url都在同一url中。然后遍历所有小文件，对每个文件单独count，用堆维护topK

## C++ 未优化版本
算法分为两部分，split和combian。
split去递归的将urlhash分片，每一层用不同的hash种子，直到分片足够小。
combian去读取每个文件，对其进行word count，然后合并到TopK结构中。
split输入文件是简单的getline输入，combian尝试进行了优化。一次read一大块到缓冲区中，之后以内存块为单位consume，这样1.对IO友好，一次读取4MB数据基本可以认为是顺序读写。2.对后面多线程流水线优化提供可能。

## 优化思路
### IO
总的IO在最优情况下应该是所有100G各读写一次
考虑分片个数，如果分片个数小，可能导致要多分几次，这样会有多余重复IO。而如果分片个数大是给下层文件系统很大的要求可能会产生很多随机写。  
对于机械硬盘单线程读写，转发给计算线程，做流水线即可。
充分利用ssd特别是nvme ssd可看下面rust版本 构思
### CPU
如果测试CPU成为瓶颈，split部分可以多线程并行计算hash，没有竞争。
进行wordcount时，直接用hash值判断相等，u64空间冲突概率不会很大。

## rust版本 构思
使用rust的原因有二，其一是一直想有个机会学习一下rust，其二是要利用rust的异步运行时、libaio的binding、并发队列等。因为时间原因，只是构思，没有实现。
### write
写文件都是针对单一文件的append-only，比较好处理，一个线程写一个文件或多个文件，前面接MPSC队列即可。
### read-line
需要实现快速对于文件中每一行字符串进行某种操作（hash或wordcount），不要求有序。给一个缓冲区大小，每个线程有一个缓冲区，由缓冲区大小文件被分为若干片。设有n个线程，文件被分成m片，原子变量维护一个当前已处理的分片数量，线程去取任务进行aio读取，读取后进行操作。对于分片之间的部分，在读取的时候特别处理放到每个分片维护的结构里，等其他全部处理完之后再进行统一处理。

## 数据生成和测试
没有做。
数据生成要有均匀分布和指数分布两种url频率分布的情况。

