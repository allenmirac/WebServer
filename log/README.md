# 同步/异步日志系统
同步/异步日志系统主要设计了两个模块，一个是日志模块，一个是阻塞队列模块,其中加入阻塞队列模块主要是解决异步写入日志做准备.

> 自定义阻塞队列
> 单例模式创建日志
> 实现了同步日志
> 实现了异步日志
> 实现按天、超行分类

同步日志：所谓同步日志，即当输出日志时，必须等待日志输出语句（比如cout,file.write）执行完毕后，才能执行后面的业务逻辑语句。

异步日志：使用异步日志进行输出时，日志输出语句与业务逻辑语句并不是在同一个线程中运行，而是有专门的线程用于进行日志输出操作，处理业务逻辑的主线程不用等待即可执行后续业务逻辑，增大了空间开销，提高了时间效率。