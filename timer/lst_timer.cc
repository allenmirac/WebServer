#include "lst_timer.h"

namespace webserver
{
SortTimerLst::SortTimerLst(){
    head=nullptr;
    tail=nullptr;
}

SortTimerLst::~SortTimerLst(){
    UtilTimer *tmp = head;
    while (tmp){
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void SortTimerLst::add_timer(UtilTimer *timer){
    if (!timer){
        return;
    }
    if (!head){
        head = tail = timer;
        return;
    }
    if (timer->expire < head->expire){
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    add_timer(timer, head);
}

void SortTimerLst::adjust_timer(UtilTimer *timer){
    if (!timer){
        return;
    }
    UtilTimer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire)){
        return;
    }
    if (timer == head){
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    }else{
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void SortTimerLst::del_timer(UtilTimer *timer){
    if (!timer){
        return;
    }
    if ((timer == head) && (timer == tail)){
        delete timer;
        head = NULL;
        tail = NULL;
        return;
    }
    if (timer == head){
        head = head->next;
        head->prev = NULL;
        delete timer;
        return;
    }
    if (timer == tail){
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void SortTimerLst::tick(){
    if (!head){
        return;
    }
    
    time_t cur = time(NULL);
    UtilTimer *tmp = head;
    while (tmp)
    {
        if (cur < tmp->expire)
        {
            break;
        }
        tmp->cb_func(tmp->user_data);
        head = tmp->next;
        if (head)
        {
            head->prev = NULL;
        }
        delete tmp;
        tmp = head;
    }
}


} // namespace webserver
