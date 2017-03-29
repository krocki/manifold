/*
* @Author: kmrocki
* @Date:   2016-02-24 10:20:09
* @Last Modified by:   Kamil M Rocki
* @Last Modified time: 2017-03-27 21:09:51
*/

#ifndef __CIFAR_IMPORTER__
#define __CIFAR_IMPORTER__

#include <deque>
#include <fstream>

//set Matrix implementation
#include <Eigen/Dense>
typedef Eigen::VectorXf Vector;

typedef struct {

	Vector x; 	//inputs
	int y; 		//label

} datapoint;

//TODO:
class C10Importer {

public:

	static std::deque<datapoint> importFromFile(const char* filename) {

		const size_t offset_bytes = 0;
		const size_t w = 32;
		const size_t h = 32;

		std::deque<datapoint> out;

		char buffer[3073];

		size_t allocs = 0;

		std::ifstream infile(filename, std::ios::in | std::ios::binary);

		if (infile.is_open()) {

			printf("Loading data from %s", filename);
			fflush(stdout);

			infile.seekg (offset_bytes, std::ios::beg);

			while (!infile.eof()) {

				infile.read(buffer, 3073);

				if (!infile.eof()) {

					unsigned label = (unsigned) buffer[0];

					Vector temp(w * h);

					allocs++;

					if (allocs % 1000 == 0) {
						putchar('.');
						fflush(stdout);
					}

					for (unsigned i = 0; i < 1024; i++) {

						//grayscale for now
						temp(i) = ((double)((uint8_t)buffer[i + 1]) + (double)((uint8_t)buffer[i + 1 + 1024]) + (double)((uint8_t)buffer[i + 1 + 2048])) / (3 * 255.0f);

					}

					datapoint dp;
					dp.x = temp;
					dp.y = (int)label;
					out.push_back(dp);

				}

			}

			printf("Finished.\n");
			infile.close();

		} else {

			printf("Oops! Couldn't find file %s\n", filename);

		}

		return out;

	}

};

#endif

