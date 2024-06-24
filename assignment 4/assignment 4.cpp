#include <iostream>
#include<Windows.h>
#include<TlHelp32.h>
#include<string.h>
#include<tchar.h>
#include<vector>

#pragma warning(disable:4996)

using namespace std;

string TCHARToString(TCHAR* str)
{
    int len = wcslen((wchar_t*)str);
    char* s = new char[2 * len + 1];
    wcstombs(s, (wchar_t*)str, 2 * len + 1);
    string out = s;
    delete[] s;
    return out;
}

class ProcessData
{
private:
    int id, pid, tab = 0;
    string name = "";
public:
    bool isParentTerminated = true;
    void SetTab(int n) { tab = n; }
    void SetId(int id) { this->id = id; }
    void SetPid(int in) { pid = in; }
    void SetName(string in) { name = in; }
    int GetTab() { return tab; }
    int GetId() { return id; }
    int GetPid() { return pid; }
    string GetName() { return name; }
};


vector<ProcessData> CalculateIndent(vector<ProcessData> data)
{
    for (int i = 1; i < data.size(); ++i)
    {
        int comp = data[i].GetPid();
        data[i].SetTab(1);
        for (int k = i; k != 0; --k)
        {
            if (comp == data[k].GetId())
            {
                data[i].SetTab(data[i].GetTab() + 1);
                comp = data[k].GetPid();
            }
        }
    }
    return data;
}

int main()
{
    vector<ProcessData> data;
    vector<ProcessData>print;
    int data_size = 0;
    HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcess == INVALID_HANDLE_VALUE)
    {
        cout << "Snap Processes Error";
        return 1;
    }
    PROCESSENTRY32 process;
    bool counter;
    process.dwSize = sizeof(PROCESSENTRY32);
    counter = Process32First(hProcess, &process);
    do {
        ++data_size;
    } while (Process32Next(hProcess, &process));

    print.reserve(data_size);
    Process32First(hProcess, &process);
    for (int i = 0; i < data_size; ++i)
    {
        ProcessData temp;
        temp.SetId(process.th32ProcessID);
        temp.SetPid(process.th32ParentProcessID);
        temp.SetName(TCHARToString(process.szExeFile));
        data.push_back(temp);
        Process32Next(hProcess, &process);
    }
    data[0].isParentTerminated = false;


    for (int i = 1; i < data_size; ++i)
    {
        for (int k = 0; k < data_size; ++k)
        {
            if (data[i].GetPid() == data[k].GetId())
            {
                data[i].isParentTerminated = false;     //isparentterminated 체크
                break;
            }
        }
    }

    print.push_back(data[0]);
    for (int i = 0; i < data_size; ++i)
    {
        if (data[i].isParentTerminated)                 //print에 초기데이터 세팅
            print.push_back(data[i]);
    }

    for (int j = 0; j < 3; ++j)
    {
        for (int i = 1; i < data_size; ++i)
        {
            for (int k = 0; k < print.size(); ++k)
            {
                if (data[i].isParentTerminated)
                    continue;
                else if (data[i].GetPid() == print[k].GetId())
                {
                    print.insert(print.begin() + k + 1, data[i]);
                    data[i].isParentTerminated = true;
                    break;
                }
            }
        }
    }
    if (print.size() != data.size())
    {
        cout << "Insert Error";
        return 1;
    }

    print = CalculateIndent(print);

    cout << "프로세스 트리" << endl;
    cout << "실행중인 프로세스 갯수 : " << data_size << endl << endl;
    for (int i = 0; i < print.size(); ++i)
    {
        printf("%03d ", i + 1);
        for (int k = 0; k < print[i].GetTab(); ++k)
        {
            if (print[i].GetTab() - 1 > k)
                cout << "  | ";
            else
                cout << "  |--";
        }
        cout << print[i].GetName() << " (" << print[i].GetId() << ") " << " (" << print[i].GetPid() << ':';
        if (print[i].isParentTerminated)
            cout << 0;
        else
            cout << print[i].GetPid();
        cout << ')' << endl;
    }

    CloseHandle(hProcess);

    return 0;
}