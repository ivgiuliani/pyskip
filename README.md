PySkip
======


What is pyskip
--------------
Pyskip is a CPython extension module that allows the use skip lists as
dictionaries.

Q&A
---
- *What's a skiplist?*

  It's a data structure that can be used as a dictionary. You can think
  of skiplists as enhanced singly linked lists.

- *What's the average and worst search time?*

  Skip list have O(logn) time in the average case and O(n) in the worst
  case.

- *What's the average and worst insertion time?*

  You can consider the skip list as a enhanced singly linked list, thus
  in order to insert an item you need to find its correct position in
  the list and then add a new pointer. This leads to a O(n) time in the
  worst case and O(logn) time in the average case (it's safe to assume
  that creating a new pointer is a O(1) operation).

- *What's the difference between a skiplist and a classic dictionary?*

  Besides the complexity analysis, a skiplist saves keys in sorted
  order.

- *So this is true with pyskip too?*

  Yes. We use the order relation defined by the \_\_lt\_\_ method.

- *Is pyskip production ready?*

  Short answer: no. Long answer: it's an experimental project that
  showed some interesting results but it's considerably slower than
  classical python dictionaries.

- *I can't run the tests, why?*

  That's probably because we rely on unittest2. See instructions on
	how to install them here: http://pypi.python.org/pypi/unittest2

