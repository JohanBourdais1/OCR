#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define HIDDEN_SIZE 128
#define MLP_SIZE 400
#define OUTPUT_SIZE 10

// Forward pass: y[i] = sum_j(W[i,j] * x[j]) + b[i]
// Backward: dL/dW[i,j] = dL/dy[i] * x[j]
//           dL/dx[j] = sum_i(W[i,j] * dL/dy[i])

void dense_backward_2(double W[][HIDDEN_SIZE], double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M){
    // N = HIDDEN_SIZE (num inputs)
    // M = OUTPUT_SIZE (num outputs)
    for(size_t i=0;i<M;i++){
        db[i] += dout[i];
        for(size_t j=0;j<N;j++){
            dW[i*N + j] += dout[i] * x[j];
        }
    }
    for(size_t j=0;j<N;j++){
        double s = 0.0f;
        for(size_t i=0;i<M;i++) s += W[i][j] * dout[i];
        dx[j] += s;
    }
}

void test_dense_backward_2() {
    printf("=== TEST: dense_backward_2 ===\n");
    
    // Petit test: 3 entrées, 2 sorties
    #define TEST_N 3
    #define TEST_M 2
    
    // Poids: W[M][N] = W[2][3]
    double W[TEST_M][TEST_N] = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    // Input: x[N] = x[3]
    double x[TEST_N] = {1.0, 2.0, 3.0};
    
    // Forward: y = W @ x + b
    // y[0] = 1*1 + 2*2 + 3*3 = 14
    // y[1] = 4*1 + 5*2 + 6*3 = 32
    
    // Gradient wrt output: dL/dy[i]
    double dout[TEST_M] = {1.0, 2.0};  // Dummy gradients
    
    // Allocate gradients
    double dW[TEST_M * TEST_N];
    double db[TEST_M];
    double dx[TEST_N];
    memset(dW, 0, TEST_M * TEST_N * sizeof(double));
    memset(db, 0, TEST_M * sizeof(double));
    memset(dx, 0, TEST_N * sizeof(double));
    
    // Call backward with correct dimensions
    dense_backward_2((double (*)[TEST_N])W, x, dout, dW, db, dx, TEST_N, TEST_M);
    
    printf("Input x: [%g, %g, %g]\n", x[0], x[1], x[2]);
    printf("dL/dy: [%g, %g]\n", dout[0], dout[1]);
    
    // Expected gradients:
    // dW[0,0] = dout[0] * x[0] = 1 * 1 = 1
    // dW[0,1] = dout[0] * x[1] = 1 * 2 = 2
    // dW[0,2] = dout[0] * x[2] = 1 * 3 = 3
    // dW[1,0] = dout[1] * x[0] = 2 * 1 = 2
    // dW[1,1] = dout[1] * x[1] = 2 * 2 = 4
    // dW[1,2] = dout[1] * x[2] = 2 * 3 = 6
    
    printf("\nComputed dW:\n");
    for (int i = 0; i < TEST_M; i++) {
        for (int j = 0; j < TEST_N; j++) {
            printf("  dW[%d,%d] = %g (expected %g) %s\n", i, j, dW[i*TEST_N + j], 
                   dout[i] * x[j], (fabs(dW[i*TEST_N + j] - dout[i] * x[j]) < 1e-10) ? "✓" : "✗");
        }
    }
    
    // Expected db:
    // db[0] = dout[0] = 1
    // db[1] = dout[1] = 2
    
    printf("\nComputed db:\n");
    for (int i = 0; i < TEST_M; i++) {
        printf("  db[%d] = %g (expected %g) %s\n", i, db[i], dout[i], 
               (fabs(db[i] - dout[i]) < 1e-10) ? "✓" : "✗");
    }
    
    // Expected dx (gradient wrt input):
    // dx[0] = W[0,0]*dout[0] + W[1,0]*dout[1] = 1*1 + 4*2 = 9
    // dx[1] = W[0,1]*dout[0] + W[1,1]*dout[1] = 2*1 + 5*2 = 12
    // dx[2] = W[0,2]*dout[0] + W[1,2]*dout[1] = 3*1 + 6*2 = 15
    
    printf("\nComputed dx:\n");
    for (int j = 0; j < TEST_N; j++) {
        double expected = 0;
        for (int i = 0; i < TEST_M; i++) expected += W[i][j] * dout[i];
        printf("  dx[%d] = %g (expected %g) %s\n", j, dx[j], expected, 
               (fabs(dx[j] - expected) < 1e-10) ? "✓" : "✗");
    }
    
    printf("\n");
}

int main() {
    test_dense_backward_2();
    return 0;
}
