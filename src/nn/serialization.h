/*
 *
 * 	@Author: kmrocki
 * 	@Date:   2016-04-05
 * 	@Last Modified by:   kmrocki
 * 	@Last Modified time: 2016-04-05 18:36:57
 *
 *	stuff for serialization
 */

#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__

#ifdef __USE_CEREAL__

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>

#endif

#endif /* __SERIALIZATION_H__ */