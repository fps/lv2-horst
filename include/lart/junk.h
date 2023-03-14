#ifndef LART_JUNK_HH
#define LART_JUNK_HH

#include <memory>
#include <iostream>

#include <pthread.h>

namespace lart
{
	/**
		@brief The base class for all junk types.
	
		See junk.h for more information.
	*/
	struct junk_base {
		virtual ~junk_base() { }
	};
	
	typedef std::shared_ptr<junk_base> junk_base_ptr;

	/**
		@brief The base class for all things to be managed by the heap.
	
		See heap.h for more details.
	*/
	template <class T>
	struct junk : public junk_base {
		T t;
	
		junk(const T &t = T()) : t(t) 
		{ 
			//std::cout << "junk(): " << this << " " << pthread_self() << std::endl;
		}

		virtual ~junk() 
		{
			//std::cout << "~junk(): " << this << " " << pthread_self() << std::endl;
		}
	};
}


#endif
