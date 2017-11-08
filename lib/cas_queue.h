//
// Created by ThinkPad on 2017/8/28.
//

#ifndef LIB_CAS_QUEUE_H
#define LIB_CAS_QUEUE_H

#include <atomic>

template <class T>
typedef struct LinkNode {
    T data;
    LinkNode* next;
}LinkNode;

template <class T>
class CasQueue {
public:
    CasQueue():m_head(NULL), m_tail(NULL), m_empty(true), m_length(0) {
        m_head = new LinkNode;
        m_head->next = NULL;
        m_tail = m_head;
    }
    ~CasQueue(){
        LinkNode *p = m_head;
        if (p) {
            LinkNode *q = p->next;
            delete p;
            p = q;
        }
    }
    int push(const T &_msg) {
        LinkNode * q = new LinkNode;
        q->data = _msg;
        q->next = NULL;

        LinkNode * p = m_tail;
        LinkNode * oldp = p;
        do {
            while (p->next != NULL)
                p = p->next;
        } while( __sync_bool_compare_and_swap(&(p->next), NULL, q) != true); //如果没有把结点链在尾上，再试

        __sync_bool_compare_and_swap(&m_tail, oldp, q); //置尾结点
        return 0;
    }
    T pop(){
        LinkNode * p;
        do{
            p = m_head;
            if (p->next == NULL){
                return "";
            }
        } while( __sync_bool_compare_and_swap(&m_head, p, p->next) != true );
        return p->next->data;
    }
    bool empty(){
        return m_empty;
    }
private:
    LinkNode * m_head;
    LinkNode * m_tail;
    bool m_empty;
    unsigned int m_length;
};

#endif //LIB_CAS_QUEUE_H
