/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __INCLUDE_SLIST_H__
#define __INCLUDE_SLIST_H__

#include <bases.h>

/*********************************************************
 *@类型说明：
 *
 *[slist_node_t]：单向循环链表的节点
 *[slist_t]：单向循环链表
 *********************************************************/
typedef struct slist_node_s {
    /* 下一个节点 */
    struct slist_node_s *next;
} slist_node_t, slist_t;



/************************************************************
 *@简介：
 ***单向循环链表结构体静态初始化
 *
 *@用法：
 ***slist_t list = SLIST_STATIC_INIT(list);
 *
 *@参数：
 *[head]：链表变量名，非地址
 *************************************************************/
#define SLIST_STATIC_INIT(head) { &head }


/************************************************************
 *@简介：
 ***单向循环链表节点结构体静态初始化
 *
 *@用法：
 ***slist_node_t node = SLIST_NODE_STATIC_INIT(node);
 *
 *@参数：
 *[node]：节点变量名，非地址
 *************************************************************/
#define SLIST_NODE_STATIC_INIT(node) { &node }


/************************************************************
 *@简介：
 ***获取链表的头节点
 *
 *@参数：
 *[slist]：单向循环链表
 *
 *@返回：单向循环链表的头节点
 *************************************************************/
#define SLIST_HEAD(slist)      ((slist_node_t *)(slist))


/************************************************************
 *@简介：
 ***获取链表节点的下一个节点
 *
 *@参数：
 *[node]：单向循环链表的节点
 *
 *@返回：单向循环链表节点的下一个节点
 *************************************************************/
#define SLIST_NODE_NEXT(node)      ((node)->next)


/*********************************************************
 *@简要：
 ***单向循环链表初始化
 *
 *@约定：
 ***1、参数slist不能为空指针
 ***2、不可对正在使用的slist进行初始化
 *
 *@参数：
 *[slist]：单向循环链表
 **********************************************************/
#define slist_init(slist)           ((slist)->next = (slist))


/*********************************************************
 *@简要：
 ***初始化单向循环链表节点
 *
 *@约定：
 ***1、参数node不能为空指针
 ***2、不可对正在使用的node进行初始化
 *
 *@参数：
 *[node]：单向循环链表的节点
 **********************************************************/
#define slist_node_init(node)           ((node)->next = (node))


/*********************************************************
 *@简要：
 ***在当前节点后面插入节点
 *
 *@约定：
 ***1、node处于链表之中
 ***2、next_node为已删除的节点
 ***3、node与next_node都不为空指针
 *
 *@参数：
 *[node]：链表中的节点
 *[next_node]：插入的节点
 **********************************************************/
static force_inline void slist_node_insert_next(slist_node_t *node,
                                        slist_node_t *next_node)
{
    next_node->next = node->next;
    node->next = next_node;
}


/*********************************************************
 *@简要：
 ***删除当前节点的下一个节点
 *
 *@约定：
 ***1、node不是链表中最后一个节点
 ***2、node不是空指针
 ***3、node处于链表中
 *
 *@参数：
 *[node]：链表中的节点
 *
 *@返回：被删除的节点
 **********************************************************/
static force_inline slist_node_t* slist_node_del_next(slist_node_t *node)
{
    slist_node_t *del_node = node->next;

    node->next = del_node->next;
    del_node->next = del_node;

    return del_node;
}


/*********************************************************
 *@简要：
 ***判断单向循环链表是否为空
 *
 *@约定：
 ***1、slist不是空指针
 *
 *@参数：
 *[slist]：单向循环链表
 *
 *@返回值：
 *[ture]：单向循环链表是空链表
 *[false]：单向循环链表不是空链表
 **********************************************************/
#define slist_is_empty(slist)           ((slist)->next == (slist))


/*********************************************************
 *@简要：
 ***判断单向循环链表是否为空
 *
 *@约定：
 ***1、slist不是空指针
 *
 *@参数：
 *[node]：单向循环链表的节点
 *
 *@返回值：
 *[ture]：节点已被删除
 *[false]：节点未被删除
 **********************************************************/
#define slist_node_is_del(node)           ((node)->next == (node))

#define slist_node_is_ref(node)   (!slist_node_is_del(node))


/*********************************************************
 *@简要：
 ***将已删除的节点标记为引用状态，使slist_node_is_del返回false
 *
 *@约定：
 ***1、node不是空指针
 *
 *@参数：
 *[node]：被引用的节点
 *
 *@返回值：
 *[true]：标记成功
 *[false]：节点未处于删除状态
 **********************************************************/
