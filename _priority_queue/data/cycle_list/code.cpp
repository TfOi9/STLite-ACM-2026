#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <deque>
#include <stdexcept>

#include "../../src/priority_queue.hpp"

namespace {

int g_fail_count = 0;

void check(bool cond, const std::string &name) {
    if (!cond) {
        ++g_fail_count;
        std::cout << "[FAIL] " << name << '\n';
    }
}

void check_list_equals_reference(sjtu::CycleList<int> lst, const std::deque<int> &ref, const std::string &name_prefix) {
    check(lst.size() == ref.size(), name_prefix + " size");
    check(lst.empty() == ref.empty(), name_prefix + " empty");

    for (size_t i = 0; i < ref.size(); ++i) {
        bool front_throw = false;
        int got = 0;
        try {
            got = lst.front();
        } catch (...) {
            front_throw = true;
        }
        check(!front_throw, name_prefix + " front should not throw while non-empty");
        if (!front_throw) {
            check(got == ref[i], name_prefix + " order");
        }
        lst.pop();
    }

    check(lst.size() == 0, name_prefix + " drained size");
    check(lst.empty(), name_prefix + " drained empty");

    bool front_throw = false;
    try {
        (void)lst.front();
    } catch (const sjtu::container_is_empty &) {
        front_throw = true;
    } catch (...) {
    }
    check(front_throw, name_prefix + " front should throw after drained");
}

void test_empty_and_basic_push_pop() {
    sjtu::CycleList<int> lst;
    check(lst.size() == 0, "empty list size should be 0");
    check(lst.empty(), "empty list empty() should be true");

    bool front_throw = false;
    try {
        (void)lst.front();
    } catch (const sjtu::container_is_empty &) {
        front_throw = true;
    } catch (...) {
    }
    check(front_throw, "front() on empty should throw container_is_empty");

    lst.push(10);
    check(lst.size() == 1, "size after one push");
    check(!lst.empty(), "empty() after one push should be false");
    check(lst.front() == 10, "front after one push");

    lst.push(20);
    lst.push(30);
    check(lst.size() == 3, "size after three pushes");
    check(lst.front() == 30, "front should be latest pushed value");

    lst.pop();
    check(lst.size() == 2, "size after one pop");
    check(lst.front() == 20, "front after pop");

    lst.pop();
    lst.pop();
    check(lst.size() == 0, "size after popping all");
    check(lst.empty(), "empty() after popping all");
}

void test_copy_constructor_deep_copy() {
    sjtu::CycleList<int> a;
    for (int i = 1; i <= 100; ++i) {
        a.push(i);
    }

    sjtu::CycleList<int> b(a);
    check(b.size() == a.size(), "copy ctor size equality");
    check(b.front() == a.front(), "copy ctor front equality");

    a.pop();
    a.push(999);

    check(b.front() != a.front(), "copy should not share head/front");
    check(b.size() == 100, "copy size should remain unchanged");
}

void test_merge_basic() {
    sjtu::CycleList<int> a;
    sjtu::CycleList<int> b;

    for (int i = 1; i <= 5; ++i) a.push(i);    // a front: 5
    for (int i = 101; i <= 105; ++i) b.push(i); // b front: 105

    a.merge(b);

    check(a.size() == 10, "merge size should be sum");
    check(!a.empty(), "merged list should be non-empty");

    // For the current push/pop contract, front should keep a's old head.
    check(a.front() == 5, "front after merge should keep receiver head");

    // Typical move-merge expectation: other should become empty.
    check(b.size() == 0, "merged-from list size should become 0");
    check(b.empty(), "merged-from list should become empty");
}

void test_merge_with_empty_lists() {
    sjtu::CycleList<int> a;
    sjtu::CycleList<int> b;

    for (int i = 1; i <= 20; ++i) a.push(i);

    a.merge(b);
    check(a.size() == 20, "merge empty into non-empty should keep size");
    check(a.front() == 20, "merge empty into non-empty should keep front");

    sjtu::CycleList<int> c;
    for (int i = 100; i < 120; ++i) c.push(i);
    sjtu::CycleList<int> d;
    d.merge(c);

    check(d.size() == 20, "merge into empty should take all nodes");
    check(!d.empty(), "merge into empty should become non-empty");
    check(c.size() == 0, "merged-from when receiver empty should become empty");
}

void test_large_scale_push_pop() {
    sjtu::CycleList<int> lst;
    const int n = 200000;

    for (int i = 1; i <= n; ++i) {
        lst.push(i);
    }
    check(lst.size() == static_cast<size_t>(n), "large push size check");

    for (int i = n; i >= 1; --i) {
        if (lst.front() != i) {
            check(false, "large pop order check");
            break;
        }
        lst.pop();
    }

    check(lst.empty(), "large push/pop final empty check");
}

void test_randomized_against_reference() {
    sjtu::CycleList<int> lst;
    std::deque<int> ref;

    std::mt19937 rng(20260323);
    std::uniform_int_distribution<int> op_dist(0, 99);
    std::uniform_int_distribution<int> val_dist(-1000000, 1000000);

    const int steps = 50000;
    for (int s = 0; s < steps; ++s) {
        int op = op_dist(rng);

        if (op < 60 || ref.empty()) {
            int v = val_dist(rng);
            lst.push(v);
            ref.push_front(v);
        } else {
            lst.pop();
            ref.pop_front();
        }

        check(lst.size() == ref.size(), "randomized size sync");
        check(lst.empty() == ref.empty(), "randomized empty sync");

        if (!ref.empty()) {
            check(lst.front() == ref.front(), "randomized front sync");
        } else {
            bool threw = false;
            try {
                (void)lst.front();
            } catch (const sjtu::container_is_empty &) {
                threw = true;
            } catch (...) {
            }
            check(threw, "randomized front throw on empty");
        }
    }
}

void test_empty_front_throw_stability() {
    sjtu::CycleList<int> lst;
    for (int i = 0; i < 20000; ++i) {
        bool threw = false;
        try {
            (void)lst.front();
        } catch (const sjtu::container_is_empty &) {
            threw = true;
        } catch (...) {
        }
        check(threw, "repeat front() on empty should always throw");
    }
}

void test_merge_order_invariant() {
    sjtu::CycleList<int> a;
    sjtu::CycleList<int> b;

    std::deque<int> ra, rb;
    for (int i = 1; i <= 8; ++i) {
        a.push(i);
        ra.push_front(i);
    }
    for (int i = 100; i <= 108; ++i) {
        b.push(i);
        rb.push_front(i);
    }

    a.merge(b);
    ra.insert(ra.end(), rb.begin(), rb.end());
    rb.clear();

    check_list_equals_reference(a, ra, "merge order invariant receiver");
    check_list_equals_reference(b, rb, "merge order invariant donor");
}

void test_large_chain_merge() {
    static constexpr int kBucket = 32;
    static constexpr int kPerBucket = 4000;

    sjtu::CycleList<int> lists[kBucket];
    std::deque<int> refs[kBucket];

    for (int i = 0; i < kBucket; ++i) {
        for (int j = 1; j <= kPerBucket; ++j) {
            int v = i * 1000000 + j;
            lists[i].push(v);
            refs[i].push_front(v);
        }
    }

    for (int i = 1; i < kBucket; ++i) {
        lists[0].merge(lists[i]);
        refs[0].insert(refs[0].end(), refs[i].begin(), refs[i].end());
        refs[i].clear();
    }

    check_list_equals_reference(lists[0], refs[0], "large chain merge receiver");
    for (int i = 1; i < kBucket; ++i) {
        check_list_equals_reference(lists[i], refs[i], "large chain merge donor");
    }
}

void test_multi_list_random_fuzz() {
    static constexpr int kListCount = 8;
    static constexpr int kSteps = 120000;

    sjtu::CycleList<int> lists[kListCount];
    std::deque<int> refs[kListCount];

    std::mt19937 rng(20260324);
    std::uniform_int_distribution<int> op_dist(0, 99);
    std::uniform_int_distribution<int> idx_dist(0, kListCount - 1);
    std::uniform_int_distribution<int> val_dist(-2000000000, 2000000000);

    for (int step = 1; step <= kSteps; ++step) {
        int op = op_dist(rng);

        if (op < 55) {
            int id = idx_dist(rng);
            int v = val_dist(rng);
            lists[id].push(v);
            refs[id].push_front(v);
        } else if (op < 80) {
            int id = idx_dist(rng);
            for (int t = 0; t < kListCount && refs[id].empty(); ++t) {
                id = (id + 1) % kListCount;
            }
            if (!refs[id].empty()) {
                lists[id].pop();
                refs[id].pop_front();
            }
        } else if (op < 95) {
            int dst = idx_dist(rng);
            int src = idx_dist(rng);
            if (dst == src) {
                src = (src + 1) % kListCount;
            }
            lists[dst].merge(lists[src]);
            refs[dst].insert(refs[dst].end(), refs[src].begin(), refs[src].end());
            refs[src].clear();
        } else {
            int id = idx_dist(rng);
            sjtu::CycleList<int> copied(lists[id]);
            check_list_equals_reference(copied, refs[id], "copy snapshot consistency");
        }

        if (step % 257 == 0) {
            for (int i = 0; i < kListCount; ++i) {
                check(lists[i].size() == refs[i].size(), "fuzz periodic size sync");
                check(lists[i].empty() == refs[i].empty(), "fuzz periodic empty sync");

                if (!refs[i].empty()) {
                    check(lists[i].front() == refs[i].front(), "fuzz periodic front sync");
                } else {
                    bool threw = false;
                    try {
                        (void)lists[i].front();
                    } catch (const sjtu::container_is_empty &) {
                        threw = true;
                    } catch (...) {
                    }
                    check(threw, "fuzz periodic empty front throw");
                }
            }
        }
    }

    for (int i = 0; i < kListCount; ++i) {
        check_list_equals_reference(lists[i], refs[i], "fuzz final full sync");
    }
}

void test_copy_after_heavy_mutation() {
    sjtu::CycleList<int> base;
    std::deque<int> ref;

    for (int i = 0; i < 70000; ++i) {
        base.push(i - 35000);
        ref.push_front(i - 35000);
    }

    sjtu::CycleList<int> snap(base);
    check_list_equals_reference(snap, ref, "copy after heavy mutation initial snapshot");

    for (int i = 0; i < 30000; ++i) {
        base.pop();
        ref.pop_front();
    }

    for (int i = 0; i < 30000; ++i) {
        base.push(1000000000 - i);
        ref.push_front(1000000000 - i);
    }

    sjtu::CycleList<int> snap2(base);
    check_list_equals_reference(snap2, ref, "copy after heavy mutation second snapshot");
}

} // namespace

int main() {
    test_empty_and_basic_push_pop();
    test_copy_constructor_deep_copy();
    test_merge_basic();
    test_merge_with_empty_lists();
    test_large_scale_push_pop();
    test_randomized_against_reference();
    test_empty_front_throw_stability();
    test_merge_order_invariant();
    test_large_chain_merge();
    test_multi_list_random_fuzz();
    test_copy_after_heavy_mutation();
    
    if (g_fail_count == 0) {
        std::cout << "ALL TESTS PASSED" << '\n';
        return 0;
    }

    std::cout << "TOTAL FAILURES: " << g_fail_count << '\n';
    return 1;
}
