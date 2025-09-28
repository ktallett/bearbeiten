# Python file for split view testing - LEFT PANE
def bubble_sort(arr):
    """Simple bubble sort implementation"""
    n = len(arr)
    for i in range(n):
        for j in range(0, n - i - 1):
            if arr[j] > arr[j + 1]:
                arr[j], arr[j + 1] = arr[j + 1], arr[j]
    return arr

def quick_sort(arr):
    """Simple quicksort implementation"""
    if len(arr) <= 1:
        return arr

    pivot = arr[len(arr) // 2]
    left = [x for x in arr if x < pivot]
    middle = [x for x in arr if x == pivot]
    right = [x for x in arr if x > pivot]

    return quick_sort(left) + middle + quick_sort(right)

# Test the sorting algorithms
test_data = [64, 34, 25, 12, 22, 11, 90]
print("Original:", test_data)
print("Bubble sort:", bubble_sort(test_data.copy()))
print("Quick sort:", quick_sort(test_data.copy()))