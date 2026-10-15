#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

struct Column 
{
    int id;
    int weight;
    vector<int> rows;
};

struct Answer 
{
    int count = numeric_limits<int>::max();
    int weight = numeric_limits<int>::max();
    vector<int> columns;
};

// 把這個欄位能覆蓋的列加進遮罩
long long cover_rows(long long mask, const Column& column)
{
    for (int row : column.rows) 
    {
        mask |= (1LL << row);
    }
    return mask;
}

// 看剩下的欄位是否還有機會覆蓋全部列
bool can_finish(int start, long long mask, long long target, const vector<Column>& columns)
{
    for (int i = start; i < static_cast<int>(columns.size()); ++i)
    {
        mask = cover_rows(mask, columns[i]);
    }
    return mask == target;
}

void search(int index, int count, int weight, long long covered, long long target, const vector<Column>& columns, vector<int>& chosen, Answer& answer)
{
    if (count > answer.count || (count == answer.count && weight >= answer.weight)) //1.選的columns比最佳解多 2.columns一樣但權重比最佳解大
    {
        return;
    }

    if (covered == target)
    {
        answer.count = count;
        answer.weight = weight;
        answer.columns = chosen;
        return;
    }

    if (index >= static_cast<int>(columns.size()))
    {
        return;
    }

    const Column& column = columns[index];
    long long new_covered = cover_rows(covered, column);

    // 選這個欄位
    if (new_covered != covered)
    {
        chosen.push_back(column.id);
        search(index + 1, count + 1, weight + column.weight, new_covered, target, columns, chosen, answer);
        chosen.pop_back();
    }

    // 不選這個欄位；如果後面已經無法補齊，就不用繼續找
    if (can_finish(index + 1, covered, target, columns))
    {
        search(index + 1, count, weight, covered, target, columns, chosen, answer);
    }
}

int main(int argc, char* argv[])
{
    ifstream fin(argv[1]);
    if (!fin)
    {
        cerr << "無法開啟輸入檔案\n";
        return 1;
    }

    int num_rows, num_columns;
    if (!(fin >> num_rows >> num_columns))
    {
        return 0;
    }

    vector<int> weights(num_columns + 1);
    for (int i = 1; i <= num_columns; ++i)
    {
        fin >> weights[i];
    }

    // row_to_columns[i]：可以覆蓋第 i 列的欄位
    // columns[i].rows：第 i 欄可以覆蓋的列
    vector<vector<int>> row_to_columns(num_rows + 1);
    vector<Column> columns(num_columns + 1);

    for (int i = 1; i <= num_columns; ++i)
    {
        columns[i].id = i;
        columns[i].weight = weights[i];
    }

    int row, column;
    while (fin >> row >> column)
    {
        row_to_columns[row].push_back(column);
        columns[column].rows.push_back(row);
    }
    fin.close();

    // bit 1 到 bit num_rows 分別代表各列是否已被覆蓋
    long long target = (1LL << (num_rows + 1)) - 2;
    long long covered = 0;
    vector<int> chosen;

    // 只有一個欄位能覆蓋的列，那個欄位一定要選
    for (int i = 1; i <= num_rows; ++i)
    {
        if (row_to_columns[i].size() == 1)
        {
            int needed = row_to_columns[i][0];
            if (find(chosen.begin(), chosen.end(), needed) == chosen.end())
            {
                chosen.push_back(needed);
                covered = cover_rows(covered, columns[needed]);
            }
        }
    }

    // 已選過的欄位，以及不能再覆蓋新列的欄位，都不必放進搜尋
    vector<Column> candidates;
    for (int i = 1; i <= num_columns; ++i)
    {
        if (find(chosen.begin(), chosen.end(), i) != chosen.end())
        {
            continue;
        }

        bool covers_new_row = false;
        for (int r : columns[i].rows)
        {
            if ((covered & (1LL << r)) == 0)
            {
                covers_new_row = true;
                break;
            }
        }

        if (covers_new_row)
        {
            candidates.push_back(columns[i]);
        }
    }

    // 先試覆蓋列數較多的欄位；列數相同時先試重量較輕的
    sort(candidates.begin(), candidates.end(), [](const Column& a, const Column& b)
    {
        if (a.rows.size() != b.rows.size())
        {
            return a.rows.size() > b.rows.size();
        }
        return a.weight < b.weight;
    });

    int chosen_weight = 0;
    for (int i : chosen)
    {
        chosen_weight += weights[i];
    }

    Answer answer;
    search(0, static_cast<int>(chosen.size()), chosen_weight, covered, target, candidates, chosen, answer);

    sort(answer.columns.begin(), answer.columns.end());
    for (int i = 0; i < static_cast<int>(answer.columns.size()); ++i)
    {
        if (i > 0)
        {
            cout << ' ';
        }
        cout << answer.columns[i];
    }
    cout << "\n(" << answer.count << ", " << answer.weight << ")\n";

    return 0;
}
