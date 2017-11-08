/*
 * @File    nocopyable.h
 * @Class   Nocopyable
 * @Describ 不可拷贝基类，所有不可拷贝的类继承它
 * @Auth    lijian
 */

#ifndef LIB_NOCOPYABLE_H
#define LIB_NOCOPYABLE_H

class Nocopyable
{
private:
    Nocopyable(const Nocopyable& x) = delete;
    Nocopyable& operator=(const Nocopyable&x) = delete;
public:
    Nocopyable() = default;
    ~Nocopyable() = default;
};

#endif //LIB_NOCOPYABLE_H
