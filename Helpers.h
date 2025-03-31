#pragma once
#include <Windows.h>
#include <vector>

//inline void SetComboBoxDropDownHeight(HWND hComboBox, int itemHeight)
//{
//    // Get combo box information
//    COMBOBOXINFO cbInfo;
//    cbInfo.cbSize = sizeof(COMBOBOXINFO);
//    GetComboBoxInfo(hComboBox, &cbInfo);
//
//    // Set the item height for the dropdown list
//    HWND hListBox = cbInfo.hwndList;
//    SendMessage(hListBox, LB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)itemHeight);
//}

template <typename T>
int removeDuplicates(std::vector<T>& vec) {
    int size = vec.size();
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    return size - vec.size(); // Return how many duplicates were removed
}