static force_inline slist_node_t *slist_node_ref(slist_node_t *node)
{
    if (node->next == node) {
        node->next = 0;
        return node;
    }

    return 0;
}


/*********************************************************
 *@简要：
 ***将引用的节点标记为未引用状态，使slist_node_is_del返回true
 *
 *@约定：
 ***1、node不是空指针
 *
 *@参数：
 *[node]：已引用的节点
 *
 *@返回值：
 *[true]：标记成功
 *[false]：节点未处于删除状态
 **********************************************************/
static force_inline bool slist_node_unref(slist_node_t *node)
{
    if (node->next == 0) {
        node->next = node;
        return true;
    }

    return false;
}


/*********************************************************
 *@简要：
 ***通过成员指针获取其外层结构体指针
 *
 *@参数：
 *[type]：外层结构体的类型名
 *[member]：成员在外层结构体中的成员名
 *[member_ptr]：结构体中成员的指针
 *
 *@返回类型：
 *[type *]：type的指针
 *
 *@返回值：成员指针所在的外层结构体指针
 **********************************************************/
#define slist_entry(member_ptr, type, member) container_of(member_ptr, type, member)


/*********************************************************
 *@简要：
 ***通过任意的外层结构体指针和成员指针获取成员所在外层结构体的指针
 *
 *@参数：
 *[eptr]：任意的外层结构体指针
 *[member]：成员在外层结构体中的成员名
 *[member_ptr]：结构体中成员的指针
 *
 *@返回类型：
 *[eptr的类型]：eptr指针类型
 *
 *@返回值：成员指针所在的外层结构体指针
 **********************************************************/
#define slist_pentry(member_ptr, eptr, member) pcontainer_of(member_ptr, eptr, member)



/*********************************************************
 *@简要：
 ***使用node遍历整个slist，node为当前被遍历的节点
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、若对节点执行插入与删除操作，则不可继续遍历。
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[node]：存放着当前被遍历的节点
 *
 *@用法：
 ***  slist_node_t *node;
 ***
 ***  slist_foreach(slist, node)
 ***  {
 ***      //对每个节点的处理代码
 ***  }
 **********************************************************/
#define slist_foreach(slist, node)                              \
    for ((node) = (slist)->next;                                \
        (node) != (slist);                                      \
        (node) = (node)->next)


/*********************************************************
 *@简要：
 ***使用node遍历整个slist，node为当前被遍历的节点
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、链表节点的安全删除操作需使用slist_node_del_next_safe
 ***3、链表节点的安全插入操作需使用slist_node_insert_next_safe
 ***4、在使用安全插入与删除操作的情况下，则可以继续遍历。
 ***5、不可对safe_node做任何处理
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[node]：存放着当前被遍历的节点
 *[safe_node]：链表遍历中进行插入与删除操作所需的安全节点
 *
 *@用法：
 ***  slist_node_t *node;
 ***  slist_node_t *safe_node;
 ***
 ***  slist_foreach_safe(slist, node, safe_node)
 ***  {
 ***      //对每个节点的处理代码
 ***  }
 **********************************************************/
#define slist_foreach_safe(slist, node, safe_node)              \
    for ((safe_node) = (slist)->next,                           \
         (node) = (safe_node);                                  \
         (node) != (slist);                                     \
         (safe_node) = (safe_node)->next,                       \
         (node) = (safe_node))


/*********************************************************
 *@简要：
 ***使用node遍历整个slist，node为当前被遍历的节点，
 ***prev_node为node的上一个节点
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、若对节点执行插入与删除操作，则不可继续遍历。
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[node]：存放着当前被遍历的节点
 *[prev_node]: 存放着当前被遍历节点的上一个节点
 *
 *@用法：
 ***  slist_node_t *node;
 ***  slist_node_t *prev_node;
 ***
 ***  slist_foreach_record_prev(slist, node, prev_node)
 ***  {
 ***      if (...) //某些条件
 ***      {
 ***          //通过prev_node移除node
 ***          slist_node_del_next(prev_node);
 ***
 ***          //在执行删除与插入后必须退出遍历
 ***          break;
 ***      }
 ***  }
 **********************************************************/
#define slist_foreach_record_prev(slist, node, prev_node)       \
    for ((node) = (slist)->next,                                \
         (prev_node) = (slist);                                 \
         (node) != (slist);                                     \
         (prev_node) = (node),                                  \
         (node) = (node)->next)


