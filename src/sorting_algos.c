#include "sorting_algos.h"

void swap(SortState *state, int i, int j) {
    int temp = state->arr[i];
    state->arr[i] = state->arr[j];
    state->arr[j] = temp;
    state->swaps++;
}

int bubbleStep(SortState *state, int *i, int *j) {
    if (*i >= state->size-1) return 1;
    if (*j < state->size - *i - 1) {
        state->comparisons++;
        if (state->arr[*j] > state->arr[*j+1])
            swap(state,*j,*j+1);
        (*j)++;
    } else {
        *j = 0;
        (*i)++;
    }
    return 0;
}

int insertionStep(SortState *state, int *i, int *j) {
    static int key;
    static int inserting = 0;

    if (*i >= state->size)
        return 1; // Done sorting

    if (!inserting) {
        key = state->arr[*i];
        *j = *i - 1;
        inserting = 1;
    }

    if (*j >= 0 && state->arr[*j] > key) {
        state->arr[*j + 1] = state->arr[*j];
        (*j)--;
        state->swaps++;
        return 0; // Continue animation
    } else {
        state->arr[*j + 1] = key;
        (*i)++;
        inserting = 0;
    }

    return 0; // Keep running until sorted
}


int selectionStep(SortState *state, int *i, int *j, int *min_idx) {
    if (*i >= state->size-1) return 1;
    if (*j < state->size) {
        if (state->arr[*j] < state->arr[*min_idx])
            *min_idx = *j;
        (*j)++;
    } else {
        swap(state, *i, *min_idx);
        (*i)++;
        *j = *i + 1;
        *min_idx = *i;
    }
    state->comparisons++;
    return 0;
}

int mergeStep(SortState *state, MergeState *m) {
    static int l = 0, r = 0, mid = 0, left[MAX_SIZE], right[MAX_SIZE];
    static int n1 = 0, n2 = 0;
    static int i = 0, j = 0, k = 0;
    static int step = 0;

    // Step 0: Setup merge sizes and arrays
    if (m->mergeSize == 0) {
        m->mergeSize = 2;
        m->mergeLeft = 0;
    }

    // When finished one full pass
    if (m->mergeLeft >= state->size - 1) {
        if (m->mergeSize >= state->size) {
            m->sorted = 1;
            return 1; // Done sorting
        }
        m->mergeLeft = 0;
        m->mergeSize *= 2;
        return 0;
    }

    // Step 1: Setup current subarrays
    if (step == 0) {
        l = m->mergeLeft;
        mid = l + m->mergeSize / 2 - 1;
        r = (l + m->mergeSize - 1 < state->size) ? (l + m->mergeSize - 1) : (state->size - 1);
        if (mid >= state->size) {
            m->mergeLeft += m->mergeSize;
            return 0;
        }

        n1 = mid - l + 1;
        n2 = r - mid;
        for (i = 0; i < n1; i++) left[i] = state->arr[l + i];
        for (j = 0; j < n2; j++) right[j] = state->arr[mid + 1 + j];
        i = j = 0;
        k = l;
        step = 1;
    }

    // Step 2: Merge animation (element by element)
    if (i < n1 && j < n2) {
        if (left[i] <= right[j]) {
            state->arr[k++] = left[i++];
        } else {
            state->arr[k++] = right[j++];
        }
        state->comparisons++;
        state->swaps++; // count as visual overwrite
        return 0;
    }

    // Step 3: Copy leftovers (left array)
    if (i < n1) {
        state->arr[k++] = left[i++];
        return 0;
    }

    // Step 4: Copy leftovers (right array)
    if (j < n2) {
        state->arr[k++] = right[j++];
        return 0;
    }

    // Step 5: Done merging this segment, move to next
    step = 0;
    m->mergeLeft += m->mergeSize;
    return 0;
}

int quickStep(SortState *state, QuickState *q) {
    if (state->size <= 1) return 1;

    // Initialize stack on first use
    if (q->top == -1 && !q->active) {
        if (state->size > 1) {
            q->top = 0;
            q->stackLeft[0] = 0;
            q->stackRight[0] = state->size - 1;
        } else {
            return 1;
        }
    }

    if (!q->active) {
        if (q->top < 0) {
            q->highlightCurrent = -1;
            q->highlightPivot = -1;
            return 1;
        }
        q->l = q->stackLeft[q->top];
        q->r = q->stackRight[q->top];
        q->top--;
        q->i = q->l - 1;
        q->j = q->l;
        q->pivotValue = state->arr[q->r];
        q->active = 1;
        q->highlightPivot = q->r;
        q->highlightCurrent = q->j;
        return 0;
    }

    if (q->j < q->r) {
        q->highlightCurrent = q->j;
        state->comparisons++;
        if (state->arr[q->j] <= q->pivotValue) {
            q->i++;
            if (q->i != q->j) {
                swap(state, q->i, q->j);
            }
        }
        q->j++;
        return 0;
    }

    int pivotIndex = q->i + 1;
    if (pivotIndex != q->r) {
        swap(state, pivotIndex, q->r);
    }
    q->highlightPivot = pivotIndex;
    q->highlightCurrent = pivotIndex;
    q->active = 0;

    if (pivotIndex - 1 > q->l) {
        q->top++;
        q->stackLeft[q->top] = q->l;
        q->stackRight[q->top] = pivotIndex - 1;
    }
    if (q->r > pivotIndex + 1) {
        q->top++;
        q->stackLeft[q->top] = pivotIndex + 1;
        q->stackRight[q->top] = q->r;
    }

    if (q->top < 0) {
        q->highlightCurrent = -1;
        q->highlightPivot = -1;
        return 1;
    }

    return 0;
}