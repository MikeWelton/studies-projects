#include <iostream>
#include <vector>
#include <bits/stdc++.h>

using namespace std;
using vector2d = vector<vector<int>>;
using vector_of_tables = vector<vector2d>;

int calculate_result_brute(int x1, int y1, int x2, int y2, const vector2d &field) {
    int max = 0, min = INT_MAX;
    for(int i = x1; i <= x2; ++i) {
        for(int j = y1; j <= y2; ++j) {
            min = field[i][j] < min ? field[i][j] : min;
            max = field[i][j] > max ? field[i][j] : max;
        }
    }
    return max - min;
}

/* O(max(n,m) log max(n,m))
 * If rows is true we create table for every row, otherwise we create table for every column. */
void preprocessing(const vector2d &field, vector_of_tables &min_tables, vector_of_tables &max_tables, bool rows) {

    /* Init first row with numbers from field table because at index 0 (in every min or max table)
     * we have minimums and maximums for subarrays of length 2^0 = 1 so elements themselves. */
    for(int i = 0; i < min_tables.size(); ++i) { // every row or column
        for(int j = 0; j < min_tables[i][0].size(); ++j) {
            if(rows) {
                min_tables[i][0][j] = max_tables[i][0][j] = field[i][j];
            }
            else {
                min_tables[i][0][j] = max_tables[i][0][j] = field[j][i];
            }
        }
    }

    for(int i = 0; i < min_tables.size(); ++i) {
        for(int exp = 1, tpow = 2; exp < min_tables[i].size(); ++exp, tpow *= 2) { // here's log max(n,m)
            for(int j = 0; j + tpow <= min_tables[i][exp].size(); ++j) {
                min_tables[i][exp][j] = min(min_tables[i][exp - 1][j], min_tables[i][exp - 1][j + tpow / 2]);
                max_tables[i][exp][j] = max(max_tables[i][exp - 1][j], max_tables[i][exp - 1][j + tpow / 2]);
            }
        }
    }
}

int calculate_result(int x1, int y1, int x2, int y2,
                     vector_of_tables &min_tables, vector_of_tables &max_tables, bool rows) {
    int curr_min = INT_MAX, curr_max = 0;
    int start = rows ? x1 : y1;
    int end = rows ? x2 : y2;
    int a = rows ? y1 : x1; // a is first index from range
    int b = rows ? y2 : x2; // b is last index from range
    int exp = floor(log2(b - a + 1)); // smallest exp that 2^exp <= b - a + 1
    int tpow = pow(2, exp);
    for(int i = start; i <= end; ++i) {
        // take min from min_tables in [a...b]
        // take max from max_tables in [a...b]
        int taken_min = min(min_tables[i][exp][a], min_tables[i][exp][b - tpow + 1]);
        int taken_max = max(max_tables[i][exp][a], max_tables[i][exp][b - tpow + 1]);
        // compare with current minimum and maximum
        curr_min = min(curr_min, taken_min);
        curr_max = max(curr_max, taken_max);
    }
    return curr_max - curr_min;
}

void read_input() {
    int n, m, k;

    cin >> n >> m >> k;
    vector<vector<int>> field(n, vector<int>(m));

    for(int i = 0; i < n; ++i) {
        for(int j = 0; j < m; ++j) {
            cin >> field[i][j];
        }
    }

    /* We make min(n,m) tables of max(n,m) * log(max(n,m)) elements each. */
    int mini = min(n, m);
    int maxi = max(n, m);
    int logmax = (int)floor(log2(maxi)) + 1;
    vector_of_tables min_tables(mini, vector2d(logmax, vector<int>(maxi)));
    vector_of_tables max_tables(mini, vector2d(logmax, vector<int>(maxi)));
    preprocessing(field, min_tables, max_tables,n < m);

    int x1, y1, x2, y2;
    for(int i = 0; i < k; ++i) {
        cin >> x1 >> y1 >> x2 >> y2;
        cout << calculate_result(x1, y1, x2, y2, min_tables, max_tables, n < m) << "\n";
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    read_input();
    return 0;
}
