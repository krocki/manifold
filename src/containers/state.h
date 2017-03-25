/*
 * 	@Date:   2016-04-05
 * 	@Last Modified by:   kmrocki
 * 	@Last Modified time: 2016-04-05 18:36:57
 *
 * 	Abstract state definition
 *
 *	MatrixArray is a dynamic array of matrices, so element of base class
 *	State can hold any number of sub - states (kept in var 'matrices')
 *	sub - states are accessible by name with operator []
 *	for example if s is of class State and derived State
 *	contains sub - state 'x' then s['x'] will return sub - matrix x
 *
 */

#ifndef __STATE_H__
#define __STATE_H__

#include <map>
#include <containers/matrixarray.h>

/* Bare class State is just a MatrixArray + it defines some virtual methods which need to be implemented */
template <typename T>
class State : public MatrixArray<T> {

  public:

	/* default constr */
	State<T>() : MatrixArray<T>() {};

	/* main constr */
	State<T> ( size_t M, size_t N, size_t B, std::string name,
	           std::initializer_list<std::tuple<std::string, size_t, size_t>>
	           args, std::string id ) :
		MatrixArray<T> ( name, args, id ) {

		/* add {x, y} as default states, assuming that any state has some inputs and outputs */
		MatrixArray<T>::add (

		{

			std::make_tuple ( "x", B, M ), 	// in
			std::make_tuple ( "y", B, N ) 	// out

		} );

	};

	/* copy constr */
	State<T> ( const State<T> &other ) : MatrixArray<T> (
		    other ) { }

	/* assignment op */
	State<T> &operator= ( const State<T> &other ) {

		MatrixArray<T>::operator= ( other );
		return *this;

	}
};

#endif /*__STATE_H__*/
