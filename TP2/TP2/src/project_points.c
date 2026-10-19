#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "Utils/imageFormationUtils.h"
#include "Utils/Util.h"


int main(int argc, char *argv[])
{
    if (argc != 16)
    {
        printf("\nUsage: %s off_file_name.off "
                            "f width height u_0 v_0 alpha_u alpha_v "
                            "gama beta alpha T_x T_y T_z "
                            "use_depth\n",
         argv[0]);
        exit(0);
    }

    // Reading the input parameters
    char* off_filename = argv[1];
    // Intrinsics
    float f = atof(argv[2]);

    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    float u_0 = atof(argv[5]);
    float v_0 = atof(argv[6]);

    float alpha_u = atof(argv[7]);
    float alpha_v = atof(argv[8]);

    // Extrinsics
    float gama = atof(argv[9]);
    float beta = atof(argv[10]);
    float alpha = atof(argv[11]);
    float T_x = atof(argv[12]);
    float T_y = atof(argv[13]);
    float T_z = atof(argv[14]);

    // Use depth map
    int use_depth = atoi(argv[15]);

    // The output filename is generated from the input
    char output_filename[512];
    sprintf(output_filename, "results/%c_%.0f_%d_%d_%.1f_%.1f_%.2f_%.2f_%.1f_%.1f_%.1f_%.1f_%.1f_%.1f_%d.ppm",
            off_filename[7], // hack: use first letter of model to create output (f or h in this TP)
            f, width, height, u_0, v_0, alpha_u, alpha_v,
            gama, beta, alpha, T_x, T_y, T_z,
            use_depth);

    // Start processing
    // Read the pointcloud
    struct point3d *points;
    int N_v = 0;
    points = readOff(off_filename, &N_v);

    // and center it
    centerThePCL(points, N_v);

    // allocate output image and initialize with white colors
    ppm_file image;
    image.rows = height;
    image.cols = width;
    image.maxval = 255;
    image.magic_number = '6';

    // Allocate what you think you need
    image.pixmap = (pixel *) malloc(image.cols * image.rows * sizeof(pixel));
    if (!image.pixmap) {
        fprintf(stderr, "Couldn't allocate memory for the image");
        exit(1);
    }

    for (int i = 0; i < image.rows * image.cols; i++) {
        image.pixmap[i].red = image.pixmap[i].green = image.pixmap[i].blue = image.maxval;
    }


    // Print if depth buffer is being used
    if (use_depth) {
        printf("Using Depth Buffer\n");
    }

    int orthogonal = 0;
    if (f==0) {
        printf("Using Orthogonal camera\n");
        orthogonal = 1;
    }


    float T[16];
    computeTrans(gama, beta, alpha, T_x, T_y, T_z, T);

    float *z_buffer = NULL;
    if (use_depth) {
        z_buffer = (float *) malloc(image.rows * image.cols * sizeof(float));
        for (int i = 0; i < image.rows * image.cols; i++) {
            z_buffer[i] = INFINITY;
        }
    }

    for (int i = 0; i < N_v; i++) {
        float X_p = points[i].x;
        float Y_p = points[i].y;
        float Z_p = points[i].z;

        float X_p_cam = T[0]*X_p + T[1]*Y_p + T[2]*Z_p + T[3];
        float Y_p_cam = T[4]*X_p + T[5]*Y_p + T[6]*Z_p + T[7];
        float Z_p_cam = T[8]*X_p + T[9]*Y_p + T[10]*Z_p + T[11];

        float X_cam, Y_cam;
        if (orthogonal) {
            X_cam = X_p_cam;
            Y_cam = Y_p_cam;
        } else {
            float denominator = 1.0 + Z_p_cam/f;
            if (denominator == 0.0) continue;
            X_cam = X_p_cam / denominator;
            Y_cam = Y_p_cam / denominator;
        }

        int u = (int) roundf(X_cam / alpha_u + u_0);
        int v = (int) roundf(Y_cam / alpha_v + v_0); 

        if (u >= 0 && u < width && v >= 0 && v < height){
            if (use_depth) {
                if (Z_p_cam < z_buffer[u + v*width]) {
                    z_buffer[u + v*width] = Z_p_cam;
                    image.pixmap[u + v*width].red   = points[i].r;
                    image.pixmap[u + v*width].green = points[i].g;
                    image.pixmap[u + v*width].blue  = points[i].b;
                }
            } else {
                image.pixmap[u + v*width].red = points[i].r;
                image.pixmap[u + v*width].green = points[i].g;
                image.pixmap[u + v*width].blue = points[i].b;
            }
        }
    }

    (use_depth && z_buffer) ? free(z_buffer) : 0;

    

    // Save the image
    printf("Writting output file %s", output_filename);
    write_ppm(image, output_filename);
    };
