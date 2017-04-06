/*
* @Author: kmrocki
* @Date:   2016-02-24 10:20:09
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-04-03 22:01:05
*/

#ifndef __IMPORTER__
#define __IMPORTER__

#include <deque>
#include <fstream>

//set Matrix implementation
#include <Eigen/Dense>
typedef Eigen::VectorXf Vector;

typedef struct {

	Vector x; 	//inputs
	int y; 		//label

} datapoint;

const size_t C10_LABEL_BYTES = 1;
const size_t C100_LABEL_BYTES = 2;

class CIFARImporter {

  public:

	static std::deque<datapoint> importFromFiles(std::initializer_list<std::string > files, size_t label_bytes = C10_LABEL_BYTES) {

		const size_t offset_bytes = 0;
		const size_t w = 32;
		const size_t h = 32;

		std::deque<datapoint> out;

		char buffer[3074];

		size_t allocs = 0;

		for ( std::string filename : files ) {

			std::ifstream infile(filename.c_str(), std::ios::in | std::ios::binary);

			if (infile.is_open()) {

				printf("Loading data from %s", filename.c_str());
				fflush(stdout);

				infile.seekg (offset_bytes, std::ios::beg);

				while (!infile.eof()) {

					infile.read(buffer, 3072 + label_bytes);

					if (!infile.eof()) {

						unsigned int label = (uint8_t) buffer[0];
						Vector temp(w * h * 3);

						allocs++;

						if (allocs % 1000 == 0) {
							putchar('.');
							fflush(stdout);
						}

						for (unsigned i = 0; i < 1024; i++) {

							//grayscale for now
							temp(3 * i) = (double)((uint8_t)buffer[i + label_bytes]) / 255.0f;
							temp(3 * i + 1) = (double)((uint8_t)buffer[i + 1024 + label_bytes]) / 255.0f;
							temp(3 * i + 2) = (double)((uint8_t)buffer[i + 2048 + label_bytes]) / 255.0f;

						}

						datapoint dp;
						dp.x = temp;
						dp.y = (int)label;
						assert(dp.y >= 0 && dp.y < 10);
						out.push_back(dp);

					}

				}

				printf("Finished.\n");
				infile.close();
			}

			else {

				printf("Oops! Couldn't find file %s\n", filename.c_str());

			}
		}

		return out;

	}

};

class MNISTImporter {

  public:

	static std::deque<datapoint> importFromFile(const char* filename, const char* labels_filename) {

		const size_t offset_bytes = 16;
		const size_t offset_bytes_lab = 8;
		const size_t w = 28;
		const size_t h = 28;

		std::deque<datapoint> out;

		char buffer[w * h];
		char buffer_lab;

		size_t allocs = 0;

		std::ifstream infile(filename, std::ios::in | std::ios::binary);
		std::ifstream labels_file(labels_filename, std::ios::in | std::ios::binary);

		if (infile.is_open() && labels_file.is_open()) {

			printf("Loading data from %s", filename);
			fflush(stdout);

			infile.seekg (offset_bytes, std::ios::beg);
			labels_file.seekg (offset_bytes_lab, std::ios::beg);

			while (!infile.eof() && !labels_file.eof()) {

				infile.read(buffer, w * h);
				labels_file.read(&buffer_lab, 1);

				if (!infile.eof() && !labels_file.eof()) {

					Vector temp(w * h);

					allocs++;

					if (allocs % 1000 == 0) {
						putchar('.');
						fflush(stdout);
					}

					for (unsigned i = 0; i < w * h; i++) {

						temp(i) = (double)((uint8_t)buffer[i]) / 255.0f;

					}

					datapoint dp;
					dp.x = temp;
					dp.y = (unsigned int)buffer_lab;
					out.push_back(dp);

				}

			}

			printf("Finished.\n");
			infile.close();
			labels_file.close();

		} else {

			printf("Oops! Couldn't find file %s or %s\n", filename, labels_filename);
		}

		return out;

	}

};

#endif

