#include <math.h>
#include <stdlib.h>

#include "filters.h"
#include "../Utils/Util.h"

int binomial_filter_3[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
int binomial_filter_5[25] = {1, 4, 6, 4, 1, 4, 16, 24, 16, 4, 6, 24, 36, 24, 6, 4, 16, 24, 16, 4, 1, 4, 6, 4, 1};

float get_array_sum(int* array, int n) {
    int sum = 0;
    for(int i = 0; i < n; i++) {
        sum += array[i];
    }
    return sum;
}

int compare(const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

pgm_file binomial_filter_single(pgm_file image, int filter_size) {
    pgm_file output_image;
    output_image.cols = image.cols;
    output_image.rows = image.rows;
    output_image.magic_number = image.magic_number;
    output_image.maxval = image.maxval;
    output_image.graymap = malloc(image.cols * image.rows * sizeof(gray));

    int* binomial_filter;
    switch (filter_size) {
        case 3: binomial_filter = binomial_filter_3; break;
        case 5 : binomial_filter = binomial_filter_5; break;
        default: perror("This filter size is not supported"); exit(1);
    }

    float filter_sum = get_array_sum(binomial_filter, filter_size * filter_size);
    int window_min = - floor(filter_size / 2);
    int window_max = floor(filter_size / 2);

    for (int i = 0; i < image.cols; i++) {
        for (int j = 0; j < image.rows; j++) {
            int window_avg = 0;
            for (int k = window_min; k <= window_max; k++) {
                for (int p = window_min; p <= window_max; p++) {
                    int idx_x = i + k;
                    int idx_y = j + p;

                    if (idx_x < 0) {
                        idx_x = 0;
                    } else if (idx_x >= image.cols) {
                        idx_x = image.cols -1;
                    }

                    if (idx_y < 0) {
                        idx_y = 0;
                    } else if (idx_y >= image.rows) {
                        idx_y = image.rows - 1;
                    }
                    
                    int filter_idx = (p + 1) * filter_size + (k + 1);
                
                    window_avg += image.graymap[idx_y * image.cols + idx_x] * binomial_filter[filter_idx];

                }
            }
            window_avg /= filter_sum;
            output_image.graymap[j * image.cols + i] = window_avg;
        }
    }


    return output_image;
}

// apply binomial filter to image
pgm_file binomial_filter(pgm_file image, int filter_size, int iterations) {
    pgm_file curr_input = image;
    for (int i = 0; i < iterations; i++) {
        curr_input = binomial_filter_single(curr_input, filter_size);
    }
    return curr_input;
}

pgm_file median_filter_single(pgm_file image, int filter_size) {
    pgm_file output_image;
    output_image.cols = image.cols;
    output_image.rows = image.rows;
    output_image.magic_number = image.magic_number;
    output_image.maxval = image.maxval;
    output_image.graymap = malloc(image.cols * image.rows * sizeof(gray));

    if ((filter_size != 3) && (filter_size != 5)) {
        perror("This filter size is not supported");
        exit(1);
    }
    
    int window_min = - floor(filter_size / 2);
    int window_max = floor(filter_size / 2);

    for (int i = 0; i < image.cols; i++) {
        for (int j = 0; j < image.rows; j++) {
            int *window_vals = malloc(filter_size * filter_size * sizeof(int));
            int cnt = 0;
            for (int k = window_min; k <= window_max; k++) {
                for (int p = window_min; p <= window_max; p++) {
                    int idx_x = i + k;
                    int idx_y = j + p;

                    if (idx_x < 0) {
                        idx_x = 0;
                    } else if (idx_x >= image.cols) {
                        idx_x = image.cols -1;
                    }

                    if (idx_y < 0) {
                        idx_y = 0;
                    } else if (idx_y >= image.rows) {
                        idx_y = image.rows - 1;
                    }
                    window_vals[cnt] = image.graymap[idx_y * image.cols + idx_x];
                    cnt++;
                    

                    
                }
            }
            qsort(window_vals, cnt, sizeof(int), compare);
            int median = window_vals[cnt / 2];
            output_image.graymap[j * image.cols + i] = median;
        }
    }

    return output_image;
}


// apply median filter to image
pgm_file median_filter(pgm_file image, int filter_size, int iterations) {
    pgm_file curr_input = image;

    for (int i = 0; i < iterations; i++) {
        curr_input = median_filter_single(curr_input, filter_size);
    }
    return curr_input;
}