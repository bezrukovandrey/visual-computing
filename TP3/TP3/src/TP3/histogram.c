
#include <stdio.h>
#include <stdlib.h>

#include "../Utils/types.h"

#define round(x) {(int)(x + 0.5)}

histogram_t compute_histogram(pgm_file image) {

    histogram_t histogram;
    histogram.size = image.maxval + 1;
    histogram.histogram = malloc(histogram.size * sizeof(int));

    for (int intensity = 0; intensity < histogram.size; intensity++) {
      histogram.histogram[intensity] = 0;
    }
    
    for (int i = 0; i < image.cols * image.rows; i++) {
        int intensity = image.graymap[i];
        (intensity >= 0 && intensity <= image.maxval) ? histogram.histogram[intensity]++ : 0;
      }

    return histogram;

}


pgm_file histogram_stretching(pgm_file image) {

  histogram_t histogram = compute_histogram(image);

  int min_intensity = 0; 
  int max_intensity = image.maxval;
  int stretching[histogram.size];
  

    for (int intensity = 0; intensity < histogram.size; intensity++) {
      if (histogram.histogram[intensity] != 0) {
        min_intensity = intensity;
        break;
      }
    }

    for (int intensity = histogram.size - 1; intensity >= 0; intensity--) {
      if (histogram.histogram[intensity] != 0) {
        max_intensity = intensity;
        break;
      }
    }

    for (int intensity = 0; intensity < histogram.size; intensity++) {
      stretching[intensity] = image.maxval * (intensity - min_intensity) / (max_intensity - min_intensity);
    }

    pgm_file output_image;
    output_image.cols = image.cols;
    output_image.rows = image.rows;
    output_image.magic_number = image.magic_number;
    output_image.maxval = image.maxval;
    output_image.graymap = malloc(image.cols * image.rows * sizeof(gray));

    for (int i = 0; i < image.cols; i++) {
        for (int j = 0; j < image.rows; j++) {
          output_image.graymap[j * image.cols + i] = stretching[image.graymap[j * image.cols + i]];
        }
      }

    return output_image;
}

pgm_file histogram_equalization(pgm_file image) {
    
    histogram_t histogram = compute_histogram(image);
    double cumulative_distribution[histogram.size];

    for (int intensity = 0; intensity < histogram.size; intensity++) {
      cumulative_distribution[intensity] = 0.0;
    }

    cumulative_distribution[0] = (double) histogram.histogram[0] / (image.cols * image.rows);

    for (int intensity = 1; intensity < histogram.size; intensity++) {
      double equalization = (double) histogram.histogram[intensity] / (image.cols * image.rows);
      cumulative_distribution[intensity] = cumulative_distribution[intensity - 1] + equalization;
    }

    pgm_file output_image;
    output_image.cols = image.cols;
    output_image.rows = image.rows;
    output_image.magic_number = image.magic_number;
    output_image.maxval = image.maxval;
    output_image.graymap = malloc(image.cols * image.rows * sizeof(gray));

    for (int i = 0; i < image.cols; i++) {
        for (int j = 0; j < image.rows; j++) {
          output_image.graymap[j * image.cols + i] = (gray) round(cumulative_distribution[image.graymap[j * image.cols + i]] * image.maxval);
        }
      }

    return output_image;
}


// Stores a histogram into a csv file format
void save_histogram(histogram_t histogram, char *path) {

  FILE *fp;
  fp = fopen(path, "w");

  if (fp == NULL) {
    printf("error in opening file %s\n", path);
    exit(1);
  }

  // first row with headers
  fprintf(fp, "Intensity, Count\n");

  // print the histogram values
  for (int i = 0; i < histogram.size; i++) {
    fprintf(fp, "%d,%d\n", i, histogram.histogram[i]);
  }

  fclose(fp);
}