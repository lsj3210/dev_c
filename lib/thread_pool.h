/*
 * @file  thread_pool.h
 * @brief ThreadPool
 * @Auth  Lijian
 */
#ifndef LIB_THREAD_POOL_H
#define LIB_THREAD_POOL_H

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>
#include <thread>
#include <functional>
#include <stdexcept>



#define  THREADPOOL_MAX_NUM 20 //线程池最大容量,应尽量设小一点
//#define  THREADPOOL_AUTO_GROW

/*
 * @class ThreadPool 线程池类 C++11
 * @brief 可以提交变参函数或拉姆达表达式的匿名函数执行,可以获取执行返回值
 *        直接支持类成员函数, 支持类静态成员函数或全局函数,Opteron()函数等
 */
using namespace std;

class ThreadPool
{
    using Task = function<void()>;	//定义类型
    vector<thread> _pool;      //线程池
    queue<Task> _tasks;             //任务队列
    mutex _lock;                    //同步
    condition_variable _task_cv;    //条件阻塞
    atomic<bool> _run{ true };      //线程池是否执行
    atomic<int>  _idlThrNum{ 0 };   //空闲线程数量

public:
    inline ThreadPool(unsigned short size = 4) { addThread(size); }
    inline ~ThreadPool()
    {
        _run=false;
        // 唤醒所有线程执行
        _task_cv.notify_all();
        for (thread& thread : _pool) {
            //thread.detach(); // 让线程“自生自灭”
            if(thread.joinable())
                // 等待任务结束， 前提：线程一定会执行完
                thread.join();
        }
    }

public:
    /*
     * @function  commit 提交任务
     * @param[in] f 变参函数或拉姆达表达式的匿名函数
     * @param[in] args f函数需要的可变参数
     * @return    std::future
     * @brief     调用.get()获取返回值会等待任务执行完,获取返回值
     *            两种方法可以实现调用类成员
     *            一种是使用   bind：  .commit(std::bind(&Dog::sayHello, &dog));
     *            一种是用    mem_fn： .commit(std::mem_fn(&Dog::sayHello), this)
     */
    template<class F, class... Args>
    auto commit(F&& f, Args&&... args) ->future<decltype(f(args...))>
    {
        if (!_run)    // stoped ??
            throw runtime_error("commit on ThreadPool is stopped.");
        // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
        using RetType = decltype(f(args...));
        // 把函数入口及参数,打包(绑定)
        auto task = make_shared<packaged_task<RetType()>>(
                bind(forward<F>(f), forward<Args>(args)...)
        );
        future<RetType> future = task->get_future();
        {    // 添加任务到队列
            lock_guard<mutex> lock{ _lock };//对当前块的语句加锁  lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候 unlock()
            _tasks.emplace([task](){ // push(Task{...}) 放到队列后面
                (*task)();
            });
        }
#ifdef THREADPOOL_AUTO_GROW
        if (_idlThrNum < 1 && _pool.size() < THREADPOOL_MAX_NUM)
			    addThread(1);
#endif // !THREADPOOL_AUTO_GROW
        _task_cv.notify_one(); // 唤醒一个线程执行

        return future;
    }

    //空闲线程数量
    int idlCount() { return _idlThrNum; }
    //线程数量
    int thrCount() { return _pool.size(); }
//#ifndef THREADPOOL_AUTO_GROW
private:
//#endif // !THREADPOOL_AUTO_GROW
    //添加指定数量的线程
    void addThread(unsigned short size)
    {
        for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
        {   //增加线程数量,但不超过 预定义数量 THREADPOOL_MAX_NUM
            _pool.emplace_back( [this]{ //工作线程函数
                while (_run)
                {
                    Task task; // 获取一个待执行的 task
                    {
                        // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                        unique_lock<mutex> lock{ _lock };
                        _task_cv.wait(lock, [this]{
                            return !_run || !_tasks.empty();
                        }); // wait 直到有 task
                        if (!_run && _tasks.empty())
                            return;
                        task = move(_tasks.front()); // 按先进先出从队列取一个 task
                        _tasks.pop();
                    }
                    _idlThrNum--;
                    task();//执行任务
                    _idlThrNum++;
                }
            });
            _idlThrNum++;
        }
    }
};

//ThreadPool test

/*
#include "thread_pool.h"
#include <iostream>

void fun1(int slp)
{
    printf("  hello, fun1 !  %d\n" ,std::this_thread::get_id());
    if (slp>0) {
        printf(" ======= fun1 sleep %d  =========  %d\n",slp, std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }
}

struct gfun {
    int operator()(int n) {
        printf("%d  hello, gfun !  %d\n" ,n, std::this_thread::get_id() );
        return 42;
    }
};

class A {
public:
    static int Afun(int n = 0) {   //函数必须是 static 的才能直接使用线程池
        std::cout << n << "  hello, Afun !  " << std::this_thread::get_id() << std::endl;
        return n;
    }

    static std::string Bfun(int n, std::string str, char c) {
        std::cout << n << "  hello, Bfun !  "<< str.c_str() <<"  " << (int)c <<"  " << std::this_thread::get_id() << std::endl;
        return str;
    }
};

int main()
try {
    ThreadPool executor{ 50 };
    A a;
    std::future<void> ff = executor.commit(fun1,0);
    std::future<int> fg = executor.commit(gfun{},0);
    //std::future<int> gg = executor.commit(a.Afun, 9999); //IDE提示错误,但可以编译运行
    std::future<std::string> gh = executor.commit(A::Bfun, 9998,"mult args", 123);
    std::future<std::string> fh = executor.commit([]()->std::string { std::cout << "hello, fh !  " << std::this_thread::get_id() << std::endl; return "hello,fh ret !"; });

    std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(900));

    for (int i = 0; i < 50; i++) {
        executor.commit(fun1,i*100 );
    }
    std::cout << " =======  commit all ========= " << std::this_thread::get_id()<< " idlsize="<<executor.idlCount() << std::endl;

    std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //ff.get(); //调用.get()获取返回值会等待线程执行完,获取返回值
    std::cout << fg.get() << "  " << fh.get().c_str()<< "  " << std::this_thread::get_id() << std::endl;

    std::cout << " =======  sleep ========= " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << " =======  fun1,55 ========= " << std::this_thread::get_id() << std::endl;
    executor.commit(fun1,55).get();    //调用.get()获取返回值会等待线程执行完

    std::cout << "end... " << std::this_thread::get_id() << std::endl;


    ThreadPool pool(4);
    std::vector< std::future<int> > results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
                pool.commit([i] {
                    std::cout << "hello " << i << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    std::cout << "world " << i << std::endl;
                    return i*i;
                })
        );
    }
    std::cout << " =======  commit all2 ========= " << std::this_thread::get_id() << std::endl;

    for (auto && result : results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    return 0;
}
catch (std::exception& e) {
    std::cout << "some unhappy happened...  " << std::this_thread::get_id() << e.what() << std::endl;
}

 */



#endif //LIB_THREAD_POOL_H
