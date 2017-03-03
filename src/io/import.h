/*
* @Author: kmrocki
* @Date:   2016-02-24 10:20:09
* @Last Modified by:   kmrocki@us.ibm.com
* @Last Modified time: 2017-03-03 15:07:39
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

