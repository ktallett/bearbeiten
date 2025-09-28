// JavaScript file for split view testing - RIGHT PANE
function bubbleSort(arr) {
    // Simple bubble sort implementation
    const n = arr.length;
    for (let i = 0; i < n; i++) {
        for (let j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                [arr[j], arr[j + 1]] = [arr[j + 1], arr[j]];
            }
        }
    }
    return arr;
}

function quickSort(arr) {
    // Simple quicksort implementation
    if (arr.length <= 1) {
        return arr;
    }

    const pivot = arr[Math.floor(arr.length / 2)];
    const left = arr.filter(x => x < pivot);
    const middle = arr.filter(x => x === pivot);
    const right = arr.filter(x => x > pivot);

    return [...quickSort(left), ...middle, ...quickSort(right)];
}

// Test the sorting algorithms
const testData = [64, 34, 25, 12, 22, 11, 90];
console.log("Original:", testData);
console.log("Bubble sort:", bubbleSort([...testData]));
console.log("Quick sort:", quickSort([...testData]));