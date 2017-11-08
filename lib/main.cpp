#include <iostream>
#include "thread_pool.h"


int Afun(int n = 0) {   //函数必须是 static 的才能直接使用线程池
    std::cout << n << "  hello, Afun !  " << std::this_thread::get_id() << std::endl;
    for(int i = 0; i< 100; i++) {
        n = n + i;
    }
    return n;
}

int main() {

    //函数指针
    int (* pAfunc)(int n);

    pAfunc = Afun;

    std::cout << "main thread id : " <<std::this_thread::get_id() << std::endl;

    auto func_test = [](){
        std::cout << "func_test thread id : " <<std::this_thread::get_id() << std::endl;
        for (int i = 0; i<100; i++) {
            printf("%s\n", "hello cpp!");
        }
    };

    ThreadPool pool { 2 };
    pool.commit(func_test);

    std::this_thread::sleep_for(std::chrono::microseconds(900));

    std::future<int> t2 = pool.commit(pAfunc,2);
    auto result = t2.get();
    std::cout << result << std::endl;

    std::this_thread::sleep_for(std::chrono::microseconds(900));

    auto func1 = [](int b,int d) {return b+d;};
    std::cout << func1(2,3) << std::endl;

    int size = 6;

    system("pause");
    return 0;
}




 