/*********************************************************
 *@简要：
 ***使用node遍历整个slist，node为当前被遍历的节点，
 ***prev_node为被遍历节点的上一个节点
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、链表节点的安全删除操作需使用slist_node_del_next_safe
 ***3、链表节点的安全插入操作需使用slist_node_insert_next_safe
 ***4、在使用安全插入与删除操作的情况下，则可以继续遍历。
 ***5、不可对safe_node做任何处理
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[node]：存放着当前被遍历的节点
 *[prev_node]: 存放着当前被遍历节点的上一个节点
 *[safe_node]：链表遍历中进行插入与删除操作所需的安全节点
 *
 *@用法：
 ***  slist_node_t *node;
 ***  slist_node_t *prev_node;
 ***  slist_node_t *safe_node;
 ***
 ***  slist_foreach_record_prev_safe(slist, node, prev_node, safe_node)
 ***  {
 ***      if (...) //某些条件
 ***      {
 ***          //通过prev_node在遍历中安全移除node
 ***          slist_node_del_next_safe(prev_node, &safe_node);
 ***
 ***          //在插入与删除之后，可以继续遍历
 ***          //break;
 ***      }
 ***  }
 **********************************************************/
#define slist_foreach_record_prev_safe(slist, node, prev_node, safe_node)       \
    for ((prev_node) = (slist),                                                 \
         (safe_node) = (slist)->next,                                           \
         (node) = (safe_node);                                                  \
         (node) != (slist);                                                     \
         (prev_node) = (safe_node),                                             \
         (safe_node) = (safe_node)->next,                                       \
         (node) = (safe_node))


/*********************************************************
 *@简要：
 ***使用entry遍历整个slist，entry为当前被遍历的对象
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、若对节点执行插入与删除操作，则不可继续遍历。
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[entry]：存放着当前被遍历节点的外层结构体指针
 *[member]: 链表节点在外层结构体中的成员名
 *
 *@用法：
 ***  student_t *student;
 ***
 ***  slist_foreach(class_list, student, class_list_node)
 ***  {
 ***      //奖励分数超过95的学生
 ***      if (student->score >= 95)
 ***      {
 ***           //奖励这个学生100元
 ***           reward_money(student, 100);
 ***      }
 ***  }
 **********************************************************/
#define slist_foreach_entry(slist, entry, member)                               \
    for (entry = pcontainer_of((slist)->next, (entry), member);    \
        &(entry)->member != (slist);                                            \
        entry = pcontainer_of((entry)->member.next, (entry), member))


/*********************************************************
 *@简要：
 ***使用entry遍历整个slist，entry为当前被遍历的对象
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、链表节点的安全删除操作需使用slist_node_del_next_safe
 ***3、链表节点的安全插入操作需使用slist_node_insert_next_safe
 ***4、在使用安全插入与删除操作的情况下，则可以继续遍历。
 ***5、不可对safe_node做任何处理
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[entry]：存放着当前被遍历节点的外层结构体指针
 *[member]：链表节点在外层结构体中的成员名
 *[safe_node]：链表遍历中进行插入与删除操作所需的安全节点
 *
 *@用法：
 ***  student_t *student;
 ***  slist_node_t *safe_node;
 ***
 ***  slist_foreach_entry_safe(class_list, student, class_list_node, safe_node)
 ***  {
 ***      //奖励分数超过95的学生
 ***      if (student->score >= 95)
 ***      {
 ***           //奖励这个学生100元
 ***           reward_money(student, 100);
 ***      }
 ***  }
 **********************************************************/
#define slist_foreach_entry_safe(slist, entry, member, safe_node)               \
    for ((safe_node) = (slist)->next,                                           \
         entry = pcontainer_of((safe_node), (entry), member);                   \
         &(entry)->member != (slist);                                           \
         (safe_node) = (safe_node)->next,                                       \
         entry = pcontainer_of((safe_node), (entry), member))


/*********************************************************
 *@简要：
 ***使用entry遍历整个slist，entry为当前被遍历的对象
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、若对节点执行插入与删除操作，则不可继续遍历。
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[entry]：存放着当前被遍历节点的外层结构体指针
 *[member]: 链表节点在外层结构体中的成员名
 *[prev_node]：存放着当前被遍历节点的上一个节点
 *
 *@用法：
 ***  student_t *student;
 ***  slist_node_t *prev_node;
 ***
 ***  slist_foreach_entry_record_prev(class_list, student, class_list_node, prev_node)
 ***  {
 ***      //将小明移除班级
 ***      if (strcmp(student->name, "小明") == 0)
 ***      {
 ***          //通过prev_node移除当前节点
 ***          slist_node_del_next(prev_node);
 ***          
 ***          //在插入或删除后，必须退出遍历
 ***          break;
 ***      }
 ***  }
 **********************************************************/
