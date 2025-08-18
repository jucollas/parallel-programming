#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include <cmath>


using namespace std;
#define rep(i,strt,end) for(int i=strt;i<int(end);++i)

std::mt19937 mrandom(std::chrono::steady_clock::now().time_since_epoch().count());

class point {
public:
    double x, y;
    point() = default;
    double operator*(const point &o) const { return x * o.x + y * o.y; }
};

int montepi(int iter) {
    point pt;
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    auto shuffle = [&]() { pt.x = dis(mrandom); pt.y = dis(mrandom); };

    int res = 0;
    rep(i, 0, iter) {
        shuffle();
        if (pt * pt <= 1.0) ++res;
    }
    return res;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <num_iter>\n";
        return 1;
    }
    int iter = stoi(argv[1]);

    // --- medir CPU time ---
    clock_t start_cpu = clock();

    // --- medir Wall time ---
    auto start_wall = chrono::high_resolution_clock::now();

    int p_res = montepi(iter);
    double res = 4.0 * double(p_res) / double(iter);

    clock_t end_cpu = clock();
    auto end_wall = chrono::high_resolution_clock::now();

    double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
    chrono::duration<double> wall_time = end_wall - start_wall;

    cout << "Pi approx: " << res << endl;
    cout << "CPU time: " << cpu_time << " seconds" << endl;
    cout << "Wall time: " << wall_time.count() << " seconds" << endl;
    cout << "Error abs: " << abs(M_PI - res) << endl;


    return 0;
}
