#define C_SIZE_X (1 << split_x)
#define C_SIZE_Y (1 << split_y)

/* GLOBAL VARIABLES */
size_t tr_size;
vec_x *tr_x;
vec_y *tr_y;

c_book_x *c_x;
c_book_y *c_y;

int split_x = 0;
int split_y = 0;

int *enc_x;
int *enc_y;

q_vec *q_tr;

void print_training_set() {
    
}

void print_codebook() {
    
}

void print_quant_levels() {
    
}

int init(covq *everything) {
    // unpack everything
    // mallocs everything
    // quantize
}

/* return number of vectors outside quantize region */
int quantize() {

}

/* source should be "X" or "Y" */
double trans_prob(int i, int j, int src) {

}

double dist(int q_lvl, int index, int src) {

}

/* return *index by reference */
double nearest_neighbour(int q_lvl, int *index, int src) {
    // calls dist, trans_prob
}

double centroid_update(int src) {
    // calls nearest_neighbour, trans_prob
}

void split(int src) {

}

int bsc_2_source_covq(covq *everything) {
    init(everything);
    // centroid_update
    // split

    // repack
    return 0; // yay
}