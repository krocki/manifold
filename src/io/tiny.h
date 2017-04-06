std::vector<image> importTinyImages(const char* filename, image_type t, size_t num) {

	std::ifstream infile(filename, std::ios::in | std::ios::binary);
	std::vector<image> out;

	out.reserve(num);

	char buffer[TINY_BYTES_PER_IMAGE];
	unsigned long allocs = 0L;

	unsigned long total_images = 79302017L;

	size_t batchsize = num;

	if (infile.is_open()) {

		tracelog("Importing %zu tiny images", num);

		unsigned long image_no;

		while (allocs < num) {

			image temp;

			if (allocs % batchsize == 0) {
				image_no = rand_int() % (total_images - batchsize);
				printf(" (starting image no: %zu)\n", image_no);
				infile.seekg (image_no * TINY_BYTES_PER_IMAGE, std::ios::beg);

			} else {

				//CHECK(allocs % batchsize == 0);
			}

			infile.read(buffer, TINY_BYTES_PER_IMAGE);

			alloc_image(&temp, t, TINY_SIZE_X, TINY_SIZE_Y);
			allocs++;

			if (allocs % 1000 == 0) {
				putchar('.');
				fflush(stdout);
			}

			if (is_image_allocated(&temp)) {

				for (unsigned i = 0; i < TINY_BYTES_PER_CHANNEL; i++) {

					unsigned row = i / TINY_SIZE_X;
					unsigned col = i % TINY_SIZE_X;

					if (temp.type == GL_RGB_F || temp.type == RGB_F) {

						set_color_float(&temp, RED, 	col, row, (float)(uint8_t) buffer[i]								/ 255.0f);
						set_color_float(&temp, GREEN, 	col, row, (float)(uint8_t) buffer[i + 	TINY_BYTES_PER_CHANNEL]	 	/ 255.0f);
						set_color_float(&temp, BLUE, 	col, row, (float)(uint8_t) buffer[i + 	TINY_BYTES_PER_CHANNEL * 2] / 255.0f);


					} else if (temp.type == GRAYSCALE_F) {

						float value = (float)(uint8_t)buffer[i] + (float)(uint8_t) buffer[i + TINY_BYTES_PER_CHANNEL] + (float)(uint8_t) buffer[i + TINY_BYTES_PER_CHANNEL * 2];
						value /= (255.0f * 3.0f);
						set_color_float(&temp, GRAY, col, row, value);

					} else {

						CHECK(temp.type == GL_RGB_F || temp.type == RGB_F || temp.type == GRAYSCALE_F);

					}

				}

				out.push_back(temp);

			} else {

				CHECK(is_image_allocated(&temp));

			}

		}

		printf("Finished.\n");
		infile.close();

	} else {

		tracelog("Oops! Couldn't find file %s\n", filename);
	}

	return out;
}

std::vector<image> importTinyImagesRealloc(const char* filename, image_type t, std::vector<image> out) {

	std::ifstream infile(filename, std::ios::in | std::ios::binary);

	char buffer[TINY_BYTES_PER_IMAGE];
	unsigned long allocs = 0L;

	unsigned long total_images = 79302017L;

	size_t batchsize = out.size();

	if (infile.is_open()) {

		tracelog("Importing %zu tiny images", out.size());

		unsigned long image_no;

		while (allocs < out.size()) {

			//image temp;

			if (allocs % batchsize == 0) {
				image_no = rand_int() % total_images - batchsize;
				printf(" (starting image no: %zu)\n", image_no);
				infile.seekg (image_no * TINY_BYTES_PER_IMAGE, std::ios::beg);

			} else {

				//CHECK(allocs % batchsize == 0);
			}

			infile.read(buffer, TINY_BYTES_PER_IMAGE);

			//alloc_image(&temp, t, TINY_SIZE_X, TINY_SIZE_Y);
			allocs++;

			if (allocs % 1000 == 0) {
				putchar('.');
				fflush(stdout);
			}

			//if (is_image_allocated(&temp)) {

			for (unsigned i = 0; i < TINY_BYTES_PER_CHANNEL; i++) {

				unsigned row = i / TINY_SIZE_X;
				unsigned col = i % TINY_SIZE_X;

				if (out[allocs].type == GL_RGB_F || out[allocs].type == RGB_F) {

					set_color_float(&out[allocs], RED, 	col, row, (float)(uint8_t) buffer[i]								/ 255.0f);
					set_color_float(&out[allocs], GREEN, 	col, row, (float)(uint8_t) buffer[i + 	TINY_BYTES_PER_CHANNEL]	 	/ 255.0f);
					set_color_float(&out[allocs], BLUE, 	col, row, (float)(uint8_t) buffer[i + 	TINY_BYTES_PER_CHANNEL * 2] / 255.0f);


				} else if (out[allocs].type == GRAYSCALE_F) {

					float value = (float)(uint8_t)buffer[i] + (float)(uint8_t) buffer[i + TINY_BYTES_PER_CHANNEL] + (float)(uint8_t) buffer[i + TINY_BYTES_PER_CHANNEL * 2];
					value /= (255.0f * 3.0f);
					set_color_float(&out[allocs], GRAY, col, row, value);

				} else {

//						CHECK(out[allocs].type == GL_RGB_F || out[allocs].type == RGB_F || out[allocs].type == GRAYSCALE_F);

				}

			}

			//out[allocs] = temp;

			// } else {

			// 	CHECK(is_image_allocated(&temp));

			// }

		}

		printf("Finished.\n");
		infile.close();

	} else {

		tracelog("Oops! Couldn't find file %s\n", filename);
	}

	return out;

}