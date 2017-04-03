/*
* @Author: kmrocki@us.ibm.com
* @Date:   2017-04-01 19:33:21
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-01 20:46:48
*
*	m = Dict(

		{	"something"	, {

			std::make_tuple("W", M, N),
			std::make_tuple("U", X, Y),

		}
	)

	will make a 2 sub-matrices "W" and "U" of dimensions M x N and X x Y
	respectively;

	later, m['W'] will return the first matrix and m['U'] the second one
	other things are just implementations of operators and IO
*/

#ifndef __DICT_H__
#define __DICT_H__

#include <map>
#include <vector>
#include <string>

template <typename T>
class Dict {

  public:

	std::vector<T> matrices;
	std::string name;
	std::map<std::string, size_t> namemap;

	Dict<T>() = default;

	/* the main constructor */
	Dict<T> ( std::string _name,
	          std::initializer_list<std::tuple<std::string, size_t, size_t>>
	          args, std::string id ) : name ( _name + " " + id ) {

		add ( args );

	}

	void add (
	    std::initializer_list<std::tuple<std::string, size_t, size_t>>
	    args ) {

		for ( auto i : args ) {

			namemap[std::get<0> ( i )] = matrices.size();
			matrices.push_back ( T ( std::get<1> ( i ),
			                         std::get<2> ( i ) ) );

			matrices.back().setZero();

		}

	}

	Dict<T> ( const Dict<T> &other ) {

		namemap = other.namemap;
		name = other.name;
		matrices = other.matrices;

	}

	Dict<T> &operator= ( const Dict<T> &other ) {

		namemap = other.namemap;
		name = other.name;
		matrices = other.matrices;

		return *this;

	}

	template <typename otherType>
	Dict<T> &operator= ( const Dict<otherType> &other ) {

		namemap = other.namemap;
		name = other.name;

		for ( size_t i = 0; i < matrices.size(); i++ )
			matrices[i] = other.matrices[i];

		return *this;

	}

	T &operator[] ( char key ) {

		return ( *this ) [std::string ( 1, key )];

	}

	T &operator[] ( std::string key ) {

		if ( namemap.find ( key ) == namemap.end() ) {

			namemap[key] = matrices.size();
			matrices.push_back(T());

		}

		return matrices[namemap[key]];

	}

	T* ptr ( std::string key ) {

		if ( namemap.find ( key ) == namemap.end() ) {

			namemap[key] = matrices.size();
			matrices.push_back(T());

		}

		return &(matrices[namemap[key]]);

	}

	void zero() {

		for ( size_t i = 0; i < matrices.size(); i++ )
			matrices[i].setZero();

	}

	template<class Archive>
	void serialize ( Archive &archive ) {

		archive ( name );
		archive ( namemap );
		archive ( matrices );

	}

};

#endif /*__Dict_H__*/
