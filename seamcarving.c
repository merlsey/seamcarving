#include "seamcarving.h"
#include <math.h>

void calc_energy(struct rgb_img *im, struct rgb_img **grad) {
    create_img(grad, im->height, im->width);
    for(int x = 0; x < im->width; x++) {
        for(int y = 0; y < im->height; y++) { 
            int x1 = x;
            int x2 = x;
            int y1 = y;
            int y2 = y;
            if(x-1 < 0) x2 = im->width;
            if(x+1 >= im->width) x1 = 0 - 1;
            if(y-1 < 0) y2 = im->height;
            if(y+1 >= im->height) y1 = 0 - 1;
    
            int R_x = get_pixel(im, y, x1+1, 0) - get_pixel(im, y, x2-1, 0);
            int G_x = get_pixel(im, y, x1+1, 1) - get_pixel(im, y, x2-1, 1);
            int B_x = get_pixel(im, y, x1+1, 2) - get_pixel(im, y, x2-1, 2);
            int R_y = get_pixel(im, y1+1, x, 0) - get_pixel(im, y2-1, x, 0);
            int G_y = get_pixel(im, y1+1, x, 1) - get_pixel(im, y2-1, x, 1);
            int B_y = get_pixel(im, y1+1, x, 2) - get_pixel(im, y2-1, x, 2);
            
            double delta_x = pow(R_x, 2) + pow(G_x, 2) + pow(B_x, 2);
            double delta_y = pow(R_y, 2) + pow(G_y, 2) + pow(B_y, 2);

            double energy = sqrt(delta_x + delta_y);
            uint8_t energy1 = (uint8_t)(energy / 10);

            set_pixel(*grad, y, x, energy1, energy1, energy1);            
        }
    }           
}

void dynamic_seam(struct rgb_img *grad, double **best_arr) {
    (*best_arr) = (double *)malloc((grad->width*grad->height)*sizeof(double));
    for(int row = 0; row < grad->height; row++) {
        for(int col = 0; col < grad->width; col++) {
            (*best_arr)[row*grad->width+col] = (double)get_pixel(grad, row, col, 0);
        }
    }
    
    for (int row = 1; row < grad->height; row++){
        for (int col = 0; col < grad->width; col++){
            if(col == 0) {
                (*best_arr)[row*grad->width+col] = fmin((*best_arr)[(row-1)*grad->width+col], (*best_arr)[(row-1)*grad->width+(col+1)]) + get_pixel(grad, row, col, 0);
            } else if(col == grad->width - 1) {
                (*best_arr)[row*grad->width+col] = fmin((*best_arr)[(row-1)*grad->width+(col-1)], (*best_arr)[(row-1)*grad->width+col]) + get_pixel(grad, row, col, 0);
            } else {
                double mini = fmin((*best_arr)[(row-1)*grad->width+col], (*best_arr)[(row-1)*grad->width+(col+1)]);
                (*best_arr)[row*grad->width+col] = fmin((*best_arr)[(row-1)*grad->width+(col-1)], mini) + get_pixel(grad, row, col, 0);
            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    (*path) = (int *)malloc(sizeof(int)*height);
     for(int row = 0; row < height; row++) {
        double cur_min = best[row*width];
        int cur_min_ind = 0;
        for(int col = 1; col < width; col++) {
            if(best[row*width+col] < cur_min) {
                cur_min = best[row*width+col];
                cur_min_ind = col;
            }
        }
        (*path)[row] = cur_min_ind;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    create_img(dest, src->height, src->width - 1);
    for(int row = 0; row < src->height; row++) {
        int path_break = 0;
        for(int col = 0; col < src->width; col++) {
            if(col == path[row]) {
                path_break = 1;
                continue;
            }

            if(path_break == 0) {
                uint8_t pixel[3];
                for(int i = 0; i < 3; i++) {
                    pixel[i] = get_pixel(src, row, col, i); 
                }
                set_pixel((*dest), row, col, pixel[0], pixel[1], pixel[2]);
            } else {
                uint8_t pixel[3];
                for(int i = 0; i < 3; i++) {
                    pixel[i] = get_pixel(src, row, col, i); 
                }
                set_pixel((*dest), row, col-1, pixel[0], pixel[1], pixel[2]);
            }
        }
    }
}

int main1() {
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 5; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
    return 0;
}
