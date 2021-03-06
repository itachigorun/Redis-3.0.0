/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
/*
 * 创建新链表
 *
 * Parameter: void
 * Return: 链表指针,失败返回NULL
 * T = O(1)
 * S = O(1)
 */
list *listCreate(void)
{
    struct list *list;

   // 申请内存
    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;

   //初始化属性
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
 /*
  * 释放链表中到所有节点
  * Parameter: 链表指针
  * Return : void
  * T = O(n)
  * S = O(1)
  */
void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;

    // 指向头节点
    current = list->head;

    // 遍历整个链表
    len = list->len;
    while(len--) {
        next = current->next;

        // 如果有free函数，释放当前节点的值
        if (list->free) list->free(current->value);

        // 释放节点结构
        zfree(current);
        current = next;
    }

    // 释放链表结构
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
 /*
  * 头插法插入节点
  * 返回链表，失败返回NULL
  * parameter: 链表，值
  * T = O(1)
  * S = O(1)
  */
list *listAddNodeHead(list *list, void *value)
{
    listNode *node;

    // 分配内存
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;

    // 链表为空时
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    // 链表不为空时
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }

    // 链表节点数加一
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
 /*
  * 尾插法添加链表节点
  * 返回链表，失败返回NULL
  * parameter: 链表，值
  * T = O(1)
  * S = O(1)
  */
list *listAddNodeTail(list *list, void *value)
{
    listNode *node;

    // 分配内存
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;

    // 链表为空时
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    // 链表不为空时
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }

    // 链表数加一
    list->len++;
    return list;
}

/*
 * 创建一个新节点，after==1时，插入到old_node之后，
 *                 after==0时，插入到old_node之前
 * parameter： 链表，给定节点，值，标志值
 * T = O(1)
 * S = O(1)
 *
 */
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    listNode *node;

    // 创建新节点
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;

    // after==1，在给定节点后添加新节点
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        // 给定节点是尾节点时
        if (list->tail == old_node) {
            list->tail = node;
        }
    // after==0, 在给定节点前添加新节点
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        // 给定节点是头节点时
        if (list->head == old_node) {
            list->head = node;
        }
    }

    // 更新新节点到前置节点
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    // 更新新节点到后置节点
    if (node->next != NULL) {
        node->next->prev = node;
    }

    // 链表数加一
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
 /*
  * 从链表中删除给定节点
  * 返回void
  * parameter: 链表，待删除节点
  * T = O(1)
  * S = O(1)
  */
void listDelNode(list *list, listNode *node)
{
    // 调整前置节点指针
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;

    // 调整后置节点指针
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    // 释放节点值
    if (list->free) list->free(node->value);

    // 释放节点
    zfree(node);

    // 链表数减一
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
 /*
  * 初始化链表迭代器，之后每次调用listnext()都会返回当前指向链表节点，
  * 并将迭代器指向下一个节点或前一节点
  * parameter:
  *           AL_START_HEAD:从表头向表尾迭代
  *           AL_START_TAIL:从表尾向表头迭代
  * T = O(1)
  * S = O(1)
  */
listIter *listGetIterator(list *list, int direction)
{
    listIter *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;

    // 从头节点开始迭代
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;

    // 记录迭代方向
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
/*
 * 释放链表迭代器具
 * T = O(1)
 * S = O(1)
 */
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
/*
 * 将迭代器重置为指向表头，并从表头往下迭代
 * T = O(1)
 * S = O(1)
 */
void listRewind(list *list, listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

/*
 * 将迭代器重置为指向表尾，并从表尾往上迭代
 * T = O(1)
 * S = O(1)
 */
void listRewindTail(list *list, listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
 /*
  * 返回迭代器当前指向节点，并将迭代器指向下一节点,可删除当前节点
  * T = O(1)
  * S = O(1)
  */
listNode *listNext(listIter *iter)
{
    listNode *current = iter->next;

    // 迭代器所指节点不为空，则根据方向，对应指向下一节点或前一节点
    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
 /*
  * 复制链表，成功返回orig到复制，因内存分配失败，返回NULL
  * 如果链表有设置复制函数dup，则用dup复制节点数据，否则，新链表节点指向原链表节点数据
  * 输入链表不会改变
  * T = O(N)
  * S = O(N)
  */
list *listDup(list *orig)
{
    list *copy;
    listIter *iter;
    listNode *node;

    // 创建新链表
    if ((copy = listCreate()) == NULL)
        return NULL;

    // 复制链表处理函数
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;

    // 创建迭代器
    iter = listGetIterator(orig, AL_START_HEAD);

    // 开始迭代
    while((node = listNext(iter)) != NULL) {
        void *value;

        // 复制节点数据
        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy);
                listReleaseIterator(iter);
                return NULL;
            }
        } else
            value = node->value;

        // 尾插法插入数据
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }

    // 释放迭代器
    listReleaseIterator(iter);

    // 返回新建链表
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
 /*
  * 从表头开始遍历，寻找和key相等的节点
  * 比较函数由match函数比较，或直接比较
  * 成功返回第一个与key值相等的节点，或返回NULL
  * T = O(N)
  * S = O(1)
  */
listNode *listSearchKey(list *list, void *key)
{
    listIter *iter;
    listNode *node;

    // 创建迭代器，从表头开始查找
    iter = listGetIterator(list, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {

               // 如果找到节点，释放迭代器，并返回节点指针
                listReleaseIterator(iter);
                return node;
            }
        } else {
            if (key == node->value) {
                listReleaseIterator(iter);
                return node;
            }
        }
    }

    // 没有找到节点，释放迭代器，返回NULL
    listReleaseIterator(iter);
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
 /*
  * 返回链表在给定下表上到值（索引）
  * 索引从0开始，也可以时负数，-1表示最后一个节点
  * 成功返回节点指针，否则返回NULL
  * T = O(N)
  * S = O(1)
  */
listNode *listIndex(list *list, long index) {
    listNode *n;

    // 索引为负数，从表尾开始查找
    if (index < 0) {
        index = (-index)-1;
        n = list->tail;

        // 表尾往前，索引先为0或者节点指向NULL退出循环
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
/*
 * 取出链表到表尾节点，将它移动到表头，成为新到表头节点
 * T = O(1)
 * S = O(1)
 */
void listRotate(list *list) {
    listNode *tail = list->tail;

    if (listLength(list) <= 1) return;

    /* Detach current tail */
    // 重置表尾节点
    list->tail = tail->prev;
    list->tail->next = NULL;

    /* Move it as head */
    // 插入到表头成为新表头节点
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
