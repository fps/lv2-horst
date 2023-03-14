#ifndef HEAP_HH
#define HEAP_HH

#include <list>
#include <iostream>

#include <lart/junk.h>

namespace lart
{
	struct heap {
		std::list<junk_base_ptr> m_junk;

		heap() { }

		virtual ~heap() { }

		/**
			@brief Add a T to the heap.
			
			Returns a std::shared_ptr<junk<T>> with a reference count of 2 since it is also stored on the heap.
		*/
		template <class T>
		std::shared_ptr<junk<T>> add(const T &t) {
			auto ret = std::make_shared<junk<T>>(t);
			m_junk.push_back(ret);
			return ret;
		}
	
		/**
			@brief Iterate over list of junk and remove those with reference count == 1.
			
			Note that this function has to be called in the same thread that writes commands, otherwise
			references might go away between the construction of a junk and binding it to a functor
			that uses it.
		*/
		virtual void cleanup() {
			for (auto it = m_junk.begin(); it != m_junk.end();) {
				if (it->unique()) {
					it = m_junk.erase(it);
				} else {
					++it;
				}
			}	
		}
	};
	
	typedef std::shared_ptr<heap> heap_ptr;
	
	/**
		@brief Utility function to save some keystrokes.
	*/
	inline heap_ptr make_heap()
	{
		return std::make_shared<heap>();
	}
}
	

#endif
