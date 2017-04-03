/*
 *
 * 	@Date:   2016-04-05
 * 	@Last Modified by:   kmrocki
 * 	@Last Modified time: 2016-04-05 18:36:57
 *
 * 	Abstract parameter class, provides interface for algorithms
 *	implementing learning (optimization.h)
 *
 *	Similar abstract class State
 */

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include <map>
#include <containers/Dict.h>

template <typename T>
class Parameters : public Dict<T> {

  public:

	/* default constr */
	Parameters<T>() : Dict<T>() {};

	/* main constr */
	Parameters<T> ( std::string name,
	                std::initializer_list<std::tuple<std::string, size_t, size_t>>
	                args, std::string id ) :
		Dict<T> ( name, args, id ) { };

	/* copy constr */
	Parameters<T> ( const Parameters<T> &other ) : Dict<T> (
		    other ) { N = other.N; }

	/* assignment op */
	Parameters<T> &operator= ( const Parameters<T> &other ) {

		N = other.N;
		Dict<T>::operator= ( other );
		return *this;

	}

	template <typename otherType>
	Parameters<T> &operator= ( const Parameters<otherType> &other ) {

		N = other.N;
		Dict<T>::operator= ( other );
		return *this;

	}

	size_t N;

};

#endif /*__PARAMETERS_H__*/
