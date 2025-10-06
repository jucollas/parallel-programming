#include <bits/stdc++.h>
#include <mpi.h>
#include "random_mat.h"
#include "operations-matrix.h"

using namespace std;
using Row = vector<int>;
using M = vector<Row>;

vector<int> flatten(const M &mat) {
    vector<int> data;
    for (auto &row : mat)
        for (int v : row) data.push_back(v);
    return data;
}

M unflatten(const vector<int> &data, int n, int m) {
    M mat(n, Row(m));
    int idx = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            mat[i][j] = data[idx++];
    return mat;
}


void distribute_work(int n, int m, int p, int q, int rank, int size) {
    M A, B;

    if (rank == 0) {
        A = create_rand_matrix(n, m);
        B = create_rand_matrix(p, q);

        cout << "A:\n"; show_mat(A);
        cout << "B:\n"; show_mat(B);

        vector<int> dataA = flatten(A);
        vector<int> dataB = flatten(B);
        int sizes[4] = {n, m, p, q};

        // Envía matrices a los trabajadores
        for (int dest = 1; dest < min(size, 4); ++dest) {
            MPI_Send(sizes, 4, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(dataA.data(), n*m, MPI_INT, dest, 1, MPI_COMM_WORLD);
            MPI_Send(dataB.data(), p*q, MPI_INT, dest, 2, MPI_COMM_WORLD);
        }
    }

    // ====== Procesos trabajadores ======
    if (rank > 0 && rank < 4) {
        int sizes[4];
        MPI_Recv(sizes, 4, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int n = sizes[0], m = sizes[1], p = sizes[2], q = sizes[3];

        vector<int> dataA(n*m), dataB(p*q);
        MPI_Recv(dataA.data(), n*m, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(dataB.data(), p*q, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        M A = unflatten(dataA, n, m);
        M B = unflatten(dataB, p, q);
        M result;

        if (rank == 1) result = addition(A, B);
        else if (rank == 2) result = transpose(A);
        else if (rank == 3) result = multiplication(A, B);

        vector<int> flatRes = flatten(result);
        int rn = result.size();
        int rm = result[0].size();
        int info[2] = {rn, rm};
        MPI_Send(info, 2, MPI_INT, 0, 3, MPI_COMM_WORLD);
        MPI_Send(flatRes.data(), rn * rm, MPI_INT, 0, 4, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        for (int src = 1; src < min(size, 4); ++src) {
            int info[2];
            MPI_Recv(info, 2, MPI_INT, src, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector<int> recvData(info[0]*info[1]);
            MPI_Recv(recvData.data(), info[0]*info[1], MPI_INT, src, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            M result = unflatten(recvData, info[0], info[1]);

            if (src == 1) cout << "\nA + B:\n";
            if (src == 2) cout << "\nTranspuesta de A:\n";
            if (src == 3) cout << "\nA x B:\n";
            show_mat(result);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 5) {
        if (rank == 0)
            cout << "Uso: <n> <m> <p> <q> (A: n×m, B: p×q)\n";
        MPI_Finalize();
        return 0;
    }

    int n = stoi(argv[1]);
    int m = stoi(argv[2]);
    int p = stoi(argv[3]);
    int q = stoi(argv[4]);

    if (m != p) {
        if (rank == 0)
            cerr << "Error: m y p deben ser iguales para la multiplicación.\n";
        MPI_Finalize();
        return 1;
    }

    distribute_work(n, m, p, q, rank, size);

    MPI_Finalize();
    return 0;
}
