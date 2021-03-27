//Author: Ye-zixiao
//Date: 2021-03-06

#include <iostream>
using namespace std;

struct ListNode {
  int val;
  ListNode *next;

  explicit ListNode(int v, ListNode *nx = nullptr)
      : val(v), next(nx) {}
  ListNode() : ListNode(-1) {}
};

struct List {
  ListNode head;

  List() : head(-1) {}

  List &add(ListNode *node) {
      node->next = head.next;
      head.next = node;
      return *this;
  }
};

void print(const ListNode *head) {
    const ListNode *pNode = head;
    while (pNode) {
        cout << pNode->val << ' ';
        pNode = pNode->next;
    }
    if (head) cout << endl;
}

//不破坏原来的链表
ListNode *merge(ListNode *lhs, ListNode *rhs) {
    if (!lhs) return rhs;
    else if (!rhs) return lhs;
    ListNode head(-1), *pNode = &head;
    while (lhs && rhs) {
        if (lhs->val <= rhs->val) {
            pNode->next = lhs;
            lhs = lhs->next;
        } else {
            pNode->next = rhs;
            rhs = rhs->next;
        }
        pNode = pNode->next;
    }
    if (rhs) pNode->next = rhs;
    if (lhs) pNode->next = lhs;
    return head.next;
}

//一个元素的迁移操作
void split(ListNode *prevTo, ListNode *prevFrom) {
    if (prevTo && prevFrom && prevFrom->next) {
        ListNode *tmp = prevFrom->next;
        prevFrom->next = tmp->next;
        tmp->next = prevTo->next;
        prevTo->next = tmp;
    }
}

ListNode *sort(ListNode *head) {
    if (head && head->next) {
        ListNode prevHead(-1, head);
        ListNode carry;
        ListNode counter[64];
        int fill = 0;

        while (prevHead.next) {
            split(&carry, &prevHead);

            int i = 0;
            while (i < fill && counter[i].next) {
                counter[i].next = merge(carry.next, counter[i].next);
                carry.next = nullptr;
                swap(carry.next, counter[i].next);
                ++i;
            }
            swap(carry.next, counter[i].next);
            if (i == fill) ++fill;
        }

        for (int j = 1; j < fill; ++j)
            counter[j].next = merge(counter[j].next, counter[j - 1].next);
        swap(head, counter[fill - 1].next);
    }
    return head;
}

int main() {
    List lst;
    for (int i = 0; i < 100; i++)
        lst.add(new ListNode(i));

    ListNode *result = sort(lst.head.next);
    cout << "result:" << endl;
    print(result);

    return 0;
}