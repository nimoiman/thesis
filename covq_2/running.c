#include "covq_2.h"

int init(struct covq *c) {
    // unpack everything
    tr_size = c->tr_size;
    tr_x = c->tr_x;
    tr_y = c->tr_y;

    q_tr = c->q_tr;

    c_x = c->c_x;
    c_y = c->c_y;

    split_x = c->split_x;
    split_y = c->split_y;

    enc_x = c->enc_x;
    enc_y = c->enc_y;

    sigma_x = c->sigma_x;
    sigma_y = c->sigma_y;
    mean_x = c->mean_x;
    mean_y = c->mean_y;

    enc_x = malloc(sizeof(int) * Q_LEVELS_X);
    enc_y = malloc(sizeof(int) * Q_LEVELS_Y);

    // quantize
    quantize();
}