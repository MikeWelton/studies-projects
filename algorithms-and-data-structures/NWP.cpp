#include <iostream>
#include <vector>
#include <climits>

using namespace std;

int n, m, c;

void read_input(vector<int> &A, vector<int> &B) {
    cin >> n >> m >> c;
    int value;
    for (int i = 0; i < n; ++i) {
        cin >> value;
        A.push_back(value);
    }
    for (int i = 0; i < m; ++i) {
        cin >> value;
        B.push_back(value);
    }
}

int calculate_result(vector<vector<bool>>& DP, vector<int> &A, vector<int> &B) {
    // Wektor zapisujący dolne ograniczenie (dolne ograniczenie, czyli
    // wartość, od której musi być >= element jeśli ma wydłużyć podciąg).
    vector<vector<int>> lower_bound(n, vector<int>(20, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 20; ++j) {
            lower_bound[i][j] = A[i] - c;
        }
    }

    for (int j = 0; j < m; ++j) { //idziemy po B
        // Podczas jednego przejścia po A zapisuje informacje o długości
        // podciągu, jeśli kończył by się na wartości B[j] (o ile ta wartość
        // zostanie znaleziona w A).
        vector<bool> lengths(20, false);
        // Podczas jednego przejścia po A zapisuje informacje, dla każdej (<= 20)
        // długości podciągu kończącego się na wartości B[j], o minimalnej wartości
        // (dolnym ograniczeniu) jakie musi spełniać nastepny element jeśli miałby
        // być dodany do podciągu.
        vector<int> min_values(20, INT_MAX);
        for (int i = 0; i < n; ++i) { //idziemy po A
            if (B[j] == A[i]) {
                if(lengths[18]) {
                    return 20;
                }
                vector<int> temp_bounds = lower_bound[i];
                vector<bool> temp_lengths = DP[i];

                DP[i][0] = true;
                for (int k = 0; k < 19 && lengths[k]; ++k) {
                    lower_bound[i][k + 1] = !DP[i][k + 1] ? min_values[k] : min(min_values[k], lower_bound[i][k + 1]);
                    DP[i][k + 1] = true;
                }

                for(int k = 0; k < 20; ++k) {
                    if(temp_lengths[k]) {
                        lengths[k] = true;
                        min_values[k] = min(min_values[k], temp_bounds[k]);
                    }
                }

            }
            else {
                for(int k = 0; k < 20 && DP[i][k]; ++k) {
                    if(B[j] >= lower_bound[i][k] && DP[i][k]) {
                        lengths[k] = true;
                        min_values[k] = min(min_values[k], max(B[j] - c, lower_bound[i][k]));
                    }
                }
            }
        }
    }

    return 0;
}

int max_value(vector<vector<bool>>& array) {
    for (int j = 18; j >= 0; --j) {
        for (int i = 0; i < n; ++i) {
            if(array[i][j]) {
                return j + 1;
            }
        }
    }
    return 0;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    vector<int> A;
    vector<int> B;

    read_input(A, B);

    // Wektor zapisujący (dla A[i][k]) czy dla ciągu A kończącego się w i
    // istnieje podciąg prawie rosnący o długości k + 1, wspólny z B.
    vector<vector<bool>> DP(n, vector<bool>(20, false));
    if(calculate_result(DP, A, B) == 20) {
        cout << 20 << "\n";
    }
    else {
        cout << max_value(DP) << "\n";

    }

    return 0;
}

/*
9 8 0
2 3 1 4 2 1 3 5 4
1 3 2 1 4 2 5 3
 *
 *
8 8 4
13 12 8 10 9 7 1 3
10 13 9 12 8 7 1 3
*/
