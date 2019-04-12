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
public:
    typedef pair<const Key, T> value_type;

public:
    class RBT {
    public:
        struct node {
            value_type *value;
            bool color; // 0 -> black, 1 -> red
            node *son[2], *fa;
            node *last, *next;

            node() : value(nullptr), color(0) {
                son[0] = son[1] = fa = nullptr;
                last = next = nullptr;
            }
            node(const value_type &val, node* _nil = nullptr) : value(new value_type(val)) {
                color = 0;
                last = next = _nil;
                son[0] = son[1] = fa = _nil;
            }
            node(const node &o) {
                value = new value_type(*o.value);
                color = o.color;
                last = o.last, next = o.next;
                son[0] = o.son[0], son[1] = o.son[1];
                fa = o.fa;
            }
            ~node() {
                if (value != nullptr)
                    delete value;
            }
            node &operator=(const node &o) {
                if (this == &o)
                    return *this;
                delete value;
                value = new value_type(*o.value), color = o.color;
                son[0] = o.son[0], son[1] = o.son[1];
                fa = o.fa;
                last = o.last, next = o.next;
                return *this;
            }
        } *nil, *root;

        size_t _size;
        Compare cmp;

        void construct(node *&o, node *oo, node *f, node *oo_nil, node *&last_create) {
            if (oo == oo_nil) {
                o = nil;
                return;
            }
            o = new node(*oo->value, nil);
            o->fa = f;
            o->color = oo->color;

            construct(o->son[0], oo->son[0], o, oo_nil, last_create);

            last_create->next = o;
            o->last = last_create;
            last_create = o;

            construct(o->son[1], oo->son[1], o, oo_nil, last_create);
        }
    public:
        RBT() {
            nil = new node;
            nil->son[0] = nil->son[1] = nil->fa = nil;

            root = nil;
            _size = 0;
        }
        RBT(const RBT &o) {
            nil = new node;
            nil->son[0] = nil->son[1] = nil->fa = nil;

            node *last_create = nil;
            construct(root, o.root, nil, o.nil, last_create);
            last_create->next = nil;
            nil->last = last_create;
            _size = o._size;
        }
        ~RBT() {
            if (root != nil)
                clear(root);
            delete nil;
        }
        RBT &operator=(const RBT &o) {
            if (this == &o)
                return *this;

            if (root != nil)
                clear(root);
            root = nullptr;
            node *last_create = nil;
            construct(root, o.root, nil, o.nil, last_create);
            last_create->next = nil;
            nil->last = last_create;
            _size = o._size;
            return *this;
        }

        void clear(const node *p) {
            if (p->son[0] != nil)
                clear(p->son[0]);
            if (p->son[1] != nil)
                clear(p->son[1]);
            delete p;
        }

        node* find(const Key &k) const {
            node *p = root;
            while (p != nil && (cmp(p->value->first, k) || cmp(k, p->value->first)))
                p = cmp(k, p->value->first) ? p->son[0] : p->son[1];
            return p == nil ? nil : p;
        }

        node* find_insert(const Key &k) const {
            node *p = root;
            while (1) {
                if (!cmp(k, p->value->first) && !cmp(p->value->first, k)) // insert fail
                    return nil;
                if (cmp(k, p->value->first)) {
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

        node* begin() {
            if (_size == 0)
                return nil;
            node *p = root;
            while (p->son[0] != nil)
                p = p->son[0];
            return p;
        }
        node *end() {
            return nil;
        }
        const node* cbegin() const {
            if (_size == 0)
                return nil;
            const node *p = root;
            while (p->son[0] != nil)
                p = p->son[0];
            return p;
        }
        const node *cend() const {
            return nil;
        }

        void rotate(node *o) {
            node *f = o->fa, *ff = f->fa;
            int l = f->son[1] == o, r = l ^ 1;

            f == root ? root = o : ff->son[ff->son[1] == f] = o;
            o->fa = ff, f->fa = o;
            if (o->son[r] != nil)
                o->son[r]->fa = f;
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
                _size++;
                root = new node(val, nil);
                return root;
            }

            node *p = find_insert(val.first);
            if (p == nil) // insert fail
                return nil;

            _size++;
            node *q = new node(val, nil);
            q->fa = p;
            if (cmp(val.first, p->value->first)) {
                p->son[0] = q;
                q->last = p->last;
                p->last->next = q;
                p->last = q;
                q->next = p;
            } else {
                p->son[1] = q;
                q->next = p->next;
                p->next->last = q;
                p->next = q;
                q->last = p;
            }
            q->color = 1;

            insert_maintain(q);
            nil->color = 0;
            nil->son[0] = nil->son[1] = nil->fa = nil->next = nil->last = nil;
            return q;
        }

        void swap_info(node *x, node *y) {
            node *tmp1 = new node, *tmp2 = new node;

            *tmp1 = *x;
            x->son[0]->fa = tmp1, x->son[1]->fa = tmp1;
            x->fa->son[x->fa->son[1] == x] = tmp1;
            x->last->next = tmp1, x->next->last = tmp1;

            *tmp2 = *y;
            y->son[0]->fa = tmp2, y->son[1]->fa = tmp2;
            y->fa->son[y->fa->son[1] == y] = tmp2;
            y->last->next = tmp2, y->next->last = tmp2;

            *y = *tmp1;
            tmp1->son[0]->fa = y, tmp1->son[1]->fa = y;
            tmp1->fa->son[tmp1->fa->son[1] == tmp1] = y;
            tmp1->last->next = y, tmp1->next->last = y;

            *x = *tmp2;
            tmp2->son[0]->fa = x, tmp2->son[1]->fa = x;
            tmp2->fa->son[tmp2->fa->son[1] == tmp2] = x;
            tmp2->last->next = x, tmp2->next->last = x;

            delete tmp1;
            delete tmp2;
        }

        void erase_maintain(node *x) {
            while (x != root && x->color == 0) {
                int l = x->fa->son[1] == x, r = l ^ 1;
                node *w = x->fa->son[r];
                if (w->color == 1) {
                    w->color = 0;
                    x->fa->color = 1;
                    rotate(w);
                    w = x->fa->son[r];
                }
                if (w->son[0]->color == 0 && w->son[1]->color == 0) {
                    w->color = 1;
                    x = x->fa;
                } else {
                    if (w->son[r]->color == 0) {
                        w->son[l]->color = 0;
                        w->color = 1;
                        rotate(w->son[l]);
                        w = x->fa->son[r];
                    }
                    w->color = x->fa->color;
                    x->fa->color = 0;
                    w->son[r]->color = 0;
                    rotate(x->fa->son[r]);
                    break;
                }
            }
            x->color = 0;
        }
        void erase(node *z) {
            _size--;
            node *y = z->son[0] == nil || z->son[1] == nil ? z : z->next;
            if (y != z) {
                if (z == root)
                    root = y;
                swap_info(y, z);
                node *t = y;
                y = z, z = t;
                value_type *tmp = y->value;
                delete z->value;
                z->value = new value_type(*y->value);
            }

            node *x = y->son[0] != nil ? y->son[0] : y->son[1];
            x->fa = y->fa;
            if (y->fa == nil)
                root = x;
            else
                y->fa->son[y->fa->son[1] == y] = x;

            if (y->color == 0)
                erase_maintain(x);
            nil->color = 0;
            nil->son[0] = nil->son[1] = nil->fa = nil->next = nil->last = nil;

            y->last->next = y->next;
            y->next->last = y->last;
            delete y;
        }
    } tr;

	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */

public:
    class const_iterator;
	class iterator {
	    friend void map::erase(iterator pos);
	    friend const_iterator;

	    typedef pair<const Key, T> value_type;
	    typedef value_type&         reference;
	    typedef value_type*           pointer;

    private:
	    map *_map;
	    typename RBT::node *p;

	public:
		iterator() : _map(nullptr), p(nullptr) {}
		iterator(const iterator &o) : _map(o._map), p(o.p) {}
		iterator(map *__map, typename RBT::node *_p) : _map(__map), p(_p) {}

		iterator operator++(int) {
            if (*this == _map->end())
                throw invalid_iterator();
            iterator res(*this);
		    p = p->next;
		    return res;
		}
		iterator &operator++() {
            if (*this == _map->end())
                throw invalid_iterator();
		    p = p->next;
		    return *this;
		}
		iterator operator--(int) {
		    if (*this == _map->begin())
		        throw invalid_iterator();
            iterator res(*this);
            if (p == _map->tr.nil) {
                p = _map->tr.root;
                while (p->son[1] != _map->tr.nil)
                    p = p->son[1];
		    } else
                p = p->last;
            return res;
		}
		iterator &operator--() {
            if (*this == _map->begin())
                throw invalid_iterator();
            if (p == _map->tr.nil) {
                p = _map->tr.root;
                while (p->son[1] != _map->tr.nil)
                    p = p->son[1];
            } else
                p = p->last;
            return *this;
		}

		reference operator*() const {
		    return *p->value;
		}
		pointer operator->() const noexcept {
		    return &(*p->value);
		}

        bool operator==(const iterator &o) const {
            return _map == o._map && p == o.p;
        }
        bool operator==(const const_iterator &o) const {
            return _map == o._map && p == o.p;
        }
        bool operator!=(const iterator &o) const {
            return _map != o._map || p != o.p;
        }
        bool operator!=(const const_iterator &o) const {
            return _map != o._map || p != o.p;
        }
	};
    class const_iterator {
        friend iterator;

        typedef const pair<const Key, T> value_type;
        typedef value_type&         reference;
        typedef value_type*           pointer;

    private:
        const map *_map;
        const typename RBT::node *p;

    public:
        const_iterator() : _map(nullptr), p(nullptr) {}
        const_iterator(const iterator &o) : _map(o._map), p(o.p) {}
        const_iterator(const const_iterator &o) : _map(o._map), p(o.p) {}
        const_iterator(const map *__map, const typename RBT::node *_p) : _map(__map), p(_p) {}

        const_iterator operator++(int) {
            if (*this == _map->cend())
                throw invalid_iterator();
            const const_iterator res(*this);
            p = p->next;
            return res;
        }
        const_iterator &operator++() {
            if (*this == _map->cend())
                throw invalid_iterator();
            p = p->next;
            return *this;
        }
        const_iterator operator--(int) {
            if (*this == _map->cbegin())
                throw invalid_iterator();
            const const_iterator res(*this);
            if (p == _map->tr.nil) {
                p = _map->tr.root;
                while (p->son[1] != _map->tr.nil)
                    p = p->son[1];
            } else
                p = p->last;
            return res;
        }
        const_iterator &operator--() {
            if (*this == _map->cbegin())
                throw invalid_iterator();
            if (p == _map->tr.nil) {
                p = _map->tr.root;
                while (p->son[1] != _map->tr.nil)
                    p = p->son[1];
            } else
                p = p->last;
            return *this;
        }

        reference operator*() const {
            return *p->value;
        }
        pointer operator->() const noexcept {
            return &(*p->value);
        }

        bool operator==(const iterator &o) const {
            return _map == o._map && p == o.p;
        }
        bool operator==(const const_iterator &o) const {
            return _map == o._map && p == o.p;
        }
        bool operator!=(const iterator &o) const {
            return _map != o._map || p != o.p;
        }
        bool operator!=(const const_iterator &o) const {
            return _map != o._map || p != o.p;
        }
    };


public:
    map() {}
    map(const map &o) = default;
    ~map() = default;
    map &operator=(const map &o) = default;

    void clear() {
        if (tr.root != tr.nil)
            tr.clear(tr.root);
        tr._size = 0;
        tr.root = tr.nil;
    }

	T &at(const Key &k) {
        typename RBT::node *p = tr.find(k);
	    if (p != tr.nil)
	        return p->value->second;
	    throw index_out_of_bound();
	}
	const T& at(const Key &k) const {
        const typename RBT::node *p = tr.find(k);
        if (p != tr.nil)
            return p->value->second;
        throw index_out_of_bound();
	}

	T &operator[](const Key &k) {
	    typename RBT::node *p = tr.find(k);
	    if (p == tr.nil)
	        return tr.insert(value_type(k, T()))->value->second;
	    return p->value->second;
	}
	const T& operator[](const Key &k) const {
        const typename RBT::node *p = tr.find(k);
        if (p != tr.nil)
            return p->value->second;
        throw index_out_of_bound();
	}

	iterator begin() {
	    typename RBT::node *p = tr.begin();
	    return iterator(this, p);
	}
	const_iterator cbegin() const {
        const typename RBT::node *p = tr.cbegin();
        return const_iterator(this, p);
	}
	iterator end() {
	    typename RBT::node *p = tr.end();
	    return iterator(this, p);
	}
	const_iterator cend() const {
        const typename RBT::node *p = tr.cend();
        return const_iterator(this, p);
	}

	bool empty() const {
	    return tr._size == 0;
	}
	size_t size() const {
	    return tr._size;
	}

	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
	    typename RBT::node *p = tr.insert(value);
	    if (p == tr.nil)
            return pair<iterator, bool>(iterator(this, tr.find(value.first)), false);
	    return pair<iterator, bool>(iterator(this, p), true);
	}

	/**
	 * erase the element at pos.
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
	    if (this != pos._map || pos == end())
            throw invalid_iterator();
	    tr.erase(pos.p);
	}

	size_t count(const Key &key) const {
	    return tr.find(key) != tr.nil ? 1 : 0;
	}

	iterator find(const Key &key) {
	    typename RBT::node *p = tr.find(key);
	    return p == tr.nil ? end() : iterator(this, p);
	}
	const_iterator find(const Key &key) const {
	    const typename RBT::node *p = tr.find(key);
        return p == tr.nil ? cend() : const_iterator(this, p);
	}
};

}

#endif