#define slist_foreach_entry_record_prev(slist, entry, member, prev_node)                \
    for (entry = pcontainer_of((slist)->next, (entry), member),                         \
         (prev_node) = (slist);                                                         \
         &(entry)->member != (slist);                                                   \
         (prev_node) = &(entry)->member,                                                \
         entry = pcontainer_of((entry)->member.next, (entry), member))


/*********************************************************
 *@简要：
 ***使用entry遍历整个slist，entry为当前被遍历的对象
 *
 *@约定：
 ***1、slist不能为空指针
 ***2、链表节点的安全删除操作需使用slist_node_del_next_safe
 ***3、链表节点的安全插入操作需使用slist_node_insert_next_safe
 ***4、在使用安全插入与删除操作的情况下，则可以继续遍历。
 ***5、不可对safe_node做任何处理
 *
 *@参数：
 *[slist]：被遍历的单向循环链表
 *[entry]：存放着当前被遍历节点的外层结构体指针
 *[member]: 链表节点在外层结构体中的成员名
 *[prev_node]：存放着当前被遍历节点的上一个节点
 *[safe_node]：链表遍历中进行插入与删除操作所需的安全节点
 *
 *@用法：
 ***  student_t *student;
 ***  slist_node_t *prev_node;
 ***  slist_node_t *safe_node;
 ***
 ***  slist_foreach_entry_record_prev_safe(class_list, student, class_list_node, prev_node, safe_node)
 ***  {
 ***      //将犯错次数超过三次的学生移除班级
 ***      if (student->mistake_count >= 3)
 ***      {
 ***          //通过prev_node移除当前节点
 ***          slist_node_del_next_safe(prev_node, &safe_node);
 ***
 ***          //在插入或删除后，可以继续遍历
 ***          //break;
 ***      }
 ***  }
 **********************************************************/
#define slist_foreach_entry_record_prev_safe(slist, entry, member, prev_node, safe_node)\
    for ((prev_node) = (slist),                                                         \
         (safe_node) = (slist)->next,                                                   \
         entry = pcontainer_of((safe_node), (entry), member);                           \
         &(entry)->member != (slist);                                                   \
         (prev_node) = (safe_node),                                                     \
         (safe_node) = (safe_node)->next,                                               \
         entry = pcontainer_of((safe_node), (entry), member))



/* 从form开始遍历，功能与slist_foreach相同 */
#define slist_foreach_from(from, slist, node)                   \
    for ((node) = (from)->next;                                 \
        (node) != (slist);                                      \
        (node) = (node)->next)



/* 从form开始遍历，功能与slist_foreach_safe相同 */
#define slist_foreach_from_safe(from, slist, node, safe_node)   \
    for ((safe_node) = (from)->next,                            \
         (node) = (safe_node);                                  \
         (node) != (slist);                                     \
         (safe_node) = (safe_node)->next,                       \
         (node) = (safe_node))



/* 从form开始遍历，功能与slist_foreach_record_prev相同 */
#define slist_foreach_from_record_prev(from, slist, node, prev_node)        \
    for ((prev_node) = (from),                                              \
         (node) = (from)->next;                                             \
         (node) != (slist);                                                 \
         (prev_node) = (node),                                              \
         (node) = (node)->next)



/* 从form开始遍历，功能与slist_foreach_record_prev_safe相同 */
#define slist_foreach_from_record_prev_safe(from, slist, node, prev_node, safe_node)    \
    for ((prev_node) = (from),                                                          \
         (safe_node) = (from)->next,                                                    \
         (node) = (safe_node);                                                          \
         (node) != (slist);                                                             \
         (prev_node) = (safe_node),                                                     \
         (safe_node) = (safe_node)->next,                                               \
         (node) = (safe_node))



/* 从form_entry开始遍历，功能与slist_foreach_entry相同 */
#define slist_foreach_from_entry(form_entry, slist, entry, member)              \
    for (entry = pcontainer_of((form_entry)->member.next, (entry), member);     \
        &(entry)->member != (slist);                                            \
        entry = pcontainer_of((entry)->member.next, (entry), member))



/* 从form_entry开始遍历，功能与slist_foreach_entry_safe相同 */
#define slist_foreach_form_entry_safe(form_entry, slist, entry, member, safe_node)          \
    for ((safe_node) = (form_entry)->member.next,                                           \
         entry = pcontainer_of((safe_node), (entry), member);                               \
         &(entry)->member != (slist);                                                       \
         (safe_node) = (safe_node)->next,                                                   \
         entry = pcontainer_of((safe_node), (entry), member))



