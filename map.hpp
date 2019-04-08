#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {
private:

    typedef pair<const Key, T> value_type;

    class RBT {
    public:
        struct node {
            value_type value;
            bool color; // 0 -> black, 1 -> red
            node *son[2], *fa;
            node *last, next;

            node(const value_type &val, node* _nil = nullptr) : value(val) {
                color = 0;
                last = next = _nil;
                son[0] = son[1] = fa = _nil;
            }
        } *nil, *root;

        size_t _size;
        Compare cmp;

    public:
        RBT() {
            nil = new node();
            nil->son[0] = nil->son[1] = nil->fa = nil;

            root = nil;
            _size = 0;
        }
        ~RBT() {
            if (root != nil)
                clear(root);
            delete nil;
        }

        void clear(const node *p) {
            if (p->son[0] != nil)
                clear(p->son[0]);
            if (p->son[1] != nil)
                clear(p->son[1]);
            delete p;
        }

        size_t size() {
            return _size;
        }

        node* find(const Key &k) {
            node *p = root;
            while (p != nil && p->value.first != k)
                p = cmp(k, p->value.first) ? p->son[0] : p->son[1];
            return p == nil ? nil : p;
        }

        node* find_insert(const Key &k) {
            node *p = root;
            while (1) {
                if (!cmp(k, p->value.first) && !cmp(p->value.first, k)) // insert fail
                    return nil;
                if (cmp(k, p->value.first)) {
                    if (p->son[0] == nil)
                        return p;
                    p = p->son[0];
                } else {
                    if (p->son[1] == nil)
                        return p;
                    p = p->son[1];
                }
            }
        }

        void rotate(node *o) {
            node *f = o->fa, *ff = f->fa;
            int l = f->son[1] == o, r = l ^ 1;

            f == root ? root = o : ff->son[ff->son[1] == f] = o;
            o->fa = ff, f->fa = o, o->son[r]->fa = f;
            f->son[l] = o->son[r], o->son[r] = f;
        }

        void insert_maintain(node *z) {
            while (z->fa->color == 1) {
                int r = z->fa == z->fa->fa->son[0], l = r ^ 1;
                if (z->fa == z->fa->fa->son[l]) {
                    node *y = z->fa->fa->son[r];
                    if (y->color == 1) {
                        z->fa->color = 0;
                        y->color = 0;
                        z->fa->fa->color = 1;
                        z = z->fa->fa;
                    } else {
                        if (z == z->fa->son[r]) {
                            z = z->fa;
                            rotate(z->son[r]);
                        }
                        z->fa->color = 0;
                        z->fa->fa->color = 1;
                        rotate(z->fa->fa->son[l]);
                    }
                }
            }
            root->color = 0;
        }

        node* insert(const value_type &val) {
            if (root == nil) {
                root = new node(val, nil);
                return root;
            }

            node *p = find_insert(val.first);
            if (p == nil) // insert fail
                return nil;

            node *q = new node(val, nil);
            q->fa = p;
            if (cmp(val.first, p->value.first)) {
                p->son[0] = q;
                q->last = p->last;
                p->last->next = q;
                p->last = q;
                q->next = p;
            } else {
                p->son[1] = q;
                q->next = p->next;
                p->next.last = q;
                p->next = q;
                q->last = p;
            }
            q->color = 1;

            insert_maintain(q);
        }

        void erase(node *z) { //TODO
            node *y = z->son[0] == nil || z->son[1] == nil ? z : z->next;
            node *x = y->son[0] != nil ? y->son[0] : y->son[1];
            x->fa = y->fa;
            if (y->fa == nil)
                root = x;
            else
                y->fa->son[y->fa->son[1] == y] = x;

            if (y != z)
                z->value = y->value;

        }
    } tr;

	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */

	class iterator {
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
	public:
		iterator() {
			// TODO
		}
		iterator(const iterator &other) {
			// TODO
		}
		/**
		 * return a new iterator which pointer n-next elements
		 *   even if there are not enough elements, just return the answer.
		 * as well as operator-
		 */
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {}
		bool operator==(const iterator &rhs) const {}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {}

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {}
	};
	using const_iterator = iterator;

	/**
	 * TODO two constructors
	 */
	map() {}
	map(const map &other) {}
	/**
	 * TODO assignment operator
	 */
	map &operator=(const map &other) {}
	/**
	 * TODO Destructors
	 */
	~map() {}

	T& at(const Key &k) {
        typename RBT::node *p = tr.find(k);
	    if (p != tr.nil)
	        return p;
	    throw index_out_of_bound();
	}
	const T & at(const Key &k) const {
        const typename RBT::node *p = tr.find(k);
        if (p != tr.nil)
            return p;
        throw index_out_of_bound();
	}

	T & operator[](const Key &k) {
	    typename RBT::node *p = tr.find(k);
	    if (p == tr.nil)
	        return tr.insert(value_type(k, T()))->value.second;
	    return p->value.second;
	}

	const T & operator[](const Key &k) const {
        const typename RBT::node *p = tr.find(k);
        if (p != tr.nil)
            return p;
        throw index_out_of_bound();
	}

	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {}
	const_iterator cbegin() const {}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {}
	const_iterator cend() const {}

	bool empty() const {
	    return tr.size() == 0;
	}
	size_t size() const {
	    return tr.size();
	}

	/**
	 * clears the contents
	 */
	void clear() {}

	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {}

	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {}

	size_t count(const Key &key) const {
	    return tr.find(key) != tr.nil;
	}

	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {}
	const_iterator find(const Key &key) const {}
};

}

#endif