/* 从form_entry开始遍历，功能与slist_foreach_entry_record_prev相同 */
#define slist_foreach_form_entry_record_prev(form_entry, slist, entry, member, prev_node)   \
    for ((prev_node) = &(form_entry)->member,                                               \
         entry = pcontainer_of((form_entry)->member.next, (entry), member);                 \
         &(entry)->member != (slist);                                                       \
         (prev_node) = &(entry)->member,                                                    \
         entry = pcontainer_of((entry)->member.next, (entry), member))



/* 从form_entry开始遍历，功能与slist_foreach_entry_record_prev_safe相同 */
#define slist_foreach_form_entry_record_prev_safe(form_entry, slist, entry, member, prev_node, safe_node)\
    for ((prev_node) = &(form_entry)->member,                                           \
         (safe_node) = (form_entry)->member.next,                                       \
         entry = pcontainer_of((safe_node), (entry), member);                           \
         &(entry)->member != (slist);                                                   \
         (prev_node) = (safe_node),                                                     \
         (safe_node) = (safe_node)->next,                                               \
         entry = pcontainer_of((safe_node), (entry), member))


/*********************************************************
 *@简要：
 ***该函数可以在slist_foreach_**safe系列的宏中，对链表节点执行安全删除操作，
 ***并保持链表正常遍历
 *
 *@参数：
 *[prev_node]：被删除节点的前一个节点
 *[safe_node]：安全遍历时使用的safe_node
 *
 *@返回：被删除的节点
 **********************************************************/
static inline slist_node_t* slist_node_del_next_safe(slist_node_t *prev_node, slist_node_t **safe_node)
{
    slist_node_t *node = prev_node->next;

    if (*safe_node == node) {
        *safe_node = prev_node;
    }

    prev_node->next = node->next;
    node->next = node;

    return node;
}



/*********************************************************
 *@简要：
 ***该函数可以在slist_foreach_**safe系列的宏中，对链表节点执行安全删除操作，
 ***并保持链表正常遍历
 *
 *@参数：
 *[prev_node]：插入节点的前一个节点
 *[node]：需要插入的节点
 *[safe_node]：安全遍历时使用的safe_node
 **********************************************************/
static inline void slist_node_insert_next_safe(slist_node_t *prev_node,
                                                  slist_node_t *node,
                                                  slist_node_t **safe_node)
{
    if (*safe_node == prev_node) {
        *safe_node = node;
    }

    node->next = prev_node->next;
    prev_node->next = node;
}


/*********************************************************
 *@简要：
 ***删除单循环链表中的node节点
 *
 *@约定：
 ***1、slist与node不是空指针
 *
 *@参数：
 *[slist]：单循环链表
 *[node]：要删除的节点
 *
 *@返回值：
 *[true]：成功从slist中删除节点
 *[false]：node不在slist之中
 **********************************************************/
static inline bool slist_del_node(slist_t *slist, slist_node_t *node)
{
    slist_node_t *prev_node;
    slist_node_t *find_node;

    slist_foreach_record_prev(slist, find_node, prev_node) {
        if (find_node == node) {
            slist_node_del_next(prev_node);
            return true;
        }
    }

    return false;
}


/*********************************************************
 *@简要：
 ***安全删除链表中的节点
 *
 *@参数：
 *[slist]：单循环链表
 *[node]：要删除的节点
 *[safe_node]: 遍历时使用的安全节点
 *
 *@返回值：
 *[true]：成功从slist中删除节点
 *[false]：node不在slist之中
 **********************************************************/
static inline bool slist_del_node_safe(slist_t *slist, slist_node_t *node, slist_node_t **safe_node)
{
    slist_node_t *prev_node;
    slist_node_t *find_node;

    slist_foreach_record_prev(slist, find_node, prev_node) {
        if (find_node == node) {
            slist_node_del_next_safe(prev_node, safe_node);
            return true;
        }
    }

    return false;
}


/*********************************************************
 *@简要：
 ***将链表所有节点转移至接收链表的头部，
 ***转移完成后，原链表变成空
 *
 *@参数：
 *[slist]：被转移的链表
 *[recv_slist]: 接收节点的链表
 **********************************************************/
static inline void slist_nodes_transfer_to(slist_t *slist, slist_t *recv_slist)
{
    slist_node_t *tail, *node;

    if (!slist_is_empty(slist)) {
        slist_foreach_record_prev(slist, node, tail);

        tail->next = recv_slist->next;
        recv_slist->next = tail;

        slist_init(slist);
    }
}

#endif /* __INCLUDE_SLIST_H__ */
